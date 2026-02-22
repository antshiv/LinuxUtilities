import {
  TOOLS,
  STORAGE_KEY,
  PALETTE,
  DEFAULT_BG,
  GRID_SNAP,
  clamp,
  uid,
  makeArtboard,
  sanitizeArtboards,
  createInitialState,
  attachStateAccessors
} from './state.js';
import { createSelectionController } from './selection.js';
import { createHandleController } from './handles.js';
import { createPathfinderController } from './pathfinder.js';
import { createHistoryController } from './history.js';
import { createRenderController } from './render.js';

const state = createInitialState();

let setSelection;
let clearSelection;
let selectedShapeIds;
let selectedShapes;
let finalizeMarqueeSelection;
let drawMarqueeOverlay;

let clampRectRadiusToBounds;
let findEditHandleAt;
let drawEditHandlesOverlay;
let beginHandleDrag;
let applyHandleDrag;

let pushHistory;
let undo;
let redo;

let applyPathfinder;
let render;

const els = {
  stage: document.getElementById('stage'),
  canvasWrap: document.getElementById('canvasWrap'),
  artboardTabs: document.getElementById('artboardTabs'),
  btnArtboardAdd: document.getElementById('btnArtboardAdd'),
  btnArtboardDup: document.getElementById('btnArtboardDup'),
  btnArtboardRename: document.getElementById('btnArtboardRename'),
  btnArtboardDelete: document.getElementById('btnArtboardDelete'),
  toolGrid: document.getElementById('toolGrid'),
  strokeColor: document.getElementById('strokeColor'),
  bgColor: document.getElementById('bgColor'),
  lineWidth: document.getElementById('lineWidth'),
  lineWidthValue: document.getElementById('lineWidthValue'),
  lineStyle: document.getElementById('lineStyle'),
  animated: document.getElementById('animated'),
  textSize: document.getElementById('textSize'),
  textContent: document.getElementById('textContent'),
  iconType: document.getElementById('iconType'),
  iconSize: document.getElementById('iconSize'),
  btnUndo: document.getElementById('btnUndo'),
  btnRedo: document.getElementById('btnRedo'),
  btnDelete: document.getElementById('btnDelete'),
  btnClear: document.getElementById('btnClear'),
  btnCenter: document.getElementById('btnCenter'),
  btnFit: document.getElementById('btnFit'),
  btnGrid: document.getElementById('btnGrid'),
  btnFocus: document.getElementById('btnFocus'),
  btnFullscreen: document.getElementById('btnFullscreen'),
  btnSaveJson: document.getElementById('btnSaveJson'),
  btnLoadJson: document.getElementById('btnLoadJson'),
  btnExportDsl: document.getElementById('btnExportDsl'),
  btnExportPng: document.getElementById('btnExportPng'),
  fileLoader: document.getElementById('fileLoader'),
  status: document.getElementById('status'),
  toolPill: document.getElementById('toolPill'),
  zoomPill: document.getElementById('zoomPill'),
  animPill: document.getElementById('animPill'),
  shapesPill: document.getElementById('shapesPill'),
  undoPill: document.getElementById('undoPill'),
  snapBadge: document.getElementById('snapBadge'),
  laserDot: document.getElementById('laserDot'),
  palette: document.getElementById('palette'),
  recentColors: document.getElementById('recentColors'),
  opacity: document.getElementById('opacity'),
  opacityValue: document.getElementById('opacityValue'),
  btnDuplicate: document.getElementById('btnDuplicate'),
  btnBringFwd: document.getElementById('btnBringFwd'),
  btnSendBack: document.getElementById('btnSendBack'),
  btnSnap: document.getElementById('btnSnap'),
  btnLineSolid: document.getElementById('btnLineSolid'),
  btnLineDashed: document.getElementById('btnLineDashed'),
  btnLineDotted: document.getElementById('btnLineDotted'),
  btnToggleArrow: document.getElementById('btnToggleArrow'),
  btnRectRadius: document.getElementById('btnRectRadius'),
  btnAlignLeft: document.getElementById('btnAlignLeft'),
  btnAlignCenter: document.getElementById('btnAlignCenter'),
  btnAlignRight: document.getElementById('btnAlignRight'),
  btnAlignTop: document.getElementById('btnAlignTop'),
  btnAlignMiddle: document.getElementById('btnAlignMiddle'),
  btnAlignBottom: document.getElementById('btnAlignBottom'),
  btnDistH: document.getElementById('btnDistH'),
  btnDistV: document.getElementById('btnDistV'),
  btnPathUnion: document.getElementById('btnPathUnion'),
  btnPathIntersect: document.getElementById('btnPathIntersect'),
  btnPathSubtract: document.getElementById('btnPathSubtract'),
  shortcutOverlay: document.getElementById('shortcutOverlay'),
  focusToggleBtn: document.getElementById('focusToggleBtn'),
  menuBtn: document.getElementById('menuBtn')
};

const ctx = els.stage.getContext('2d');
const DPR = Math.max(1, Math.min(2, window.devicePixelRatio || 1));

state.activeArtboardId = state.artboards[0].id;

function currentArtboard() {
  let board = state.artboards.find((a) => a.id === state.activeArtboardId);
  if (!board) {
    if (!state.artboards.length) {
      state.artboards.push(makeArtboard('Artboard 1'));
    }
    board = state.artboards[0];
    state.activeArtboardId = board.id;
  }
  return board;
}

attachStateAccessors(state, currentArtboard);

function setStatus(msg) {
  els.status.textContent = msg;
}

// Selection/marquee and editable handles are provided by module controllers.

function renderArtboardTabs() {
  if (!els.artboardTabs) return;
  els.artboardTabs.innerHTML = '';
  for (const board of state.artboards) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'art-tab';
    btn.classList.toggle('active', board.id === state.activeArtboardId);
    btn.textContent = `${board.name} (${board.shapes.length})`;
    btn.addEventListener('click', () => switchArtboard(board.id));
    els.artboardTabs.appendChild(btn);
  }
}

function switchArtboard(id) {
  if (!id || id === state.activeArtboardId) {
    return;
  }
  const exists = state.artboards.some((a) => a.id === id);
  if (!exists) {
    return;
  }
  state.activeArtboardId = id;
  clearSelection();
  syncControls();
  renderArtboardTabs();
  render();
  saveToLocalStorage();
  setStatus(`Switched to ${currentArtboard().name}.`);
}

function addArtboard() {
  const nextNum = state.artboards.length + 1;
  const board = makeArtboard(`Artboard ${nextNum}`);
  state.artboards.push(board);
  state.activeArtboardId = board.id;
  clearSelection();
  pushHistory();
  syncControls();
  renderArtboardTabs();
  render();
  setStatus(`Added ${board.name}.`);
}

function duplicateArtboard() {
  const src = currentArtboard();
  const copy = makeArtboard(`${src.name} Copy`, src);
  copy.shapes = copy.shapes.map((shape) => {
    const next = JSON.parse(JSON.stringify(shape));
    next.id = uid();
    return next;
  });
  state.artboards.push(copy);
  state.activeArtboardId = copy.id;
  clearSelection();
  pushHistory();
  syncControls();
  renderArtboardTabs();
  render();
  setStatus(`Duplicated ${src.name}.`);
}

function renameArtboard() {
  const board = currentArtboard();
  const raw = window.prompt('Artboard name:', board.name);
  if (raw === null) return;
  const name = raw.trim();
  if (!name) {
    setStatus('Artboard name cannot be empty.');
    return;
  }
  board.name = name;
  pushHistory();
  renderArtboardTabs();
  setStatus('Artboard renamed.');
}

function deleteArtboard() {
  if (state.artboards.length <= 1) {
    setStatus('At least one artboard is required.');
    return;
  }
  const idx = state.artboards.findIndex((a) => a.id === state.activeArtboardId);
  if (idx < 0) return;
  const removed = state.artboards[idx];
  state.artboards.splice(idx, 1);
  const nextIdx = Math.max(0, idx - 1);
  state.activeArtboardId = state.artboards[nextIdx].id;
  clearSelection();
  pushHistory();
  syncControls();
  renderArtboardTabs();
  render();
  setStatus(`Deleted ${removed.name}.`);
}

function snapPoint(p) {
  if (!state.snapGrid) return p;
  return {
    x: Math.round(p.x / GRID_SNAP) * GRID_SNAP,
    y: Math.round(p.y / GRID_SNAP) * GRID_SNAP
  };
}

function toWorld(px, py) {
  const rect = els.stage.getBoundingClientRect();
  const x = (px - rect.left) / state.camera.scale + state.camera.x;
  const y = (py - rect.top) / state.camera.scale + state.camera.y;
  return { x, y };
}

function toScreen(x, y) {
  return {
    x: (x - state.camera.x) * state.camera.scale,
    y: (y - state.camera.y) * state.camera.scale
  };
}

function styleFromControls() {
  return {
    color: state.strokeColor,
    width: state.lineWidth,
    style: state.lineStyle,
    animated: state.animated,
    opacity: state.opacity
  };
}

function getDashPattern(shape) {
  const mode = shape.style || 'solid';
  const width = Math.max(1, shape.width || 2);
  if (mode === 'dashed') {
    return [width * 2.6, width * 1.6];
  }
  if (mode === 'dotted') {
    return [width * 0.5, width * 1.55];
  }
  return [];
}

function getRevealProgress(shape) {
  if (!shape || !shape.animated) {
    return 1;
  }
  if (typeof shape.revealStartMs !== 'number' || typeof shape.revealDurationMs !== 'number' || shape.revealDurationMs <= 0) {
    return 1;
  }
  const progress = clamp((Date.now() - shape.revealStartMs) / shape.revealDurationMs, 0, 1);
  if (progress >= 1) {
    delete shape.revealStartMs;
    delete shape.revealDurationMs;
    return 1;
  }
  return progress;
}

function constrainEndpoint(tempShape, world, shiftKey) {
  let x = world.x;
  let y = world.y;
  if (!shiftKey || !tempShape) {
    return { x, y };
  }

  const dx = x - tempShape.x1;
  const dy = y - tempShape.y1;
  if (tempShape.type === 'line' || tempShape.type === 'arrow') {
    if (Math.abs(dx) >= Math.abs(dy)) {
      y = tempShape.y1;
    } else {
      x = tempShape.x1;
    }
    return { x, y };
  }

  if (tempShape.type === 'rect' || tempShape.type === 'ellipse') {
    const prevX2 = typeof tempShape.x2 === 'number' ? tempShape.x2 : tempShape.x1;
    const prevY2 = typeof tempShape.y2 === 'number' ? tempShape.y2 : tempShape.y1;
    const prevDx = prevX2 - tempShape.x1;
    const prevDy = prevY2 - tempShape.y1;
    const size = Math.max(Math.abs(dx), Math.abs(dy));
    const sx = Math.sign(dx) || Math.sign(prevDx) || 1;
    const sy = Math.sign(dy) || Math.sign(prevDy) || 1;
    x = tempShape.x1 + size * sx;
    y = tempShape.y1 + size * sy;
    return { x, y };
  }
  return { x, y };
}

function shouldRevealShape(shape) {
  if (!shape || !shape.animated) {
    return false;
  }
  return shape.type === 'line' || shape.type === 'arrow' || shape.type === 'rect' || shape.type === 'ellipse';
}

// History/undo logic is provided by the history controller module.

function drawGrid() {
  if (!state.grid) {
    return;
  }
  const spacing = 48;
  const left = state.camera.x;
  const top = state.camera.y;
  const right = left + els.stage.width / DPR / state.camera.scale;
  const bottom = top + els.stage.height / DPR / state.camera.scale;

  const startX = Math.floor(left / spacing) * spacing;
  const startY = Math.floor(top / spacing) * spacing;

  ctx.save();
  ctx.strokeStyle = 'rgba(190, 206, 255, 0.08)';
  ctx.lineWidth = 1 / state.camera.scale;
  ctx.setLineDash([]);

  for (let x = startX; x < right; x += spacing) {
    ctx.beginPath();
    ctx.moveTo(x, top);
    ctx.lineTo(x, bottom);
    ctx.stroke();
  }
  for (let y = startY; y < bottom; y += spacing) {
    ctx.beginPath();
    ctx.moveTo(left, y);
    ctx.lineTo(right, y);
    ctx.stroke();
  }

  ctx.restore();
}

function drawArrowHead(x1, y1, x2, y2, color, width) {
  const angle = Math.atan2(y2 - y1, x2 - x1);
  const size = Math.max(10, width * 3.5);
  const spread = Math.PI / 7;
  ctx.save();
  ctx.fillStyle = color;
  ctx.beginPath();
  ctx.moveTo(x2, y2);
  ctx.lineTo(x2 - size * Math.cos(angle - spread), y2 - size * Math.sin(angle - spread));
  ctx.lineTo(x2 - size * Math.cos(angle + spread), y2 - size * Math.sin(angle + spread));
  ctx.closePath();
  ctx.fill();
  ctx.restore();
}

function drawIconShape(shape) {
  const x = shape.x;
  const y = shape.y;
  const size = Math.max(20, shape.size || 80);
  const color = shape.color || '#9fd3ff';
  const w = size;
  const h = size;
  const sx = x - w / 2;
  const sy = y - h / 2;

  ctx.save();
  ctx.strokeStyle = color;
  ctx.fillStyle = 'rgba(126, 186, 255, 0.12)';
  ctx.lineWidth = Math.max(2, size / 18);
  ctx.lineJoin = 'round';
  ctx.lineCap = 'round';

  switch (shape.icon) {
    case 'server': {
      const unitH = h / 3.4;
      for (let i = 0; i < 3; i += 1) {
        const yy = sy + i * (unitH + h * 0.04);
        roundRect(sx + w * 0.12, yy, w * 0.76, unitH, w * 0.08, true, true);
        ctx.fillStyle = color;
        ctx.beginPath();
        ctx.arc(sx + w * 0.22, yy + unitH / 2, Math.max(2, size / 26), 0, Math.PI * 2);
        ctx.fill();
        ctx.fillStyle = 'rgba(126, 186, 255, 0.12)';
      }
      break;
    }
    case 'database': {
      const rx = w * 0.34;
      const ry = h * 0.11;
      const cx = x;
      let cy = sy + h * 0.22;
      for (let i = 0; i < 3; i += 1) {
        ctx.beginPath();
        ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
        ctx.stroke();
        ctx.beginPath();
        ctx.moveTo(cx - rx, cy);
        ctx.lineTo(cx - rx, cy + h * 0.18);
        ctx.moveTo(cx + rx, cy);
        ctx.lineTo(cx + rx, cy + h * 0.18);
        ctx.stroke();
        cy += h * 0.2;
      }
      ctx.beginPath();
      ctx.ellipse(cx, sy + h * 0.82, rx, ry, 0, 0, Math.PI);
      ctx.stroke();
      break;
    }
    case 'cloud': {
      ctx.beginPath();
      ctx.arc(x - w * 0.16, y + h * 0.03, w * 0.16, Math.PI * 0.9, Math.PI * 1.95);
      ctx.arc(x + w * 0.05, y - h * 0.06, w * 0.2, Math.PI, Math.PI * 2.02);
      ctx.arc(x + w * 0.27, y + h * 0.05, w * 0.14, Math.PI * 1.15, Math.PI * 2.02);
      ctx.lineTo(x + w * 0.3, y + h * 0.23);
      ctx.lineTo(x - w * 0.31, y + h * 0.23);
      ctx.closePath();
      ctx.fill();
      ctx.stroke();
      break;
    }
    case 'user': {
      ctx.beginPath();
      ctx.arc(x, y - h * 0.17, w * 0.15, 0, Math.PI * 2);
      ctx.stroke();
      ctx.beginPath();
      ctx.arc(x, y + h * 0.2, w * 0.28, Math.PI, 0);
      ctx.stroke();
      break;
    }
    case 'api': {
      roundRect(sx + w * 0.08, sy + h * 0.12, w * 0.84, h * 0.72, w * 0.08, true, true);
      ctx.fillStyle = color;
      ctx.font = `${Math.max(12, Math.floor(size * 0.22))}px "JetBrains Mono", monospace`;
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText('API', x, y);
      break;
    }
    case 'dataset': {
      for (let i = 0; i < 3; i += 1) {
        const yy = sy + h * (0.18 + i * 0.18);
        roundRect(sx + w * 0.18, yy, w * 0.64, h * 0.15, w * 0.04, true, true);
      }
      break;
    }
    case 'gpu': {
      roundRect(sx + w * 0.16, sy + h * 0.16, w * 0.68, h * 0.68, w * 0.07, true, true);
      for (let i = 0; i < 5; i += 1) {
        const xx = sx + w * (0.1 + i * 0.16);
        ctx.beginPath();
        ctx.moveTo(xx, sy + h * 0.09);
        ctx.lineTo(xx, sy + h * 0.16);
        ctx.moveTo(xx, sy + h * 0.84);
        ctx.lineTo(xx, sy + h * 0.91);
        ctx.stroke();
      }
      ctx.fillStyle = color;
      ctx.font = `${Math.max(10, Math.floor(size * 0.17))}px "JetBrains Mono", monospace`;
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText('GPU', x, y);
      break;
    }
    case 'tensor': {
      roundRect(sx + w * 0.18, sy + h * 0.18, w * 0.64, h * 0.64, w * 0.06, false, true);
      for (let i = 1; i <= 3; i += 1) {
        const xx = sx + w * (0.18 + i * 0.16);
        const yy = sy + h * (0.18 + i * 0.16);
        ctx.beginPath();
        ctx.moveTo(xx, sy + h * 0.18);
        ctx.lineTo(xx, sy + h * 0.82);
        ctx.moveTo(sx + w * 0.18, yy);
        ctx.lineTo(sx + w * 0.82, yy);
        ctx.stroke();
      }
      break;
    }
    case 'gear': {
      const cx = x;
      const cy = y;
      const r1 = w * 0.2;
      const r2 = w * 0.34;
      const teeth = 10;
      ctx.beginPath();
      for (let i = 0; i < teeth * 2; i += 1) {
        const a = (Math.PI * i) / teeth;
        const r = i % 2 === 0 ? r2 : r1;
        const px = cx + Math.cos(a) * r;
        const py = cy + Math.sin(a) * r;
        if (i === 0) {
          ctx.moveTo(px, py);
        } else {
          ctx.lineTo(px, py);
        }
      }
      ctx.closePath();
      ctx.stroke();
      ctx.beginPath();
      ctx.arc(cx, cy, w * 0.1, 0, Math.PI * 2);
      ctx.stroke();
      break;
    }
    case 'box': {
      roundRect(sx + w * 0.12, sy + h * 0.12, w * 0.76, h * 0.76, w * 0.06, true, true);
      break;
    }
    case 'check': {
      ctx.beginPath();
      ctx.moveTo(sx + w * 0.2, sy + h * 0.56);
      ctx.lineTo(sx + w * 0.44, sy + h * 0.76);
      ctx.lineTo(sx + w * 0.82, sy + h * 0.28);
      ctx.stroke();
      break;
    }
    default:
      roundRect(sx + w * 0.12, sy + h * 0.12, w * 0.76, h * 0.76, w * 0.06, true, true);
  }
  ctx.restore();
}

function roundRect(x, y, width, height, radius, fill, stroke) {
  const r = Math.min(radius, width / 2, height / 2);
  ctx.beginPath();
  ctx.moveTo(x + r, y);
  ctx.arcTo(x + width, y, x + width, y + height, r);
  ctx.arcTo(x + width, y + height, x, y + height, r);
  ctx.arcTo(x, y + height, x, y, r);
  ctx.arcTo(x, y, x + width, y, r);
  ctx.closePath();
  if (fill) {
    ctx.fill();
  }
  if (stroke) {
    ctx.stroke();
  }
}

function drawShape(shape, highlight) {
  ctx.save();
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';
  ctx.globalAlpha = typeof shape.opacity === 'number' ? shape.opacity : 1;
  const color = shape.color || '#7ec8ff';
  const width = Math.max(1, shape.width || 2);
  const dash = getDashPattern(shape);
  const animated = !!shape.animated && dash.length > 0;
  const revealProgress = getRevealProgress(shape);

  ctx.strokeStyle = color;
  ctx.fillStyle = color;
  ctx.lineWidth = width;
  ctx.setLineDash(dash);
  if (animated) {
    ctx.lineDashOffset = -state.dashTick * 1.5;
  }

  if (shape.type === 'pen') {
    const pts = shape.points || [];
    if (pts.length > 1) {
      ctx.beginPath();
      ctx.moveTo(pts[0].x, pts[0].y);
      for (let i = 1; i < pts.length; i += 1) {
        ctx.lineTo(pts[i].x, pts[i].y);
      }
      ctx.stroke();
    }
  } else if (shape.type === 'line' || shape.type === 'arrow') {
    const ex = shape.x1 + (shape.x2 - shape.x1) * revealProgress;
    const ey = shape.y1 + (shape.y2 - shape.y1) * revealProgress;
    ctx.beginPath();
    ctx.moveTo(shape.x1, shape.y1);
    ctx.lineTo(ex, ey);
    ctx.stroke();
    if (shape.type === 'arrow' && Math.hypot(ex - shape.x1, ey - shape.y1) > 0.5 / state.camera.scale) {
      drawArrowHead(shape.x1, shape.y1, ex, ey, color, width);
    }
  } else if (shape.type === 'rect') {
    const x2 = shape.x1 + (shape.x2 - shape.x1) * revealProgress;
    const y2 = shape.y1 + (shape.y2 - shape.y1) * revealProgress;
    const x = Math.min(shape.x1, x2);
    const y = Math.min(shape.y1, y2);
    const w = Math.abs(x2 - shape.x1);
    const h = Math.abs(y2 - shape.y1);
    const radius = Math.max(0, Number(shape.radius) || 0);
    if (radius > 0.1 && w > 0.5 && h > 0.5) {
      roundRect(x, y, w, h, radius, false, true);
    } else {
      ctx.strokeRect(x, y, w, h);
    }
  } else if (shape.type === 'ellipse') {
    const x2 = shape.x1 + (shape.x2 - shape.x1) * revealProgress;
    const y2 = shape.y1 + (shape.y2 - shape.y1) * revealProgress;
    const cx = (shape.x1 + x2) / 2;
    const cy = (shape.y1 + y2) / 2;
    const rx = Math.abs(x2 - shape.x1) / 2;
    const ry = Math.abs(y2 - shape.y1) / 2;
    ctx.beginPath();
    ctx.ellipse(cx, cy, rx, ry, 0, 0, Math.PI * 2);
    ctx.stroke();
  } else if (shape.type === 'text') {
    ctx.setLineDash([]);
    ctx.fillStyle = color;
    ctx.font = `${Math.max(12, shape.size || 28)}px "Fira Sans", "Noto Sans", sans-serif`;
    ctx.textAlign = 'left';
    ctx.textBaseline = 'middle';
    const lines = String(shape.text || 'Text').split('\n');
    const step = Math.max(14, (shape.size || 28) * 1.24);
    for (let i = 0; i < lines.length; i += 1) {
      ctx.fillText(lines[i], shape.x, shape.y + i * step);
    }
  } else if (shape.type === 'icon') {
    drawIconShape(shape);
  }

  if (highlight) {
    const box = getBounds(shape);
    if (box) {
      ctx.setLineDash([8 / state.camera.scale, 6 / state.camera.scale]);
      ctx.lineWidth = 1.4 / state.camera.scale;
      ctx.strokeStyle = 'rgba(176, 204, 255, 0.9)';
      ctx.strokeRect(box.x - 8 / state.camera.scale, box.y - 8 / state.camera.scale, box.w + 16 / state.camera.scale, box.h + 16 / state.camera.scale);
    }
  }

  ctx.restore();
}

function getBounds(shape) {
  if (!shape) {
    return null;
  }
  if (shape.type === 'pen') {
    const pts = shape.points || [];
    if (!pts.length) {
      return null;
    }
    let minX = pts[0].x;
    let maxX = pts[0].x;
    let minY = pts[0].y;
    let maxY = pts[0].y;
    for (const p of pts) {
      minX = Math.min(minX, p.x);
      maxX = Math.max(maxX, p.x);
      minY = Math.min(minY, p.y);
      maxY = Math.max(maxY, p.y);
    }
    return { x: minX, y: minY, w: maxX - minX, h: maxY - minY };
  }
  if (shape.type === 'line' || shape.type === 'arrow' || shape.type === 'rect' || shape.type === 'ellipse') {
    const minX = Math.min(shape.x1, shape.x2);
    const minY = Math.min(shape.y1, shape.y2);
    return {
      x: minX,
      y: minY,
      w: Math.abs(shape.x2 - shape.x1),
      h: Math.abs(shape.y2 - shape.y1)
    };
  }
  if (shape.type === 'text') {
    const fontSize = Math.max(12, shape.size || 28);
    const text = String(shape.text || 'Text');
    const lines = text.split('\n');
    ctx.save();
    ctx.font = `${fontSize}px "Fira Sans", "Noto Sans", sans-serif`;
    let maxW = 0;
    for (const line of lines) {
      maxW = Math.max(maxW, ctx.measureText(line).width);
    }
    ctx.restore();
    return { x: shape.x, y: shape.y - fontSize * 0.55, w: maxW, h: lines.length * fontSize * 1.24 };
  }
  if (shape.type === 'icon') {
    const size = Math.max(20, shape.size || 80);
    return { x: shape.x - size / 2, y: shape.y - size / 2, w: size, h: size };
  }
  return null;
}

function distToSegment(px, py, x1, y1, x2, y2) {
  const dx = x2 - x1;
  const dy = y2 - y1;
  if (dx === 0 && dy === 0) {
    return Math.hypot(px - x1, py - y1);
  }
  const t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy);
  const clamped = clamp(t, 0, 1);
  const x = x1 + clamped * dx;
  const y = y1 + clamped * dy;
  return Math.hypot(px - x, py - y);
}

function hitShape(shape, x, y) {
  const tol = Math.max(8 / state.camera.scale, (shape.width || 3) + 3);
  if (shape.type === 'pen') {
    const pts = shape.points || [];
    for (let i = 1; i < pts.length; i += 1) {
      if (distToSegment(x, y, pts[i - 1].x, pts[i - 1].y, pts[i].x, pts[i].y) <= tol) {
        return true;
      }
    }
    return false;
  }

  if (shape.type === 'line' || shape.type === 'arrow') {
    return distToSegment(x, y, shape.x1, shape.y1, shape.x2, shape.y2) <= tol;
  }

  if (shape.type === 'rect' || shape.type === 'ellipse' || shape.type === 'text' || shape.type === 'icon') {
    const box = getBounds(shape);
    if (!box) {
      return false;
    }
    return x >= box.x - tol && x <= box.x + box.w + tol && y >= box.y - tol && y <= box.y + box.h + tol;
  }
  return false;
}

function findShapeAt(x, y) {
  for (let i = state.shapes.length - 1; i >= 0; i -= 1) {
    const shape = state.shapes[i];
    if (hitShape(shape, x, y)) {
      return shape;
    }
  }
  return null;
}

function moveShape(shape, dx, dy) {
  if (shape.type === 'pen') {
    for (const p of shape.points || []) {
      p.x += dx;
      p.y += dy;
    }
    return;
  }
  if (shape.type === 'text' || shape.type === 'icon') {
    shape.x += dx;
    shape.y += dy;
    return;
  }
  if (typeof shape.x1 === 'number') {
    shape.x1 += dx;
    shape.y1 += dy;
    shape.x2 += dx;
    shape.y2 += dy;
  }
}

function beginDraw(world) {
  const style = styleFromControls();
  const tool = state.tool;
  state.pointer.temp = null;
  const w = snapPoint(world);

  if (tool === 'laser') {
    state.pointer.drawing = false;
    return;
  }

  if (tool === 'pen') {
    state.pointer.temp = {
      id: uid(),
      type: 'pen',
      points: [{ x: w.x, y: w.y }],
      color: style.color,
      width: style.width,
      style: style.style,
      animated: style.animated,
      opacity: style.opacity
    };
    state.pointer.drawing = true;
    return;
  }

  if (tool === 'line' || tool === 'arrow' || tool === 'rect' || tool === 'ellipse') {
    const temp = {
      id: uid(),
      type: tool,
      x1: w.x,
      y1: w.y,
      x2: w.x,
      y2: w.y,
      color: style.color,
      width: style.width,
      style: style.style,
      animated: style.animated,
      opacity: style.opacity
    };
    if (tool === 'rect') {
      temp.radius = 0;
    }
    state.pointer.temp = temp;
    state.pointer.drawing = true;
    return;
  }

  if (tool === 'text') {
    const rawText = (state.textContent || 'Text').trim();
    const payload = rawText ? rawText : 'Text';
    const id = uid();
    state.shapes.push({
      id,
      type: 'text',
      x: w.x,
      y: w.y,
      text: payload,
      size: state.textSize,
      color: style.color,
      opacity: style.opacity
    });
    trackRecentColor(style.color);
    pushHistory();
    setSelection([id]);
    setStatus('Text placed.');
    return;
  }

  if (tool === 'icon') {
    const id = uid();
    state.shapes.push({
      id,
      type: 'icon',
      x: w.x,
      y: w.y,
      size: state.iconSize,
      icon: state.iconType,
      color: style.color,
      opacity: style.opacity
    });
    trackRecentColor(style.color);
    pushHistory();
    setSelection([id]);
    setStatus('Icon placed.');
    return;
  }

  if (tool === 'eraser') {
    const hit = findShapeAt(world.x, world.y);
    if (hit) {
      state.shapes = state.shapes.filter((s) => s.id !== hit.id);
      clearSelection();
      pushHistory();
      setStatus('Shape erased.');
    }
    return;
  }

  if (tool === 'select') {
    const hit = findShapeAt(world.x, world.y);
    setSelection(hit ? [hit.id] : []);
    state.pointer.drawing = false;
    setStatus(hit ? 'Shape selected. Drag to move.' : 'Selection cleared.');
    return;
  }

  if (tool === 'pan') {
    state.pointer.drawing = false;
    return;
  }
}

function continueDraw(world, shiftKey) {
  if (!state.pointer.temp) {
    return;
  }
  const t = state.pointer.temp;
  if (t.type === 'pen') {
    const pts = t.points;
    const last = pts[pts.length - 1];
    const d = Math.hypot(world.x - last.x, world.y - last.y);
    if (d > 1.2 / state.camera.scale) {
      pts.push({ x: world.x, y: world.y });
    }
    return;
  }
  const snapped = snapPoint(world);
  const constrained = constrainEndpoint(t, snapped, shiftKey);
  t.x2 = constrained.x;
  t.y2 = constrained.y;
}

function finishDraw() {
  if (!state.pointer.temp) {
    return;
  }
  const t = state.pointer.temp;
  state.pointer.temp = null;
  state.pointer.drawing = false;

  if (t.type === 'pen' && (t.points || []).length < 2) {
    return;
  }

  if ((t.type === 'line' || t.type === 'arrow' || t.type === 'rect' || t.type === 'ellipse') &&
      Math.hypot(t.x2 - t.x1, t.y2 - t.y1) < 2 / state.camera.scale) {
    return;
  }

  if (shouldRevealShape(t)) {
    t.revealStartMs = Date.now();
    t.revealDurationMs = state.drawAnimDurationMs;
  }

  state.shapes.push(t);
  setSelection([t.id]);
  trackRecentColor(t.color);
  pushHistory();
}

function deleteSelected() {
  const ids = selectedShapeIds();
  if (!ids.length) {
    return;
  }
  const countBefore = state.shapes.length;
  state.shapes = state.shapes.filter((s) => !ids.includes(s.id));
  clearSelection();
  if (state.shapes.length !== countBefore) {
    pushHistory();
    setStatus('Deleted selected shape(s).');
  }
}

function clearAll() {
  state.shapes = [];
  clearSelection();
  pushHistory();
  setStatus('Artboard cleared.');
}

function saveToLocalStorage() {
  try {
    const payload = {
      version: 2,
      artboards: state.artboards,
      activeArtboardId: state.activeArtboardId,
      grid: state.grid,
      ui: {
        lineWidth: state.lineWidth,
        strokeColor: state.strokeColor,
        lineStyle: state.lineStyle,
        animated: state.animated,
        opacity: state.opacity,
        textSize: state.textSize,
        textContent: state.textContent,
        iconType: state.iconType,
        iconSize: state.iconSize,
        tool: state.tool,
        snapGrid: state.snapGrid,
        recentColors: state.recentColors
      }
    };
    localStorage.setItem(STORAGE_KEY, JSON.stringify(payload));
  } catch (_err) {
  }
}

function loadFromLocalStorage() {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (!raw) {
      return false;
    }
    const payload = JSON.parse(raw);
    if (Array.isArray(payload.artboards) && payload.artboards.length) {
      state.artboards = sanitizeArtboards(payload.artboards);
      state.activeArtboardId = payload.activeArtboardId || state.artboards[0].id;
    } else {
      state.artboards = sanitizeArtboards(null, {
        name: 'Artboard 1',
        bgColor: payload.bgColor || DEFAULT_BG,
        camera: payload.camera || { x: 0, y: 0, scale: 1 },
        shapes: Array.isArray(payload.shapes) ? payload.shapes : []
      });
      state.activeArtboardId = state.artboards[0].id;
    }
    state.grid = payload.grid !== false;
    if (payload.ui) {
      state.lineWidth = payload.ui.lineWidth || state.lineWidth;
      state.strokeColor = payload.ui.strokeColor || state.strokeColor;
      state.lineStyle = payload.ui.lineStyle || state.lineStyle;
      state.animated = !!payload.ui.animated;
      state.textSize = payload.ui.textSize || state.textSize;
      state.textContent = payload.ui.textContent || state.textContent;
      state.iconType = payload.ui.iconType || state.iconType;
      state.iconSize = payload.ui.iconSize || state.iconSize;
      state.tool = payload.ui.tool || state.tool;
      state.opacity = typeof payload.ui.opacity === 'number' ? payload.ui.opacity : 1;
      state.snapGrid = !!payload.ui.snapGrid;
      state.recentColors = Array.isArray(payload.ui.recentColors) ? payload.ui.recentColors : [];
    }
    clearSelection();
    renderArtboardTabs();
    pushHistory();
    setStatus('Restored autosaved canvas.');
    return true;
  } catch (_err) {
    return false;
  }
}

function exportJson() {
  const board = currentArtboard();
  const payload = {
    version: 2,
    createdAt: new Date().toISOString(),
    activeArtboardId: state.activeArtboardId,
    artboards: state.artboards,
    bgColor: board.bgColor,
    grid: state.grid,
    camera: board.camera,
    shapes: board.shapes
  };
  const blob = new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `presenter-canvas-${Date.now()}.json`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
  setStatus('Saved JSON.');
}

function exportDslStarter() {
  const primitives = state.shapes.map((shape, idx) => {
    const copy = JSON.parse(JSON.stringify(shape));
    if (!copy.id) {
      copy.id = `shape-${idx + 1}`;
    }
    return copy;
  });

  const hasEdges = primitives.some((p) => p.type === 'line' || p.type === 'arrow' || p.type === 'pen');
  const hasIcons = primitives.some((p) => p.type === 'icon');

  const timeline = [
    { at: 0.0, do: 'hide', targets: '*', duration: 0 },
    { at: 0.1, do: 'show', targets: ['type:node', 'type:icon'], duration: 0.55, ease: 'easeOutCubic' }
  ];

  if (hasEdges) {
    timeline.push({ at: 0.85, do: 'draw', targets: 'type:edge', duration: 0.95, ease: 'easeInOutCubic' });
  }
  if (hasIcons) {
    timeline.push({ at: 2.0, do: 'pulse', targets: 'type:icon', duration: 0.75, scale: 1.12, ease: 'easeInOutCubic' });
  }

  timeline.push({ at: 3.0, do: 'zoom', duration: 0.65, to: 1.15, ease: 'easeInOutCubic' });
  timeline.push({ at: 3.8, do: 'pan', duration: 0.65, x: state.camera.x - 80, y: state.camera.y - 20, ease: 'easeInOutCubic' });

  const payload = {
    version: 1,
    meta: {
      title: `${currentArtboard().name} Storyboard`,
      bg: state.bgColor,
      duration: 5.5,
      camera: {
        x: state.camera.x,
        y: state.camera.y,
        scale: state.camera.scale
      }
    },
    scene: { primitives },
    timeline
  };

  const blob = new Blob([JSON.stringify(payload, null, 2)], { type: 'application/json' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `storyboard-starter-${Date.now()}.json`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
  setStatus('Exported DSL starter JSON.');
}

function importJson(file) {
  if (!file) {
    return;
  }
  const reader = new FileReader();
  reader.onload = () => {
    try {
      const payload = JSON.parse(String(reader.result || '{}'));
      if (Array.isArray(payload.artboards) && payload.artboards.length) {
        state.artboards = sanitizeArtboards(payload.artboards);
        state.activeArtboardId = payload.activeArtboardId || state.artboards[0].id;
      } else if (Array.isArray(payload.shapes)) {
        state.artboards = sanitizeArtboards(null, {
          name: 'Artboard 1',
          bgColor: payload.bgColor || DEFAULT_BG,
          camera: payload.camera || { x: 0, y: 0, scale: 1 },
          shapes: payload.shapes
        });
        state.activeArtboardId = state.artboards[0].id;
      } else {
        throw new Error('Missing shapes/artboards');
      }
      state.grid = payload.grid !== false;
      clearSelection();
      pushHistory();
      renderArtboardTabs();
      syncControls();
      setStatus(`Loaded ${state.shapes.length} shapes in ${currentArtboard().name}.`);
    } catch (err) {
      setStatus('Invalid JSON file.');
    }
  };
  reader.readAsText(file);
}

function exportPng() {
  const previousIds = selectedShapeIds();
  clearSelection();
  render();
  const url = els.stage.toDataURL('image/png');
  const a = document.createElement('a');
  a.href = url;
  a.download = `presenter-canvas-${Date.now()}.png`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  setSelection(previousIds);
  render();
  setStatus('Exported PNG.');
}

function centerView() {
  state.camera.x = 0;
  state.camera.y = 0;
  state.camera.scale = 1;
  saveToLocalStorage();
  render();
  setStatus('View reset to center.');
}

function fitContent() {
  if (!state.shapes.length) {
    centerView();
    return;
  }
  let minX = Infinity;
  let minY = Infinity;
  let maxX = -Infinity;
  let maxY = -Infinity;

  for (const shape of state.shapes) {
    const b = getBounds(shape);
    if (!b) {
      continue;
    }
    minX = Math.min(minX, b.x);
    minY = Math.min(minY, b.y);
    maxX = Math.max(maxX, b.x + b.w);
    maxY = Math.max(maxY, b.y + b.h);
  }

  if (!isFinite(minX)) {
    centerView();
    return;
  }

  const padding = 80;
  minX -= padding;
  minY -= padding;
  maxX += padding;
  maxY += padding;

  const worldW = Math.max(1, maxX - minX);
  const worldH = Math.max(1, maxY - minY);
  const screenW = els.stage.width / DPR;
  const screenH = els.stage.height / DPR;

  const scale = clamp(Math.min(screenW / worldW, screenH / worldH), 0.2, 5);
  state.camera.scale = scale;
  state.camera.x = minX - (screenW / scale - worldW) / 2;
  state.camera.y = minY - (screenH / scale - worldH) / 2;
  render();
  saveToLocalStorage();
  setStatus('Fitted content to viewport.');
}

function applyPointerDown(event) {
  const isMiddle = event.button === 1;
  const panWithSpace = state.spacePan;
  const shouldPan = isMiddle || panWithSpace || state.tool === 'pan';

  state.pointer.down = true;
  state.pointer.start = { x: event.clientX, y: event.clientY };
  state.pointer.current = { x: event.clientX, y: event.clientY };

  const world = toWorld(event.clientX, event.clientY);

  if (state.tool === 'select' && !shouldPan) {
    const handleHit = findEditHandleAt(world.x, world.y);
    if (handleHit) {
      setSelection([handleHit.shape.id]);
      state.draggingSelected = false;
      state.pointer.lastWorld = null;
      state.pointer.marquee = null;
      beginHandleDrag(handleHit);
      render();
      return;
    }
    state.pointer.handleDrag = null;

    const hit = findShapeAt(world.x, world.y);
    const shift = !!event.shiftKey;
    const alt = !!event.altKey;
    if (hit) {
      const selected = selectedShapeIds();
      if (shift) {
        if (selected.includes(hit.id)) {
          setSelection(selected.filter((id) => id !== hit.id));
        } else {
          setSelection(selected.concat(hit.id));
        }
      } else if (alt) {
        if (selected.includes(hit.id)) {
          setSelection(selected.filter((id) => id !== hit.id));
        }
      } else {
        setSelection([hit.id]);
      }

      if (selectedShapeIds().includes(hit.id) && !alt) {
        state.draggingSelected = true;
        state.pointer.lastWorld = world;
      } else {
        state.draggingSelected = false;
        state.pointer.lastWorld = null;
      }

      state.pointer.marquee = null;
    } else {
      state.draggingSelected = false;
      state.pointer.lastWorld = null;
      state.pointer.marquee = {
        start: world,
        current: world,
        mode: alt ? 'subtract' : shift ? 'add' : 'replace'
      };
      if (!shift && !alt) {
        clearSelection();
      }
    }
    render();
    return;
  }

  if (shouldPan) {
    state.pointer.handleDrag = null;
    state.panning = true;
    els.stage.style.cursor = 'grabbing';
    return;
  }

  state.pointer.handleDrag = null;
  beginDraw(world);
  render();
}

function applyPointerMove(event) {
  // Laser pointer: always track, even when not "down"
  if (state.tool === 'laser') {
    showLaser(event.clientX, event.clientY);
  }

  if (!state.pointer.down) {
    return;
  }

  const dxScreen = event.clientX - state.pointer.current.x;
  const dyScreen = event.clientY - state.pointer.current.y;
  state.pointer.current = { x: event.clientX, y: event.clientY };

  if (state.panning) {
    state.camera.x -= dxScreen / state.camera.scale;
    state.camera.y -= dyScreen / state.camera.scale;
    render();
    return;
  }

  const world = toWorld(event.clientX, event.clientY);

  if (state.pointer.handleDrag) {
    applyHandleDrag(world, event.shiftKey);
    render();
    return;
  }

  if (state.pointer.marquee) {
    state.pointer.marquee.current = world;
    render();
    return;
  }

  if (state.draggingSelected && selectedShapeIds().length) {
    const selected = selectedShapes();
    if (selected.length && state.pointer.lastWorld) {
      const dx = world.x - state.pointer.lastWorld.x;
      const dy = world.y - state.pointer.lastWorld.y;
      for (const shape of selected) {
        moveShape(shape, dx, dy);
      }
      state.pointer.lastWorld = world;
      render();
      return;
    }
  }

  continueDraw(world, event.shiftKey);
  render();
}

function applyPointerUp() {
  if (!state.pointer.down) {
    return;
  }

  if (state.pointer.marquee) {
    finalizeMarqueeSelection();
  }

  if (state.pointer.handleDrag) {
    const drag = state.pointer.handleDrag;
    state.pointer.handleDrag = null;
    if (drag.didMove) {
      pushHistory();
      if (drag.handleId === 'radius') {
        setStatus('Adjusted rectangle corner radius.');
      } else {
        setStatus('Edited selected shape points.');
      }
    }
  }

  if (state.draggingSelected && selectedShapeIds().length) {
    state.draggingSelected = false;
    pushHistory();
    setStatus('Moved selected shape(s).');
  }

  finishDraw();

  state.pointer.down = false;
  state.panning = false;
  state.draggingSelected = false;
  state.pointer.start = null;
  state.pointer.current = null;
  state.pointer.lastWorld = null;
  state.pointer.marquee = null;
  state.pointer.handleDrag = null;
  if (state.tool === 'pan') {
    els.stage.style.cursor = 'grab';
  } else if (state.tool === 'select') {
    els.stage.style.cursor = 'default';
  } else if (state.tool === 'eraser') {
    els.stage.style.cursor = 'not-allowed';
  } else if (state.tool === 'laser') {
    els.stage.style.cursor = 'none';
  } else {
    els.stage.style.cursor = 'crosshair';
  }
  render();
}

function resizeCanvas() {
  const rect = els.canvasWrap.getBoundingClientRect();
  els.stage.width = Math.max(1, Math.floor(rect.width * DPR));
  els.stage.height = Math.max(1, Math.floor(rect.height * DPR));
  ctx.setTransform(DPR, 0, 0, DPR, 0, 0);
  render();
}

// Render loop function is provided by the render controller module.

function setTool(tool) {
  state.tool = tool;
  if (tool === 'pan') {
    els.stage.style.cursor = 'grab';
  } else if (tool === 'select') {
    els.stage.style.cursor = 'default';
  } else if (tool === 'eraser') {
    els.stage.style.cursor = 'not-allowed';
  } else if (tool === 'laser') {
    els.stage.style.cursor = 'none';
  } else {
    els.stage.style.cursor = 'crosshair';
  }

  // Hide laser dot when switching away from laser
  if (tool !== 'laser' && els.laserDot) {
    els.laserDot.classList.remove('visible');
  }

  for (const btn of els.toolGrid.querySelectorAll('button.tool')) {
    btn.classList.toggle('active', btn.dataset.tool === tool);
  }
  saveToLocalStorage();
  render();
}

function toggleFocusMode(force) {
  const willFocus = typeof force === 'boolean' ? force : !document.body.classList.contains('focus');
  document.body.classList.toggle('focus', willFocus);
  if (!willFocus) {
    document.body.classList.remove('menu-open');
  }
}

function toggleFullscreen() {
  if (!document.fullscreenElement) {
    document.documentElement.requestFullscreen().catch(() => {});
  } else {
    document.exitFullscreen().catch(() => {});
  }
}

function initTools() {
  for (const tool of TOOLS) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'tool';
    btn.dataset.tool = tool.id;
    btn.textContent = `${tool.label}\n[${tool.hotkey}]`;
    btn.addEventListener('click', () => setTool(tool.id));
    els.toolGrid.appendChild(btn);
  }
}

function syncControls() {
  els.strokeColor.value = state.strokeColor;
  els.bgColor.value = state.bgColor;
  els.lineWidth.value = String(state.lineWidth);
  els.lineWidthValue.textContent = `${state.lineWidth}px`;
  els.lineStyle.value = state.lineStyle;
  els.animated.checked = state.animated;
  els.opacity.value = String(state.opacity);
  els.opacityValue.textContent = state.opacity.toFixed(2);
  els.textSize.value = String(state.textSize);
  els.textContent.value = state.textContent;
  els.iconType.value = state.iconType;
  els.iconSize.value = String(state.iconSize);
  els.btnSnap.textContent = state.snapGrid ? 'Snap: On' : 'Snap: Off';
  renderArtboardTabs();
  updatePalette();
  renderRecentColors();
  setTool(state.tool);
}

function animate() {
  state.dashTick += 1;
  render();
  requestAnimationFrame(animate);
}

function setupBindings() {
  els.strokeColor.addEventListener('input', () => {
    state.strokeColor = els.strokeColor.value;
    saveToLocalStorage();
  });
  els.bgColor.addEventListener('input', () => {
    state.bgColor = els.bgColor.value;
    saveToLocalStorage();
    render();
  });
  els.lineWidth.addEventListener('input', () => {
    state.lineWidth = Number(els.lineWidth.value) || 4;
    els.lineWidthValue.textContent = `${state.lineWidth}px`;
    saveToLocalStorage();
  });
  els.lineStyle.addEventListener('change', () => {
    state.lineStyle = els.lineStyle.value;
    saveToLocalStorage();
  });
  els.animated.addEventListener('change', () => {
    state.animated = els.animated.checked;
    saveToLocalStorage();
    render();
  });
  els.textSize.addEventListener('change', () => {
    state.textSize = clamp(Number(els.textSize.value) || 32, 12, 128);
    els.textSize.value = String(state.textSize);
    saveToLocalStorage();
  });
  els.textContent.addEventListener('change', () => {
    state.textContent = els.textContent.value || 'Idea';
    saveToLocalStorage();
  });
  els.iconType.addEventListener('change', () => {
    state.iconType = els.iconType.value;
    saveToLocalStorage();
  });
  els.iconSize.addEventListener('change', () => {
    state.iconSize = clamp(Number(els.iconSize.value) || 88, 20, 240);
    els.iconSize.value = String(state.iconSize);
    saveToLocalStorage();
  });

  els.btnUndo.addEventListener('click', undo);
  els.btnRedo.addEventListener('click', redo);
  els.btnDelete.addEventListener('click', deleteSelected);
  els.btnClear.addEventListener('click', clearAll);
  els.btnCenter.addEventListener('click', centerView);
  els.btnFit.addEventListener('click', fitContent);
  els.btnArtboardAdd.addEventListener('click', addArtboard);
  els.btnArtboardDup.addEventListener('click', duplicateArtboard);
  els.btnArtboardRename.addEventListener('click', renameArtboard);
  els.btnArtboardDelete.addEventListener('click', deleteArtboard);
  els.btnGrid.addEventListener('click', () => {
    state.grid = !state.grid;
    saveToLocalStorage();
    render();
    setStatus(state.grid ? 'Grid enabled.' : 'Grid hidden.');
  });
  els.btnFocus.addEventListener('click', () => toggleFocusMode());
  els.btnFullscreen.addEventListener('click', toggleFullscreen);
  els.focusToggleBtn.addEventListener('click', () => toggleFocusMode(false));
  els.menuBtn.addEventListener('click', () => {
    document.body.classList.toggle('menu-open');
  });

  els.btnSaveJson.addEventListener('click', exportJson);
  els.btnLoadJson.addEventListener('click', () => els.fileLoader.click());
  els.fileLoader.addEventListener('change', () => {
    importJson(els.fileLoader.files[0]);
    els.fileLoader.value = '';
  });
  els.btnExportDsl.addEventListener('click', exportDslStarter);
  els.btnExportPng.addEventListener('click', exportPng);

  // Opacity slider
  els.opacity.addEventListener('input', () => {
    state.opacity = parseFloat(els.opacity.value) || 1;
    els.opacityValue.textContent = state.opacity.toFixed(2);
    saveToLocalStorage();
  });

  // Duplicate
  els.btnDuplicate.addEventListener('click', duplicateSelected);

  // Z-order
  els.btnBringFwd.addEventListener('click', bringForward);
  els.btnSendBack.addEventListener('click', sendBackward);

  // Selected-shape quick style edits
  els.btnLineSolid.addEventListener('click', () => setSelectedLineStyle('solid'));
  els.btnLineDashed.addEventListener('click', () => setSelectedLineStyle('dashed'));
  els.btnLineDotted.addEventListener('click', () => setSelectedLineStyle('dotted'));
  els.btnToggleArrow.addEventListener('click', toggleLineArrowSelection);
  els.btnRectRadius.addEventListener('click', editSelectedRectRadius);

  // Alignment
  els.btnAlignLeft.addEventListener('click', () => alignSelected('left'));
  els.btnAlignCenter.addEventListener('click', () => alignSelected('center'));
  els.btnAlignRight.addEventListener('click', () => alignSelected('right'));
  els.btnAlignTop.addEventListener('click', () => alignSelected('top'));
  els.btnAlignMiddle.addEventListener('click', () => alignSelected('middle'));
  els.btnAlignBottom.addEventListener('click', () => alignSelected('bottom'));
  els.btnDistH.addEventListener('click', () => distributeSelected('x'));
  els.btnDistV.addEventListener('click', () => distributeSelected('y'));

  // Pathfinder
  els.btnPathUnion.addEventListener('click', () => applyPathfinder('union'));
  els.btnPathIntersect.addEventListener('click', () => applyPathfinder('intersect'));
  els.btnPathSubtract.addEventListener('click', () => applyPathfinder('subtract'));

  // Snap toggle
  els.btnSnap.addEventListener('click', () => {
    state.snapGrid = !state.snapGrid;
    els.btnSnap.textContent = state.snapGrid ? 'Snap: On' : 'Snap: Off';
    saveToLocalStorage();
    render();
    setStatus(state.snapGrid ? 'Snap to grid enabled.' : 'Snap to grid disabled.');
  });

  // Shortcut overlay
  els.shortcutOverlay.addEventListener('click', (e) => {
    if (e.target === els.shortcutOverlay) toggleShortcutOverlay(false);
  });

  els.stage.addEventListener('pointerdown', (event) => {
    event.preventDefault();
    els.stage.setPointerCapture(event.pointerId);
    applyPointerDown(event);
  });
  els.stage.addEventListener('pointermove', (event) => {
    event.preventDefault();
    applyPointerMove(event);
  });
  els.stage.addEventListener('pointerup', (event) => {
    event.preventDefault();
    applyPointerUp();
    if (els.stage.hasPointerCapture(event.pointerId)) {
      els.stage.releasePointerCapture(event.pointerId);
    }
  });
  els.stage.addEventListener('pointercancel', applyPointerUp);
  els.stage.addEventListener('dblclick', (event) => {
    const world = toWorld(event.clientX, event.clientY);
    const hit = findShapeAt(world.x, world.y);
    if (!hit || hit.type !== 'rect') {
      return;
    }
    setSelection([hit.id]);
    editRectRadiusForShape(hit);
  });

  els.stage.addEventListener('wheel', (event) => {
    event.preventDefault();
    const factor = event.deltaY < 0 ? 1.1 : 0.9;
    const oldScale = state.camera.scale;
    const newScale = clamp(oldScale * factor, 0.15, 8);
    if (Math.abs(newScale - oldScale) < 0.0001) {
      return;
    }

    const worldBefore = toWorld(event.clientX, event.clientY);
    state.camera.scale = newScale;
    const worldAfter = toWorld(event.clientX, event.clientY);
    state.camera.x += worldBefore.x - worldAfter.x;
    state.camera.y += worldBefore.y - worldAfter.y;
    saveToLocalStorage();
    render();
  }, { passive: false });

  window.addEventListener('resize', resizeCanvas);
  window.addEventListener('keydown', (event) => {
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'z') {
      event.preventDefault();
      if (event.shiftKey) {
        redo();
      } else {
        undo();
      }
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'y') {
      event.preventDefault();
      redo();
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.shiftKey && event.key.toLowerCase() === 's') {
      event.preventDefault();
      exportDslStarter();
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') {
      event.preventDefault();
      exportJson();
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'o') {
      event.preventDefault();
      els.fileLoader.click();
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'd') {
      event.preventDefault();
      duplicateSelected();
      return;
    }

    if (event.key === 'Delete' || event.key === 'Backspace') {
      if (event.target && (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA')) {
        return;
      }
      event.preventDefault();
      deleteSelected();
      return;
    }

    if (event.key === ' ') {
      if (!event.repeat) {
        state.spacePan = true;
        if (!state.pointer.down) {
          els.stage.style.cursor = 'grab';
        }
      }
      return;
    }

    if (event.key.toLowerCase() === 'f') {
      if (event.target && (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA' || event.target.tagName === 'SELECT')) {
        return;
      }
      event.preventDefault();
      toggleFocusMode();
      return;
    }

    if (event.key.toLowerCase() === 'h') {
      if (event.target && (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA' || event.target.tagName === 'SELECT')) {
        return;
      }
      event.preventDefault();
      if (window.matchMedia('(max-width: 1024px)').matches) {
        document.body.classList.toggle('menu-open');
      } else {
        toggleFocusMode();
      }
      return;
    }

    if (event.key.toLowerCase() === 'g') {
      event.preventDefault();
      state.grid = !state.grid;
      saveToLocalStorage();
      render();
      return;
    }

    if (event.key.toLowerCase() === 's') {
      if (event.target && (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA' || event.target.tagName === 'SELECT')) return;
      event.preventDefault();
      state.snapGrid = !state.snapGrid;
      els.btnSnap.textContent = state.snapGrid ? 'Snap: On' : 'Snap: Off';
      saveToLocalStorage();
      render();
      setStatus(state.snapGrid ? 'Snap enabled.' : 'Snap disabled.');
      return;
    }

    if (event.key.toLowerCase() === 'l') {
      if (event.target && (event.target.tagName === 'INPUT' || event.target.tagName === 'TEXTAREA' || event.target.tagName === 'SELECT')) return;
      event.preventDefault();
      setTool('laser');
      return;
    }

    if (event.key === '?') {
      event.preventDefault();
      toggleShortcutOverlay();
      return;
    }

    if (event.key === '+' || event.key === '=') {
      state.camera.scale = clamp(state.camera.scale * 1.1, 0.15, 8);
      render();
      saveToLocalStorage();
      return;
    }

    if (event.key === '-') {
      state.camera.scale = clamp(state.camera.scale * 0.9, 0.15, 8);
      render();
      saveToLocalStorage();
      return;
    }

    if (event.key === '0') {
      setTool('pan');
      return;
    }

    const numeric = Number(event.key);
    if (!Number.isNaN(numeric) && numeric >= 1 && numeric <= 9) {
      const tool = TOOLS[numeric - 1];
      if (tool) {
        setTool(tool.id);
      }
    }
  });

  window.addEventListener('keyup', (event) => {
    if (event.key === ' ') {
      state.spacePan = false;
      if (!state.pointer.down) {
        setTool(state.tool);
      }
    }
  });

  // Pinch-to-zoom (trackpad / touch)
  let lastPinchDist = 0;
  els.stage.addEventListener('touchstart', (e) => {
    if (e.touches.length === 2) {
      lastPinchDist = Math.hypot(
        e.touches[0].clientX - e.touches[1].clientX,
        e.touches[0].clientY - e.touches[1].clientY
      );
    }
  }, { passive: true });
  els.stage.addEventListener('touchmove', (e) => {
    if (e.touches.length === 2) {
      e.preventDefault();
      const dist = Math.hypot(
        e.touches[0].clientX - e.touches[1].clientX,
        e.touches[0].clientY - e.touches[1].clientY
      );
      if (lastPinchDist > 0) {
        const factor = dist / lastPinchDist;
        const midX = (e.touches[0].clientX + e.touches[1].clientX) / 2;
        const midY = (e.touches[0].clientY + e.touches[1].clientY) / 2;
        const worldBefore = toWorld(midX, midY);
        state.camera.scale = clamp(state.camera.scale * factor, 0.15, 8);
        const worldAfter = toWorld(midX, midY);
        state.camera.x += worldBefore.x - worldAfter.x;
        state.camera.y += worldBefore.y - worldAfter.y;
        render();
      }
      lastPinchDist = dist;
    }
  }, { passive: false });
  els.stage.addEventListener('touchend', () => { lastPinchDist = 0; }, { passive: true });

  // Canvas leave — hide laser
  els.canvasWrap.addEventListener('mouseleave', () => {
    if (els.laserDot) els.laserDot.classList.remove('visible');
  });
}

/* ═══════════════════════════════
   New feature functions
   ═══════════════════════════════ */

// ── Color palette ──
function initPalette() {
  els.palette.innerHTML = '';
  PALETTE.forEach((c) => {
    const s = document.createElement('div');
    s.className = 'swatch';
    s.style.background = c;
    s.dataset.color = c;
    if (c === state.strokeColor) s.classList.add('active');
    s.addEventListener('click', () => {
      state.strokeColor = c;
      els.strokeColor.value = c;
      updatePalette();
      saveToLocalStorage();
    });
    els.palette.appendChild(s);
  });
}

function updatePalette() {
  els.palette.querySelectorAll('.swatch').forEach((s) => {
    s.classList.toggle('active', s.dataset.color === state.strokeColor);
  });
}

// ── Recent colors ──
function initRecentColors() {
  renderRecentColors();
}

function trackRecentColor(color) {
  if (!color) return;
  state.recentColors = state.recentColors.filter((c) => c !== color);
  state.recentColors.unshift(color);
  if (state.recentColors.length > 6) state.recentColors.length = 6;
  renderRecentColors();
  saveToLocalStorage();
}

function renderRecentColors() {
  if (!els.recentColors) return;
  els.recentColors.innerHTML = '';
  if (!state.recentColors.length) {
    els.recentColors.innerHTML = '<span style="font-size:10px;color:var(--muted)">Recent colors appear here</span>';
    return;
  }
  state.recentColors.forEach((c) => {
    const s = document.createElement('div');
    s.className = 'recent-swatch';
    s.style.background = c;
    s.title = c;
    s.addEventListener('click', () => {
      state.strokeColor = c;
      els.strokeColor.value = c;
      updatePalette();
      saveToLocalStorage();
    });
    els.recentColors.appendChild(s);
  });
}

function setSelectedLineStyle(style) {
  const targets = selectedShapes().filter((shape) => shape.type === 'line' || shape.type === 'arrow');
  if (!targets.length) {
    setStatus('Select at least one line/arrow first.');
    return;
  }
  for (const shape of targets) {
    shape.style = style;
  }
  pushHistory();
  render();
  setStatus(`Applied ${style} style.`);
}

function toggleLineArrowSelection() {
  const targets = selectedShapes().filter((shape) => shape.type === 'line' || shape.type === 'arrow');
  if (!targets.length) {
    setStatus('Select at least one line/arrow first.');
    return;
  }
  for (const shape of targets) {
    shape.type = shape.type === 'line' ? 'arrow' : 'line';
  }
  pushHistory();
  render();
  setStatus('Toggled line/arrow style.');
}

function editRectRadiusForShape(shape) {
  if (!shape || shape.type !== 'rect') {
    setStatus('Select a rectangle first.');
    return;
  }
  const current = Math.max(0, Number(shape.radius) || 0);
  const raw = window.prompt('Rectangle corner radius (px):', String(Math.round(current)));
  if (raw === null) {
    return;
  }
  const value = Number(raw);
  if (!Number.isFinite(value) || value < 0) {
    setStatus('Radius must be a number >= 0.');
    return;
  }
  shape.radius = clamp(value, 0, 2000);
  clampRectRadiusToBounds(shape);
  pushHistory();
  render();
  setStatus(`Rectangle radius set to ${Math.round(shape.radius)}px.`);
}

function editSelectedRectRadius() {
  const rect = selectedShapes().find((shape) => shape.type === 'rect');
  editRectRadiusForShape(rect || null);
}

function alignSelected(mode) {
  const validModes = new Set(['left', 'center', 'right', 'top', 'middle', 'bottom']);
  if (!validModes.has(mode)) {
    setStatus('Unknown align mode.');
    return;
  }

  const items = selectedShapes();
  if (items.length < 2) {
    setStatus('Select at least two shapes to align.');
    return;
  }

  const records = items.map((shape) => ({ shape, bounds: getBounds(shape) })).filter((r) => r.bounds);
  if (records.length < 2) {
    setStatus('Selected shapes are not alignable.');
    return;
  }

  const minX = Math.min(...records.map((r) => r.bounds.x));
  const minY = Math.min(...records.map((r) => r.bounds.y));
  const maxX = Math.max(...records.map((r) => r.bounds.x + r.bounds.w));
  const maxY = Math.max(...records.map((r) => r.bounds.y + r.bounds.h));
  const centerX = (minX + maxX) / 2;
  const centerY = (minY + maxY) / 2;

  for (const rec of records) {
    let dx = 0;
    let dy = 0;
    if (mode === 'left') dx = minX - rec.bounds.x;
    if (mode === 'center') dx = centerX - (rec.bounds.x + rec.bounds.w / 2);
    if (mode === 'right') dx = maxX - (rec.bounds.x + rec.bounds.w);
    if (mode === 'top') dy = minY - rec.bounds.y;
    if (mode === 'middle') dy = centerY - (rec.bounds.y + rec.bounds.h / 2);
    if (mode === 'bottom') dy = maxY - (rec.bounds.y + rec.bounds.h);
    moveShape(rec.shape, dx, dy);
  }

  pushHistory();
  render();
  const skipped = items.length - records.length;
  setStatus(skipped > 0
    ? `Aligned ${records.length} shape(s), skipped ${skipped}.`
    : `Aligned ${records.length} shapes (${mode}).`);
}

function distributeSelected(axis) {
  if (axis !== 'x' && axis !== 'y') {
    setStatus('Unknown distribute axis.');
    return;
  }

  const items = selectedShapes();
  if (items.length < 3) {
    setStatus('Select at least three shapes to distribute.');
    return;
  }

  const records = items.map((shape) => ({ shape, bounds: getBounds(shape) })).filter((r) => r.bounds);
  if (records.length < 3) {
    setStatus('Selected shapes are not distributable.');
    return;
  }

  if (axis === 'x') {
    records.sort((a, b) => (a.bounds.x + a.bounds.w / 2) - (b.bounds.x + b.bounds.w / 2));
  } else {
    records.sort((a, b) => (a.bounds.y + a.bounds.h / 2) - (b.bounds.y + b.bounds.h / 2));
  }

  const first = records[0].bounds;
  const last = records[records.length - 1].bounds;
  const firstCenter = axis === 'x' ? first.x + first.w / 2 : first.y + first.h / 2;
  const lastCenter = axis === 'x' ? last.x + last.w / 2 : last.y + last.h / 2;
  const step = (lastCenter - firstCenter) / (records.length - 1);

  for (let i = 1; i < records.length - 1; i += 1) {
    const rec = records[i];
    const center = axis === 'x' ? rec.bounds.x + rec.bounds.w / 2 : rec.bounds.y + rec.bounds.h / 2;
    const target = firstCenter + step * i;
    const delta = target - center;
    moveShape(rec.shape, axis === 'x' ? delta : 0, axis === 'y' ? delta : 0);
  }

  pushHistory();
  render();
  const skipped = items.length - records.length;
  const base = axis === 'x' ? 'Distributed horizontally.' : 'Distributed vertically.';
  setStatus(skipped > 0 ? `${base} Skipped ${skipped} shape(s).` : base);
}

// Pathfinder logic is provided by the pathfinder controller module.

// ── Duplicate selected ──
function duplicateSelected() {
  const originals = selectedShapes();
  if (!originals.length) {
    setStatus('Nothing selected to duplicate.');
    return;
  }
  const selectedCloneIds = [];
  const offset = 24 / state.camera.scale;
  for (const original of originals) {
    const clone = JSON.parse(JSON.stringify(original));
    clone.id = uid();
    if (clone.type === 'pen') {
      for (const p of clone.points || []) { p.x += offset; p.y += offset; }
    } else if (typeof clone.x1 === 'number') {
      clone.x1 += offset; clone.y1 += offset;
      clone.x2 += offset; clone.y2 += offset;
    } else if (typeof clone.x === 'number') {
      clone.x += offset; clone.y += offset;
    }
    state.shapes.push(clone);
    selectedCloneIds.push(clone.id);
  }
  setSelection(selectedCloneIds);
  pushHistory();
  setStatus(`Duplicated ${selectedCloneIds.length} shape(s).`);
}

// ── Z-order ──
function bringForward() {
  const ids = selectedShapeIds();
  if (!ids.length) {
    setStatus('Select at least one shape first.');
    return;
  }
  const selectedSet = new Set(ids);
  let moved = false;
  for (let i = state.shapes.length - 2; i >= 0; i -= 1) {
    const curr = state.shapes[i];
    const next = state.shapes[i + 1];
    if (selectedSet.has(curr.id) && !selectedSet.has(next.id)) {
      [state.shapes[i], state.shapes[i + 1]] = [state.shapes[i + 1], state.shapes[i]];
      moved = true;
    }
  }
  if (!moved) {
    setStatus('Selection is already at the front.');
    return;
  }
  pushHistory();
  render();
  setStatus(`Brought ${ids.length} shape(s) forward.`);
}

function sendBackward() {
  const ids = selectedShapeIds();
  if (!ids.length) {
    setStatus('Select at least one shape first.');
    return;
  }
  const selectedSet = new Set(ids);
  let moved = false;
  for (let i = 1; i < state.shapes.length; i += 1) {
    const prev = state.shapes[i - 1];
    const curr = state.shapes[i];
    if (!selectedSet.has(prev.id) && selectedSet.has(curr.id)) {
      [state.shapes[i - 1], state.shapes[i]] = [state.shapes[i], state.shapes[i - 1]];
      moved = true;
    }
  }
  if (!moved) {
    setStatus('Selection is already at the back.');
    return;
  }
  pushHistory();
  render();
  setStatus(`Sent ${ids.length} shape(s) backward.`);
}

// ── Laser pointer ──
function showLaser(clientX, clientY) {
  const rect = els.canvasWrap.getBoundingClientRect();
  const x = clientX - rect.left;
  const y = clientY - rect.top;
  els.laserDot.style.left = x + 'px';
  els.laserDot.style.top = y + 'px';
  els.laserDot.classList.add('visible');

  // Spawn a fading trail dot
  const trail = document.createElement('div');
  trail.className = 'laser-trail';
  trail.style.left = x + 'px';
  trail.style.top = y + 'px';
  trail.style.width = '12px';
  trail.style.height = '12px';
  trail.style.opacity = '0.6';
  els.canvasWrap.appendChild(trail);
  requestAnimationFrame(() => {
    trail.style.transition = 'opacity 0.5s ease-out';
    trail.style.opacity = '0';
  });
  setTimeout(() => { trail.remove(); }, 550);
}

// ── Shortcut overlay ──
function toggleShortcutOverlay(force) {
  const open = typeof force === 'boolean' ? force : !els.shortcutOverlay.classList.contains('open');
  els.shortcutOverlay.classList.toggle('open', open);
}

function initControllers() {
  const selectionController = createSelectionController({
    state,
    getBounds,
    setStatus,
    ctx
  });
  ({ setSelection, clearSelection, selectedShapeIds, selectedShapes, finalizeMarqueeSelection, drawMarqueeOverlay } = selectionController);

  const handleController = createHandleController({
    state,
    selectedShapes,
    setSelection,
    snapPoint,
    constrainEndpoint,
    clamp,
    ctx
  });
  ({
    clampRectRadiusToBounds,
    findEditHandleAt,
    drawEditHandlesOverlay,
    beginHandleDrag,
    applyHandleDrag
  } = handleController);

  const renderController = createRenderController({
    state,
    els,
    ctx,
    DPR,
    TOOLS,
    currentArtboard,
    drawGrid,
    drawShape,
    selectedShapeIds,
    drawMarqueeOverlay,
    drawEditHandlesOverlay
  });
  ({ render } = renderController);

  const historyController = createHistoryController({
    state,
    DEFAULT_BG,
    sanitizeArtboards,
    clearSelection,
    renderArtboardTabs,
    render,
    syncControls,
    saveToLocalStorage,
    setStatus
  });
  ({ pushHistory, undo, redo } = historyController);

  const pathfinderController = createPathfinderController({
    state,
    uid,
    getBounds,
    selectedShapeIds,
    setSelection,
    pushHistory,
    render,
    setStatus
  });
  ({ applyPathfinder } = pathfinderController);
}

function init() {
  initControllers();
  initPalette();
  initRecentColors();
  initTools();
  const restored = loadFromLocalStorage();
  if (!restored) {
    pushHistory();
    setStatus('New canvas ready.');
  }
  syncControls();
  setupBindings();
  resizeCanvas();
  requestAnimationFrame(animate);
}

init();
