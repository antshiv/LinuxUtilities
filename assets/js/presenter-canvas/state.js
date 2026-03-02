export const TOOLS = [
  { id: 'select', label: 'Select', hotkey: '1', icon: '▣' },
  { id: 'pen', label: 'Pen', hotkey: '2', icon: '✎' },
  { id: 'line', label: 'Line', hotkey: '3', icon: '╱' },
  { id: 'arrow', label: 'Arrow', hotkey: '4', icon: '➜' },
  { id: 'rect', label: 'Rect', hotkey: '5', icon: '▭' },
  { id: 'ellipse', label: 'Ellipse', hotkey: '6', icon: '◯' },
  { id: 'text', label: 'Text', hotkey: '7', icon: 'T' },
  { id: 'icon', label: 'Icon', hotkey: '8', icon: '⌁' },
  { id: 'eraser', label: 'Eraser', hotkey: '9', icon: '⌫' },
  { id: 'laser', label: 'Laser', hotkey: 'L', icon: '◎' },
  { id: 'pan', label: 'Pan', hotkey: '0', icon: '✥' }
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
  // Prefer platform UUIDs for collision-safe ids across sessions/imports.
  if (typeof crypto !== 'undefined' && typeof crypto.randomUUID === 'function') {
    return crypto.randomUUID();
  }
  return `${Date.now().toString(36)}-${Math.random().toString(36).slice(2, 10)}`;
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
      handleDrag: null,
      selectionDrag: null
    },
    snapGuides: { x: null, y: null },
    history: [],
    historyIndex: -1,
    dashTick: 0,
    drawAnimDurationMs: 420,
    opacity: 1,
    fillEnabled: false,
    fillColor: '#6ec8ff',
    snapGrid: false,
    recentColors: [],
    laserTrails: [],
    panelCollapsed: {
      layers: false,
      shapes: false,
      timeline: false,
      tools: false,
      style: false,
      canvas: false,
      selected: true,
      align: true,
      pathfinder: true,
      save: false,
      notes: true
    },
    timeline: {
      currentTime: 0,
      duration: 30,
      playing: false,
      lastTickMs: 0,
      zoomPxPerSec: 120,
      snapSec: 0.25,
      captionStyle: {
        template: 'karaoke_classic',
        mode: 'karaoke',
        textColor: '#f7fbff',
        strokeColor: '#05070d',
        strokeWidth: 4,
        highlightColor: '#ffe05d',
        highlightTextColor: '#121315',
        highlightFontFamily: '',
        highlightFontWeight: 800,
        highlightPadY: 0,
        highlightPadX: 3,
        highlightRadius: 3,
        overlayBgColor: '#0b1018',
        overlayBgOpacity: 0.76,
        fontSize: 22,
        fontWeight: 800,
        lineHeight: 1.34,
        letterSpacing: 0,
        wordHighlight: true
      },
      transcriptSegments: [],
      transcriptName: '',
      audioName: ''
    }
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
