export const TOOLS = [
  { id: 'select', label: 'Select', hotkey: '1' },
  { id: 'pen', label: 'Pen', hotkey: '2' },
  { id: 'line', label: 'Line', hotkey: '3' },
  { id: 'arrow', label: 'Arrow', hotkey: '4' },
  { id: 'rect', label: 'Rect', hotkey: '5' },
  { id: 'ellipse', label: 'Ellipse', hotkey: '6' },
  { id: 'text', label: 'Text', hotkey: '7' },
  { id: 'icon', label: 'Icon', hotkey: '8' },
  { id: 'eraser', label: 'Eraser', hotkey: '9' },
  { id: 'laser', label: 'Laser', hotkey: 'L' },
  { id: 'pan', label: 'Pan', hotkey: '0' }
];

export const STORAGE_KEY = 'linuxutilities.presenter_canvas.v1';

export const PALETTE = [
  '#6ec8ff', '#9a87ff', '#ff7b9c', '#76e8c6',
  '#ffb86c', '#f1fa8c', '#ffffff', '#99a9cd'
];

export const DEFAULT_BG = '#06080f';
export const GRID_SNAP = 48;

export function clamp(val, min, max) {
  return Math.max(min, Math.min(max, val));
}

export function uid() {
  return `${Date.now()}-${Math.floor(Math.random() * 1e6)}`;
}

export function makeArtboard(name, base) {
  const fallback = {
    name: name || 'Artboard',
    bgColor: DEFAULT_BG,
    camera: { x: 0, y: 0, scale: 1 },
    shapes: []
  };
  const source = base || fallback;
  return {
    id: uid(),
    name: source.name || name || 'Artboard',
    bgColor: source.bgColor || DEFAULT_BG,
    camera: source.camera ? {
      x: Number(source.camera.x) || 0,
      y: Number(source.camera.y) || 0,
      scale: clamp(Number(source.camera.scale) || 1, 0.15, 8)
    } : { x: 0, y: 0, scale: 1 },
    shapes: Array.isArray(source.shapes) ? JSON.parse(JSON.stringify(source.shapes)) : []
  };
}

export function sanitizeArtboards(raw, fallback) {
  if (!Array.isArray(raw) || !raw.length) {
    return [makeArtboard('Artboard 1', fallback)];
  }
  const list = [];
  for (let i = 0; i < raw.length; i += 1) {
    const candidate = raw[i] || {};
    list.push({
      id: candidate.id || uid(),
      name: candidate.name || `Artboard ${i + 1}`,
      bgColor: candidate.bgColor || DEFAULT_BG,
      camera: {
        x: Number(candidate.camera && candidate.camera.x) || 0,
        y: Number(candidate.camera && candidate.camera.y) || 0,
        scale: clamp(Number(candidate.camera && candidate.camera.scale) || 1, 0.15, 8)
      },
      shapes: Array.isArray(candidate.shapes) ? candidate.shapes : []
    });
  }
  return list;
}

export function createInitialState() {
  return {
    tool: 'pen',
    lineWidth: 4,
    strokeColor: '#6ec8ff',
    lineStyle: 'solid',
    animated: false,
    textSize: 34,
    textContent: 'Idea',
    iconType: 'server',
    iconSize: 88,
    grid: true,
    artboards: [makeArtboard('Artboard 1')],
    activeArtboardId: null,
    selectedId: null,
    selectedIds: [],
    pointer: {
      down: false,
      drawing: false,
      start: null,
      current: null,
      temp: null,
      lastWorld: null,
      marquee: null,
      handleDrag: null
    },
    history: [],
    historyIndex: -1,
    dashTick: 0,
    drawAnimDurationMs: 420,
    opacity: 1,
    snapGrid: false,
    recentColors: [],
    laserTrails: []
  };
}

export function attachStateAccessors(state, currentArtboard) {
  Object.defineProperty(state, 'shapes', {
    configurable: true,
    enumerable: true,
    get() {
      return currentArtboard().shapes;
    },
    set(value) {
      currentArtboard().shapes = Array.isArray(value) ? value : [];
    }
  });

  Object.defineProperty(state, 'bgColor', {
    configurable: true,
    enumerable: true,
    get() {
      return currentArtboard().bgColor;
    },
    set(value) {
      currentArtboard().bgColor = value || DEFAULT_BG;
    }
  });

  Object.defineProperty(state, 'camera', {
    configurable: true,
    enumerable: true,
    get() {
      return currentArtboard().camera;
    },
    set(value) {
      const next = value || {};
      currentArtboard().camera = {
        x: Number(next.x) || 0,
        y: Number(next.y) || 0,
        scale: clamp(Number(next.scale) || 1, 0.15, 8)
      };
    }
  });
}
