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
const COLLAPSIBLE_PANEL_KEYS = [
  'layers',
  'shapes',
  'script',
  'timeline',
  'tools',
  'style',
  'canvas',
  'selected',
  'align',
  'pathfinder',
  'save',
  'format',
  'notes'
];
const COLLAPSIBLE_PANEL_LABELS = {
  layers: 'Layers',
  shapes: 'Shapes',
  script: 'Script Editor',
  timeline: 'Timeline',
  tools: 'Tools',
  style: 'Style',
  canvas: 'Canvas',
  selected: 'Selected Shape',
  align: 'Align',
  pathfinder: 'Pathfinder',
  save: 'Save / Load',
  format: 'Format / Shorts',
  notes: 'Recording Notes'
};
const PANEL_COLLAPSE_DEFAULTS = {
  layers: false,
  shapes: false,
  script: false,
  timeline: false,
  tools: false,
  style: false,
  canvas: false,
  selected: true,
  align: true,
  pathfinder: true,
  save: false,
  format: true,
  notes: true
};

const INLINE_ICONS = {
  artboardAdd: '<svg viewBox="0 0 16 16"><rect x="2.5" y="2.5" width="11" height="11" rx="1.5"/><path d="M8 5v6"/><path d="M5 8h6"/></svg>',
  duplicate: '<svg viewBox="0 0 16 16"><rect x="3.5" y="5.5" width="7" height="7" rx="1.2"/><rect x="6.5" y="2.5" width="7" height="7" rx="1.2"/></svg>',
  edit: '<svg viewBox="0 0 16 16"><path d="M3 11.8 3.7 9l6.6-6.6 2.6 2.6L6.3 11.6z"/><path d="M9.9 2.8 12.6 5.5"/></svg>',
  trash: '<svg viewBox="0 0 16 16"><path d="M3.5 4.5h9"/><path d="M6 4.5v-1h4v1"/><path d="M5.2 4.5v7.5h5.6V4.5"/><path d="M7 6.5v4"/><path d="M9 6.5v4"/></svg>',
  arrowUp: '<svg viewBox="0 0 16 16"><path d="M8 13V3"/><path d="M4.5 6.5 8 3l3.5 3.5"/></svg>',
  arrowDown: '<svg viewBox="0 0 16 16"><path d="M8 3v10"/><path d="M4.5 9.5 8 13l3.5-3.5"/></svg>',
  group: '<svg viewBox="0 0 16 16"><rect x="2.5" y="2.5" width="5" height="5" rx="1"/><rect x="8.5" y="8.5" width="5" height="5" rx="1"/><path d="M6.5 6.5h3"/><path d="M9.5 6.5v3"/></svg>',
  ungroup: '<svg viewBox="0 0 16 16"><rect x="2.5" y="2.5" width="5" height="5" rx="1"/><rect x="8.5" y="8.5" width="5" height="5" rx="1"/><path d="M6.5 6.5h3"/><path d="M11.8 2.8h1.7"/><path d="M2.5 11.5h1.7"/></svg>',
  eye: '<svg viewBox="0 0 16 16"><path d="M1.5 8s2.2-3.5 6.5-3.5S14.5 8 14.5 8 12.3 11.5 8 11.5 1.5 8 1.5 8Z"/><circle cx="8" cy="8" r="2"/></svg>',
  eyeOff: '<svg viewBox="0 0 16 16"><path d="M1.5 8s2.2-3.5 6.5-3.5S14.5 8 14.5 8 12.3 11.5 8 11.5 1.5 8 1.5 8Z"/><path d="M2.4 13.6 13.6 2.4"/></svg>',
  lock: '<svg viewBox="0 0 16 16"><rect x="3.5" y="7" width="9" height="6.5" rx="1.3"/><path d="M5.5 7V5.8a2.5 2.5 0 0 1 5 0V7"/></svg>',
  unlock: '<svg viewBox="0 0 16 16"><rect x="3.5" y="7" width="9" height="6.5" rx="1.3"/><path d="M9.8 3.8a2.5 2.5 0 0 1 2.2 2.5V7"/></svg>',
  alignLeft: '<svg viewBox="0 0 16 16"><path d="M3 2.5v11"/><path d="M5.5 4.5h7"/><path d="M5.5 8h5.5"/><path d="M5.5 11.5h7"/></svg>',
  alignCenter: '<svg viewBox="0 0 16 16"><path d="M8 2.5v11"/><path d="M3.5 4.5h9"/><path d="M5 8h6"/><path d="M3.5 11.5h9"/></svg>',
  alignRight: '<svg viewBox="0 0 16 16"><path d="M13 2.5v11"/><path d="M3.5 4.5h7"/><path d="M5 8h5.5"/><path d="M3.5 11.5h7"/></svg>',
  alignTop: '<svg viewBox="0 0 16 16"><path d="M2.5 3h11"/><path d="M4.5 5.5v7"/><path d="M8 5.5v5.5"/><path d="M11.5 5.5v7"/></svg>',
  alignMiddle: '<svg viewBox="0 0 16 16"><path d="M2.5 8h11"/><path d="M4.5 3.5v9"/><path d="M8 5v6"/><path d="M11.5 3.5v9"/></svg>',
  alignBottom: '<svg viewBox="0 0 16 16"><path d="M2.5 13h11"/><path d="M4.5 3.5v7"/><path d="M8 5v5.5"/><path d="M11.5 3.5v7"/></svg>',
  distributeH: '<svg viewBox="0 0 16 16"><path d="M2.5 3v10"/><path d="M13.5 3v10"/><rect x="5.2" y="5.2" width="2.2" height="5.6" rx="0.5"/><rect x="8.6" y="5.2" width="2.2" height="5.6" rx="0.5"/></svg>',
  distributeV: '<svg viewBox="0 0 16 16"><path d="M3 2.5h10"/><path d="M3 13.5h10"/><rect x="5.2" y="5.2" width="5.6" height="2.2" rx="0.5"/><rect x="5.2" y="8.6" width="5.6" height="2.2" rx="0.5"/></svg>',
  rect: '<svg viewBox="0 0 16 16"><rect x="2.5" y="3.5" width="11" height="9" rx="1.5"/></svg>',
  ellipse: '<svg viewBox="0 0 16 16"><ellipse cx="8" cy="8" rx="5.5" ry="4.5"/></svg>',
  line: '<svg viewBox="0 0 16 16"><path d="M3 12.5 13 3.5"/></svg>',
  arrow: '<svg viewBox="0 0 16 16"><path d="M3 12.5 12.5 3"/><path d="M9 3h3.5v3.5"/></svg>',
  text: '<svg viewBox="0 0 16 16"><path d="M3 3.5h10"/><path d="M8 3.5v9"/><path d="M5 12.5h6"/></svg>',
  icon: '<svg viewBox="0 0 16 16"><path d="M8 2.5 9.6 6l3.9.4-2.9 2.5.8 3.8L8 10.8l-3.4 1.9.8-3.8L2.5 6.4 6.4 6z"/></svg>'
};

const BUTTON_ICON_LABELS = {
  btnArtboardAdd: { iconSvg: INLINE_ICONS.artboardAdd },
  btnArtboardDup: { iconSvg: INLINE_ICONS.duplicate },
  btnArtboardRename: { iconSvg: INLINE_ICONS.edit },
  btnArtboardDelete: { iconSvg: INLINE_ICONS.trash },
  btnLayerUp: { iconSvg: INLINE_ICONS.arrowUp },
  btnLayerDown: { iconSvg: INLINE_ICONS.arrowDown },
  btnGroupSelection: { iconSvg: INLINE_ICONS.group },
  btnUngroupSelection: { iconSvg: INLINE_ICONS.ungroup },
  btnHideSelection: { iconSvg: INLINE_ICONS.eyeOff },
  btnShowSelection: { iconSvg: INLINE_ICONS.eye },
  btnLockSelection: { iconSvg: INLINE_ICONS.lock },
  btnUnlockSelection: { iconSvg: INLINE_ICONS.unlock },
  btnAddRect: { iconSvg: INLINE_ICONS.rect },
  btnAddEllipse: { iconSvg: INLINE_ICONS.ellipse },
  btnAddLine: { iconSvg: INLINE_ICONS.line },
  btnAddArrow: { iconSvg: INLINE_ICONS.arrow },
  btnAddText: { iconSvg: INLINE_ICONS.text },
  btnAddIcon: { iconSvg: INLINE_ICONS.icon },
  btnTimelineStop: { icon: '■' },
  btnTimelineLoadTranscript: { icon: '≣', label: 'Load Transcript' },
  btnTimelineLoadAudio: { icon: '♪', label: 'Load Audio' },
  btnTimelineMicRecord: { icon: '◉', label: 'Record Mic' },
  btnTimelineAddCaption: { icon: '+T', label: 'Add Caption At Time' },
  btnCaptionApplyAll: { icon: '◎', label: 'Apply To All Clips' },
  btnCaptionApplySelected: { icon: '◉', label: 'Apply To Selected Clip(s)' },
  btnUndo: { icon: '↺' },
  btnRedo: { icon: '↻' },
  btnDelete: { icon: '⌦' },
  btnDuplicate: { icon: '⧉' },
  btnBringFwd: { icon: '↑', label: 'Bring Fwd' },
  btnSendBack: { icon: '↓', label: 'Send Back' },
  btnClear: { icon: '✕' },
  btnCenter: { icon: '⌖', label: 'Center View' },
  btnFit: { icon: '⤢', label: 'Fit Content' },
  btnGrid: { icon: '⌗', label: 'Toggle Grid' },
  btnFocus: { icon: '◴', label: 'Focus Mode' },
  btnFullscreen: { icon: '⛶', label: 'Fullscreen' },
  btnLineSolid: { icon: '─', label: 'Solid' },
  btnLineDashed: { icon: '╌', label: 'Dashed' },
  btnLineDotted: { icon: '┈', label: 'Dotted' },
  btnToggleArrow: { icon: '↔', label: 'Line ↔ Arrow' },
  btnRectRadius: { icon: '◧', label: 'Set Rect Radius' },
  btnMotionInsertKeyframe: { icon: '◆', label: 'Insert KF' },
  btnMotionSetKeyframe: { icon: '◎', label: 'Set KF' },
  btnMotionDeleteKeyframe: { icon: '◇', label: 'Delete KF' },
  btnAlignLeft: { iconSvg: INLINE_ICONS.alignLeft, label: 'Left' },
  btnAlignCenter: { iconSvg: INLINE_ICONS.alignCenter, label: 'Center' },
  btnAlignRight: { iconSvg: INLINE_ICONS.alignRight, label: 'Right' },
  btnAlignTop: { iconSvg: INLINE_ICONS.alignTop, label: 'Top' },
  btnAlignMiddle: { iconSvg: INLINE_ICONS.alignMiddle, label: 'Middle' },
  btnAlignBottom: { iconSvg: INLINE_ICONS.alignBottom, label: 'Bottom' },
  btnDistH: { iconSvg: INLINE_ICONS.distributeH, label: 'Distribute H' },
  btnDistV: { iconSvg: INLINE_ICONS.distributeV, label: 'Distribute V' },
  btnPathUnion: { icon: '∪', label: 'Union' },
  btnPathIntersect: { icon: '∩', label: 'Intersect' },
  btnPathSubtract: { icon: '−', label: 'Subtract' },
  btnSaveJson: { icon: '↓', label: 'Save JSON' },
  btnLoadJson: { icon: '↑', label: 'Load JSON' },
  btnExportDsl: { icon: '⇄', label: 'Export DSL Starter' },
  btnExportPng: { icon: '⤓', label: 'Export PNG' },
  btnTimelineDockStop: { icon: '■', label: 'Stop' },
  btnTimelineInsertKeyframe: { icon: '◆', label: 'Insert KF' },
  btnTimelineSplit: { icon: '⫶', label: 'Split' },
  btnTimelineDeleteClip: { icon: '⌦', label: 'Delete Clip' }
};

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
let renderNow;

const els = {
  stage: document.getElementById('stage'),
  canvasWrap: document.getElementById('canvasWrap'),
  artboardTabs: document.getElementById('artboardTabs'),
  btnArtboardAdd: document.getElementById('btnArtboardAdd'),
  btnArtboardDup: document.getElementById('btnArtboardDup'),
  btnArtboardRename: document.getElementById('btnArtboardRename'),
  btnArtboardDelete: document.getElementById('btnArtboardDelete'),
  layerList: document.getElementById('layerList'),
  btnLayerUp: document.getElementById('btnLayerUp'),
  btnLayerDown: document.getElementById('btnLayerDown'),
  btnGroupSelection: document.getElementById('btnGroupSelection'),
  btnUngroupSelection: document.getElementById('btnUngroupSelection'),
  btnHideSelection: document.getElementById('btnHideSelection'),
  btnShowSelection: document.getElementById('btnShowSelection'),
  btnLockSelection: document.getElementById('btnLockSelection'),
  btnUnlockSelection: document.getElementById('btnUnlockSelection'),
  btnAddRect: document.getElementById('btnAddRect'),
  btnAddEllipse: document.getElementById('btnAddEllipse'),
  btnAddLine: document.getElementById('btnAddLine'),
  btnAddArrow: document.getElementById('btnAddArrow'),
  btnAddText: document.getElementById('btnAddText'),
  btnAddIcon: document.getElementById('btnAddIcon'),
  btnTimelinePlay: document.getElementById('btnTimelinePlay'),
  btnTimelineStop: document.getElementById('btnTimelineStop'),
  btnTimelineLoadTranscript: document.getElementById('btnTimelineLoadTranscript'),
  btnTimelineLoadAudio: document.getElementById('btnTimelineLoadAudio'),
  btnTimelineMicRecord: document.getElementById('btnTimelineMicRecord'),
  btnTimelineAddCaption: document.getElementById('btnTimelineAddCaption'),
  timelineScrub: document.getElementById('timelineScrub'),
  timelineTimeLabel: document.getElementById('timelineTimeLabel'),
  transcriptNow: document.getElementById('transcriptNow'),
  transcriptList: document.getElementById('transcriptList'),
  captionTemplateList: document.getElementById('captionTemplateList'),
  btnCaptionApplyAll: document.getElementById('btnCaptionApplyAll'),
  btnCaptionApplySelected: document.getElementById('btnCaptionApplySelected'),
  captionCustomControls: document.getElementById('captionCustomControls'),
  captionTextColor: document.getElementById('captionTextColor'),
  captionStrokeColor: document.getElementById('captionStrokeColor'),
  captionHighlightColor: document.getElementById('captionHighlightColor'),
  captionHighlightTextColor: document.getElementById('captionHighlightTextColor'),
  captionHighlightFontFamily: document.getElementById('captionHighlightFontFamily'),
  captionHighlightFontWeight: document.getElementById('captionHighlightFontWeight'),
  captionHighlightPadY: document.getElementById('captionHighlightPadY'),
  captionHighlightPadX: document.getElementById('captionHighlightPadX'),
  captionHighlightRadius: document.getElementById('captionHighlightRadius'),
  captionOverlayBg: document.getElementById('captionOverlayBg'),
  captionFontSize: document.getElementById('captionFontSize'),
  captionStrokeWidth: document.getElementById('captionStrokeWidth'),
  captionLineHeight: document.getElementById('captionLineHeight'),
  captionLetterSpacing: document.getElementById('captionLetterSpacing'),
  captionOverlayOpacity: document.getElementById('captionOverlayOpacity'),
  captionActionMode: document.getElementById('captionActionMode'),
  captionFontWeight: document.getElementById('captionFontWeight'),
  captionWordHighlight: document.getElementById('captionWordHighlight'),
  captionFontFamily: document.getElementById('captionFontFamily'),
  timelineTranscriptLoader: document.getElementById('timelineTranscriptLoader'),
  timelineAudioLoader: document.getElementById('timelineAudioLoader'),
  liveTranscriptOverlay: document.getElementById('liveTranscriptOverlay'),
  timelineDock: document.getElementById('timelineDock'),
  btnTimelineDockPlay: document.getElementById('btnTimelineDockPlay'),
  btnTimelineDockStop: document.getElementById('btnTimelineDockStop'),
  btnTimelineInsertKeyframe: document.getElementById('btnTimelineInsertKeyframe'),
  btnTimelineSplit: document.getElementById('btnTimelineSplit'),
  btnTimelineDeleteClip: document.getElementById('btnTimelineDeleteClip'),
  timelineSnap: document.getElementById('timelineSnap'),
  timelineZoom: document.getElementById('timelineZoom'),
  timelineDockScrub: document.getElementById('timelineDockScrub'),
  timelineDockTime: document.getElementById('timelineDockTime'),
  timelineEditorScroll: document.getElementById('timelineEditorScroll'),
  timelineEditorContent: document.getElementById('timelineEditorContent'),
  timelineRulerBody: document.getElementById('timelineRulerBody'),
  timelineTrackAudioBody: document.getElementById('timelineTrackAudioBody'),
  timelineTrackShapeBody: document.getElementById('timelineTrackShapeBody'),
  timelineTrackCaptionBody: document.getElementById('timelineTrackCaptionBody'),
  timelinePlayhead: document.getElementById('timelinePlayhead'),
  toolGrid: document.getElementById('toolGrid'),
  strokeColor: document.getElementById('strokeColor'),
  bgColor: document.getElementById('bgColor'),
  fillEnabled: document.getElementById('fillEnabled'),
  fillColor: document.getElementById('fillColor'),
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
  captionFloatPanel: document.getElementById('captionFloatPanel'),
  cfpBackdrop: document.getElementById('cfpBackdrop'),
  cfpScroll: document.getElementById('cfpScroll'),
  btnOpenCaptionStyles: document.getElementById('btnOpenCaptionStyles'),
  btnCaptionPanelClose: document.getElementById('btnCaptionPanelClose'),
  btnCfpSelected: document.getElementById('btnCfpSelected'),
  btnCfpAll: document.getElementById('btnCfpAll'),
  btnSaveJson: document.getElementById('btnSaveJson'),
  btnLoadJson: document.getElementById('btnLoadJson'),
  btnExportDsl: document.getElementById('btnExportDsl'),
  btnExportPng: document.getElementById('btnExportPng'),
  btnExportSRT: document.getElementById('btnExportSRT'),
  canvasTextEditor: document.getElementById('canvasTextEditor'),
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
  motionStart: document.getElementById('motionStart'),
  motionDuration: document.getElementById('motionDuration'),
  motionFromX: document.getElementById('motionFromX'),
  motionFromY: document.getElementById('motionFromY'),
  motionToX: document.getElementById('motionToX'),
  motionToY: document.getElementById('motionToY'),
  motionFromOpacity: document.getElementById('motionFromOpacity'),
  motionToOpacity: document.getElementById('motionToOpacity'),
  motionTrimStart: document.getElementById('motionTrimStart'),
  motionTrimEnd: document.getElementById('motionTrimEnd'),
  motionEase: document.getElementById('motionEase'),
  btnMotionInsertKeyframe: document.getElementById('btnMotionInsertKeyframe'),
  btnMotionSetKeyframe: document.getElementById('btnMotionSetKeyframe'),
  btnMotionDeleteKeyframe: document.getElementById('btnMotionDeleteKeyframe'),
  btnMotionApply: document.getElementById('btnMotionApply'),
  btnMotionClear: document.getElementById('btnMotionClear'),
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
  btnPresent: document.getElementById('btnPresent'),
  teleprompter: document.getElementById('teleprompter'),
  tpCurrent: document.getElementById('tpCurrent'),
  tpNext: document.getElementById('tpNext'),
  recBadge: document.getElementById('recBadge'),
  recTime: document.getElementById('recTime'),
  countdownOverlay: document.getElementById('countdownOverlay'),
  countdownNum: document.getElementById('countdownNum'),
  menuBtn: document.getElementById('menuBtn'),
  scriptEditorText: document.getElementById('scriptEditorText'),
  scriptEditorStat: document.getElementById('scriptEditorStat'),
  scriptSplitMode: document.getElementById('scriptSplitMode'),
  scriptWordCountRow: document.getElementById('scriptWordCountRow'),
  scriptWordsPerCap: document.getElementById('scriptWordsPerCap'),
  scriptWPM: document.getElementById('scriptWPM'),
  scriptWpmValue: document.getElementById('scriptWpmValue'),
  btnScriptGenerate: document.getElementById('btnScriptGenerate'),
  btnScriptAppend: document.getElementById('btnScriptAppend'),
  btnScriptClear: document.getElementById('btnScriptClear'),
  btnScriptFromTranscript: document.getElementById('btnScriptFromTranscript'),
  formatGuide: document.getElementById('formatGuide'),
  captionZoneIndicator: document.getElementById('captionZoneIndicator'),
  webcamFeed: document.getElementById('webcamFeed'),
  btnFormat16x9: document.getElementById('btnFormat16x9'),
  btnFormat9x16: document.getElementById('btnFormat9x16'),
  btnFormat1x1: document.getElementById('btnFormat1x1'),
  btnCaptionPosTop: document.getElementById('btnCaptionPosTop'),
  btnCaptionPosCenter: document.getElementById('btnCaptionPosCenter'),
  btnCaptionPosBottom: document.getElementById('btnCaptionPosBottom'),
  captionOffsetPct: document.getElementById('captionOffsetPct'),
  captionOffsetPctValue: document.getElementById('captionOffsetPctValue'),
  captionZoneVisible: document.getElementById('captionZoneVisible'),
  btnWebcamToggle: document.getElementById('btnWebcamToggle'),
  webcamPosition: document.getElementById('webcamPosition'),
  webcamShape: document.getElementById('webcamShape'),
  webcamSize: document.getElementById('webcamSize'),
  webcamSizeValue: document.getElementById('webcamSizeValue')
};

const ctx = els.stage.getContext('2d');
const DPR = Math.max(1, Math.min(2, window.devicePixelRatio || 1));
const timelineRuntime = {
  audio: null,
  audioUrl: '',
  activeIndex: -1,
  micRecorder: null,
  micStream: null,
  micChunks: [],
  micFileExt: 'webm'
};
const TIMELINE_MIN_CLIP_SEC = 0.08;
const TIMELINE_LABEL_WIDTH = 78;
const TIMELINE_MIN_PX_PER_SEC = 50;
const TIMELINE_MAX_PX_PER_SEC = 260;
const MOTION_KEYFRAME_EPSILON_SEC = 0.06;
const timelineEditRuntime = {
  selectedIds: new Set(),
  drag: null
};
const AUTOSAVE_DEBOUNCE_MS = 180;
const SPATIAL_INDEX_CELL = 220;
const SPATIAL_INDEX_GROW = 28;

let autosaveTimer = null;
let autosaveDirty = false;
let autosaveErrorStampMs = 0;
// Layer DOM is expensive; keep it dirty-flagged instead of rebuilding each frame.
let layerPanelDirty = true;
let stageRectCache = null;
let shapeBoundsCache = new Map();
let spatialIndexCache = null;
let spatialIndexDirty = true;
// Demand-driven RAF loop: active only for timeline playback or animated strokes.
let renderLoopHandle = 0;

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
state.panelCollapsed = normalizeCollapsedPanels(state.panelCollapsed);

function setStatus(msg) {
  els.status.textContent = msg;
}

function markLayerPanelDirty() {
  layerPanelDirty = true;
}

function shouldRenderLayerPanel() {
  return layerPanelDirty;
}

function invalidateStageRectCache() {
  stageRectCache = null;
}

function getStageRect() {
  if (!stageRectCache) {
    stageRectCache = els.stage.getBoundingClientRect();
  }
  return stageRectCache;
}

function invalidateShapeCaches() {
  shapeBoundsCache = new Map();
  spatialIndexCache = null;
  spatialIndexDirty = true;
}

function setButtonIconLabel(button, iconOrSpec, label) {
  if (!button) {
    return;
  }
  const spec = iconOrSpec && typeof iconOrSpec === 'object'
    ? iconOrSpec
    : { icon: iconOrSpec, label };
  const iconSvg = typeof spec.iconSvg === 'string' ? spec.iconSvg.trim() : '';
  const safeIcon = String(spec.icon || '').trim() || '•';
  const safeLabel = String(spec.label || button.dataset.baseLabel || button.textContent || '').trim();
  button.dataset.baseLabel = safeLabel;
  button.classList.add('with-icon');
  const iconMarkup = iconSvg
    ? `<span class="btn-ico svg" aria-hidden="true">${iconSvg}</span>`
    : `<span class="btn-ico" aria-hidden="true">${safeIcon}</span>`;
  button.innerHTML = `${iconMarkup}<span class="btn-label">${safeLabel}</span>`;
  button.title = safeLabel;
}

function initButtonIconLabels() {
  for (const [id, spec] of Object.entries(BUTTON_ICON_LABELS)) {
    const button = els[id];
    if (!button) {
      continue;
    }
    setButtonIconLabel(button, spec);
  }
}

// Selection/marquee and editable handles are provided by module controllers.

const CAPTION_TEMPLATE_PRESETS = {
  typewriter: {
    template: 'typewriter',
    mode: 'typewriter',
    fontFamily: '',
    textColor: '#e9f2ff',
    strokeColor: '#05070d',
    strokeWidth: 3,
    highlightColor: '#7be0ff',
    highlightTextColor: '#0b1221',
    highlightFontFamily: '',
    highlightFontWeight: 700,
    highlightPadY: 0,
    highlightPadX: 3,
    highlightRadius: 3,
    overlayBgColor: '#081321',
    overlayBgOpacity: 0.72,
    fontSize: 22,
    fontWeight: 700,
    lineHeight: 1.33,
    letterSpacing: 0,
    wordHighlight: false
  },
  karaoke_classic: {
    template: 'karaoke_classic',
    mode: 'karaoke',
    fontFamily: '',
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
  impact_yellow: {
    template: 'impact_yellow',
    mode: 'karaoke',
    fontFamily: 'Oswald',
    textColor: '#ffe25a',
    strokeColor: '#020306',
    strokeWidth: 5,
    highlightColor: '#5aff9f',
    highlightTextColor: '#03120a',
    highlightFontFamily: '',
    highlightFontWeight: 900,
    highlightPadY: 0,
    highlightPadX: 3,
    highlightRadius: 2,
    overlayBgColor: '#06090f',
    overlayBgOpacity: 0.64,
    fontSize: 24,
    fontWeight: 900,
    lineHeight: 1.3,
    letterSpacing: 0.2,
    wordHighlight: true
  },
  clean_paragraph: {
    template: 'clean_paragraph',
    mode: 'plain',
    fontFamily: '',
    textColor: '#f2f5fa',
    strokeColor: '#111826',
    strokeWidth: 1,
    highlightColor: '#9fd6ff',
    highlightTextColor: '#0f1d2b',
    highlightFontFamily: '',
    highlightFontWeight: 700,
    highlightPadY: 0,
    highlightPadX: 3,
    highlightRadius: 3,
    overlayBgColor: '#101726',
    overlayBgOpacity: 0.48,
    fontSize: 21,
    fontWeight: 600,
    lineHeight: 1.42,
    letterSpacing: 0,
    wordHighlight: false
  },
  bold_two_words: {
    template: 'bold_two_words',
    mode: 'karaoke',
    fontFamily: 'Oswald',
    textColor: '#ffffff',
    strokeColor: '#05070d',
    strokeWidth: 4.5,
    highlightColor: '#ffd95a',
    highlightTextColor: '#0c0f18',
    highlightFontFamily: '',
    highlightFontWeight: 900,
    highlightPadY: 0,
    highlightPadX: 3,
    highlightRadius: 3,
    overlayBgColor: '#04060a',
    overlayBgOpacity: 0.72,
    fontSize: 24,
    fontWeight: 900,
    lineHeight: 1.28,
    letterSpacing: 0.4,
    wordHighlight: true
  },
  capcut_black: {
    template: 'capcut_black',
    mode: 'capcut_black',
    fontFamily: 'Sora',
    textColor: '#ffffff',
    strokeColor: '#000000',
    strokeWidth: 0,
    highlightColor: '#ff4d79',
    highlightTextColor: '#ffffff',
    highlightFontFamily: 'Sora',
    highlightFontWeight: 800,
    highlightPadY: 2,
    highlightPadX: 6,
    highlightRadius: 4,
    overlayBgColor: '#000000',
    overlayBgOpacity: 0,
    fontSize: 28,
    fontWeight: 800,
    lineHeight: 1.3,
    letterSpacing: 0.2,
    wordHighlight: true
  },
  word_pop: {
    template: 'word_pop',
    mode: 'word_pop',
    fontFamily: 'Nunito',
    textColor: '#ffffff',
    strokeColor: '#030608',
    strokeWidth: 4,
    highlightColor: '#ffe05d',
    highlightTextColor: '#121315',
    highlightFontFamily: 'Nunito',
    highlightFontWeight: 900,
    highlightPadY: 0,
    highlightPadX: 2,
    highlightRadius: 3,
    overlayBgColor: '#000000',
    overlayBgOpacity: 0.72,
    fontSize: 26,
    fontWeight: 900,
    lineHeight: 1.3,
    letterSpacing: 0.2,
    wordHighlight: true
  },
  slide_up: {
    template: 'slide_up',
    mode: 'slide_up',
    fontFamily: 'Sora',
    textColor: '#ffffff',
    strokeColor: '#030608',
    strokeWidth: 3.5,
    highlightColor: '#7bffd6',
    highlightTextColor: '#081a14',
    highlightFontFamily: 'Sora',
    highlightFontWeight: 800,
    highlightPadY: 0,
    highlightPadX: 3,
    highlightRadius: 3,
    overlayBgColor: '#060b15',
    overlayBgOpacity: 0.72,
    fontSize: 23,
    fontWeight: 800,
    lineHeight: 1.32,
    letterSpacing: 0,
    wordHighlight: true
  }
};

function cloneCaptionStyle(style) {
  return JSON.parse(JSON.stringify(style || {}));
}

function captionDefaultStyle() {
  return cloneCaptionStyle(CAPTION_TEMPLATE_PRESETS.karaoke_classic);
}

function normalizeCaptionStyle(rawStyle, fallbackStyle) {
  const fallback = fallbackStyle || captionDefaultStyle();
  const raw = rawStyle && typeof rawStyle === 'object' ? rawStyle : {};
  const template = typeof raw.template === 'string' && raw.template ? raw.template : fallback.template;
  const VALID_MODES = ['typewriter', 'karaoke', 'plain', 'capcut_black', 'word_pop', 'slide_up'];
  const mode = VALID_MODES.includes(raw.mode) ? raw.mode : (VALID_MODES.includes(fallback.mode) ? fallback.mode : 'plain');
  const fontSize = clamp(Number(raw.fontSize) || Number(fallback.fontSize) || 22, 14, 96);
  const fontWeight = clamp(Math.round(Number(raw.fontWeight) || Number(fallback.fontWeight) || 700), 500, 900);
  const lineHeight = clamp(Number(raw.lineHeight) || Number(fallback.lineHeight) || 1.34, 1.05, 2.2);
  const strokeWidth = clamp(Number(raw.strokeWidth), 0, 8);
  const letterSpacing = clamp(Number(raw.letterSpacing), -1, 5);
  const overlayBgOpacity = clamp(Number(raw.overlayBgOpacity), 0, 1);
  const highlightPadY = clamp(Number(raw.highlightPadY), 0, 18);
  const highlightPadX = clamp(Number(raw.highlightPadX), 0, 18);
  const highlightRadius = clamp(Number(raw.highlightRadius), 0, 24);
  const highlightFontWeight = clamp(Math.round(Number(raw.highlightFontWeight) || Number(fallback.highlightFontWeight) || Number(fallback.fontWeight) || 700), 500, 900);
  const fontFamily = typeof raw.fontFamily === 'string' ? raw.fontFamily : (typeof fallback.fontFamily === 'string' ? fallback.fontFamily : '');
  const highlightFontFamily = typeof raw.highlightFontFamily === 'string'
    ? raw.highlightFontFamily
    : (typeof fallback.highlightFontFamily === 'string' ? fallback.highlightFontFamily : '');
  return {
    template,
    mode,
    fontFamily,
    textColor: typeof raw.textColor === 'string' && raw.textColor ? raw.textColor : (fallback.textColor || '#f7fbff'),
    strokeColor: typeof raw.strokeColor === 'string' && raw.strokeColor ? raw.strokeColor : (fallback.strokeColor || '#05070d'),
    strokeWidth: Number.isFinite(strokeWidth) ? strokeWidth : (Number(fallback.strokeWidth) || 0),
    highlightColor: typeof raw.highlightColor === 'string' && raw.highlightColor ? raw.highlightColor : (fallback.highlightColor || '#ffe05d'),
    highlightTextColor: typeof raw.highlightTextColor === 'string' && raw.highlightTextColor ? raw.highlightTextColor : (fallback.highlightTextColor || '#121315'),
    highlightFontFamily,
    highlightFontWeight: Number.isFinite(highlightFontWeight) ? highlightFontWeight : (Number(fallback.highlightFontWeight) || Number(fallback.fontWeight) || 700),
    highlightPadY: Number.isFinite(highlightPadY) ? highlightPadY : (Number(fallback.highlightPadY) || 0),
    highlightPadX: Number.isFinite(highlightPadX) ? highlightPadX : (Number(fallback.highlightPadX) || 3),
    highlightRadius: Number.isFinite(highlightRadius) ? highlightRadius : (Number(fallback.highlightRadius) || 3),
    overlayBgColor: typeof raw.overlayBgColor === 'string' && raw.overlayBgColor ? raw.overlayBgColor : (fallback.overlayBgColor || '#0b1018'),
    overlayBgOpacity: Number.isFinite(overlayBgOpacity) ? overlayBgOpacity : (Number(fallback.overlayBgOpacity) || 0.72),
    fontSize,
    fontWeight,
    lineHeight,
    letterSpacing: Number.isFinite(letterSpacing) ? letterSpacing : 0,
    wordHighlight: typeof raw.wordHighlight === 'boolean' ? raw.wordHighlight : !!fallback.wordHighlight,
    wordsPerLine: clamp(Math.round(Number(raw.wordsPerLine) || Number(fallback.wordsPerLine) || 0), 0, 20)
  };
}

function presetCaptionStyle(templateId) {
  const preset = CAPTION_TEMPLATE_PRESETS[templateId];
  if (!preset) {
    return null;
  }
  return normalizeCaptionStyle(preset, captionDefaultStyle());
}

function escapeHtml(text) {
  return String(text || '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function toOverlayBg(style) {
  const color = String(style.overlayBgColor || '#0b1018');
  const alpha = clamp(Number(style.overlayBgOpacity) || 0, 0, 1);
  if (/^#([0-9a-fA-F]{6})$/.test(color)) {
    const hex = color.slice(1);
    const r = Number.parseInt(hex.slice(0, 2), 16);
    const g = Number.parseInt(hex.slice(2, 4), 16);
    const b = Number.parseInt(hex.slice(4, 6), 16);
    return `rgba(${r}, ${g}, ${b}, ${alpha.toFixed(3)})`;
  }
  return color;
}

function captionStrokeShadow(style) {
  const width = clamp(Number(style.strokeWidth) || 0, 0, 8);
  if (width < 0.01) {
    return '0 1px 1px rgba(0,0,0,0.35)';
  }
  const c = style.strokeColor || '#000000';
  const offsets = [
    [-1, 0], [1, 0], [0, -1], [0, 1],
    [-1, -1], [1, -1], [-1, 1], [1, 1]
  ];
  const shadows = offsets.map(([dx, dy]) => `${(dx * width).toFixed(2)}px ${(dy * width).toFixed(2)}px 0 ${c}`);
  shadows.push('0 2px 8px rgba(0,0,0,0.45)');
  return shadows.join(', ');
}

// ── Webcam PiP ──
const webcamState = {
  enabled: false,
  stream: null,
  position: 'br',  // br | bl | tr | tl
  size: 80,
  shape: 'circle'  // circle | rounded
};

async function startWebcam() {
  if (webcamState.stream) {
    return;
  }
  try {
    const stream = await navigator.mediaDevices.getUserMedia({ video: true, audio: false });
    webcamState.stream = stream;
    webcamState.enabled = true;
    if (els.webcamFeed) {
      els.webcamFeed.srcObject = stream;
    }
    if (els.btnWebcamToggle) {
      els.btnWebcamToggle.textContent = 'Disable Cam';
    }
    setStatus('Webcam PiP active.');
    ensureRenderLoop();
  } catch (err) {
    setStatus(`Webcam error: ${err.message}`);
    webcamState.enabled = false;
  }
}

function stopWebcam() {
  if (webcamState.stream) {
    for (const track of webcamState.stream.getTracks()) {
      track.stop();
    }
    webcamState.stream = null;
  }
  webcamState.enabled = false;
  if (els.webcamFeed) {
    els.webcamFeed.srcObject = null;
  }
  if (els.btnWebcamToggle) {
    els.btnWebcamToggle.textContent = 'Enable Cam';
  }
  setStatus('Webcam disabled.');
}

function drawWebcamPiP() {
  const video = els.webcamFeed;
  if (!video || !webcamState.enabled || video.readyState < 2) {
    return;
  }
  const width = els.stage.width / DPR;
  const height = els.stage.height / DPR;
  const size = webcamState.size;
  const pad = 16;

  let cx, cy;
  switch (webcamState.position) {
    case 'tl': cx = pad + size / 2; cy = pad + size / 2; break;
    case 'tr': cx = width - pad - size / 2; cy = pad + size / 2; break;
    case 'bl': cx = pad + size / 2; cy = height - pad - size / 2; break;
    default:   cx = width - pad - size / 2; cy = height - pad - size / 2; break;
  }

  ctx.save();
  ctx.beginPath();
  if (webcamState.shape === 'circle') {
    ctx.arc(cx, cy, size / 2, 0, Math.PI * 2);
  } else {
    const r = size * 0.12;
    if (typeof ctx.roundRect === 'function') {
      ctx.roundRect(cx - size / 2, cy - size / 2, size, size, r);
    } else {
      ctx.rect(cx - size / 2, cy - size / 2, size, size);
    }
  }
  ctx.clip();
  ctx.drawImage(video, cx - size / 2, cy - size / 2, size, size);
  ctx.restore();

  // Border ring
  ctx.save();
  ctx.strokeStyle = 'rgba(255,255,255,0.6)';
  ctx.lineWidth = 2;
  ctx.beginPath();
  if (webcamState.shape === 'circle') {
    ctx.arc(cx, cy, size / 2, 0, Math.PI * 2);
  } else {
    const r = size * 0.12;
    if (typeof ctx.roundRect === 'function') {
      ctx.roundRect(cx - size / 2, cy - size / 2, size, size, r);
    } else {
      ctx.rect(cx - size / 2, cy - size / 2, size, size);
    }
  }
  ctx.stroke();
  ctx.restore();
}

// ── Canvas Format / Safe-zone guides ──
let canvasFormat = '16:9';

function setCanvasFormat(fmt) {
  canvasFormat = fmt;
  if (els.btnFormat16x9) els.btnFormat16x9.classList.toggle('active', fmt === '16:9');
  if (els.btnFormat9x16) els.btnFormat9x16.classList.toggle('active', fmt === '9:16');
  if (els.btnFormat1x1)  els.btnFormat1x1.classList.toggle('active', fmt === '1:1');
  updateFormatGuide();
  updateCaptionOverlayGeometry();
  saveToLocalStorage();
}

function updateFormatGuide() {
  const guide = els.formatGuide;
  if (!guide) {
    return;
  }
  guide.innerHTML = '';
  if (canvasFormat === '16:9') {
    return;
  }
  const wrap = els.canvasWrap.getBoundingClientRect();
  const ww = wrap.width;
  const wh = wrap.height;
  let fw, fh;
  if (canvasFormat === '9:16') {
    const ratio = 9 / 16;
    if (ww / wh > ratio) {
      fh = wh;
      fw = Math.round(wh * ratio);
    } else {
      fw = ww;
      fh = Math.round(ww / ratio);
    }
  } else {
    const side = Math.min(ww, wh);
    fw = fh = Math.round(side);
  }
  const ml = Math.round((ww - fw) / 2);
  const mt = Math.round((wh - fh) / 2);

  // Mask panels (4 sides)
  const masks = [
    `left:0;top:0;width:${ml}px;height:100%`,
    `right:0;top:0;width:${ml}px;height:100%`,
    `left:0;top:0;width:100%;height:${mt}px`,
    `left:0;bottom:0;width:100%;height:${mt}px`
  ];
  for (const css of masks) {
    const d = document.createElement('div');
    d.className = 'format-guide-mask';
    d.style.cssText = css;
    guide.appendChild(d);
  }
  // Frame border
  const frame = document.createElement('div');
  frame.className = 'format-guide-frame';
  frame.style.cssText = `left:${ml}px;top:${mt}px;width:${fw}px;height:${fh}px`;
  guide.appendChild(frame);
  // Label
  const label = document.createElement('div');
  label.className = 'format-guide-label';
  label.style.cssText = `left:${ml + fw / 2}px;top:${mt + 8}px;transform:translateX(-50%)`;
  label.textContent = canvasFormat === '9:16' ? '9:16 Shorts' : '1:1 Square';
  guide.appendChild(label);
}

// ── Caption Placement ──
const captionPlacement = {
  vertical: 'bottom',   // 'top' | 'center' | 'bottom'
  offsetPct: 12,        // distance from edge as % of frame height
  zoneVisible: true     // show dashed zone indicator
};

/** Compute the safe-zone frame rect (in canvas-wrap CSS pixels). */
function getSafeZoneRect() {
  const wrap = els.canvasWrap.getBoundingClientRect();
  const ww = wrap.width;
  const wh = wrap.height;
  if (canvasFormat === '16:9') {
    return { left: 0, top: 0, width: ww, height: wh };
  }
  let fw, fh;
  if (canvasFormat === '9:16') {
    const ratio = 9 / 16;
    if (ww / wh > ratio) { fh = wh; fw = Math.round(wh * ratio); }
    else                  { fw = ww; fh = Math.round(ww / ratio); }
  } else { // 1:1
    const side = Math.min(ww, wh);
    fw = fh = Math.round(side);
  }
  return {
    left:   Math.round((ww - fw) / 2),
    top:    Math.round((wh - fh) / 2),
    width:  fw,
    height: fh
  };
}

/**
 * Re-positions the live-transcript-overlay and caption-zone-indicator
 * so they sit at the correct spot inside the active format frame.
 */
function captionZoneTargetHeightPx(style, overlay) {
  const fontPx = clamp(Number(style && style.fontSize) || 22, 14, 96);
  const lineHeight = clamp(Number(style && style.lineHeight) || 1.34, 1.05, 2.2);
  const hlPadY = clamp(Number(style && style.highlightPadY) || 0, 0, 18);
  let target = Math.round(fontPx * lineHeight + 12 + hlPadY * 2);
  if (overlay) {
    const overlayRect = overlay.getBoundingClientRect();
    const overlayHeight = Math.round(Number(overlayRect.height) || 0);
    if (overlayHeight > 0) {
      target = Math.round(overlayHeight + 10);
    }
  }
  return clamp(target, 34, 86);
}

function syncCaptionPlacementControls() {
  if (els.btnCaptionPosTop) {
    els.btnCaptionPosTop.classList.toggle('active', captionPlacement.vertical === 'top');
  }
  if (els.btnCaptionPosCenter) {
    els.btnCaptionPosCenter.classList.toggle('active', captionPlacement.vertical === 'center');
  }
  if (els.btnCaptionPosBottom) {
    els.btnCaptionPosBottom.classList.toggle('active', captionPlacement.vertical === 'bottom');
  }
  if (els.captionOffsetPct) {
    els.captionOffsetPct.value = String(captionPlacement.offsetPct);
  }
  if (els.captionOffsetPctValue) {
    els.captionOffsetPctValue.textContent = String(captionPlacement.offsetPct);
  }
}

function updateCaptionOverlayGeometry() {
  const overlay = els.liveTranscriptOverlay;
  const indicator = els.captionZoneIndicator;
  if (!overlay || !els.canvasWrap) {
    return;
  }

  const frame = getSafeZoneRect();
  const PAD = 16; // horizontal padding inside frame
  const overlayWidth = frame.width - PAD * 2;
  const offsetPx = Math.round(frame.height * (captionPlacement.offsetPct / 100));
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  const showZone = captionPlacement.zoneVisible && !state.timeline.playing;
  const allowDrag = showZone;

  // ── position the overlay ──
  overlay.style.left      = `${frame.left + PAD}px`;
  overlay.style.width     = `${overlayWidth}px`;
  overlay.style.minWidth  = '0';
  overlay.style.maxWidth  = `${overlayWidth}px`;
  overlay.classList.toggle('draggable', allowDrag);
  overlay.style.pointerEvents = allowDrag ? 'auto' : 'none';

  if (captionPlacement.vertical === 'bottom') {
    overlay.style.bottom  = `${frame.top + offsetPx}px`;
    overlay.style.top     = '';
    overlay.style.transform = 'none';
  } else if (captionPlacement.vertical === 'top') {
    overlay.style.top     = `${frame.top + offsetPx}px`;
    overlay.style.bottom  = '';
    overlay.style.transform = 'none';
  } else {
    // center: vertically centered in frame
    overlay.style.top     = `${frame.top + Math.round(frame.height / 2)}px`;
    overlay.style.bottom  = '';
    overlay.style.transform = 'translateY(-50%)';
  }

  // ── position the zone indicator ──
  if (!indicator) {
    return;
  }
  const zoneHeight = captionZoneTargetHeightPx(style, overlay);
  indicator.classList.toggle('hidden', !showZone);

  if (showZone) {
    indicator.style.left  = `${frame.left + PAD}px`;
    indicator.style.width = `${overlayWidth}px`;
    indicator.style.height = `${zoneHeight}px`;

    if (captionPlacement.vertical === 'bottom') {
      indicator.style.bottom = `${frame.top + offsetPx}px`;
      indicator.style.top    = '';
    } else if (captionPlacement.vertical === 'top') {
      indicator.style.top    = `${frame.top + offsetPx}px`;
      indicator.style.bottom = '';
    } else {
      indicator.style.top    = `${frame.top + Math.round(frame.height / 2) - Math.round(zoneHeight / 2)}px`;
      indicator.style.bottom = '';
    }
  }
}

function setCaptionPlacementVertical(v) {
  captionPlacement.vertical = v;
  syncCaptionPlacementControls();
  updateCaptionOverlayGeometry();
  saveToLocalStorage();
}

function applyCaptionPlacementFromRelativePct(relPct) {
  let newVertical = 'center';
  let newOffsetPct = captionPlacement.offsetPct;
  if (relPct < 38) {
    newVertical = 'top';
    newOffsetPct = clamp(Math.round(relPct), 2, 48);
  } else if (relPct > 62) {
    newVertical = 'bottom';
    newOffsetPct = clamp(Math.round(100 - relPct), 2, 48);
  }
  captionPlacement.vertical = newVertical;
  captionPlacement.offsetPct = newOffsetPct;
  syncCaptionPlacementControls();
}

function setupCaptionZoneDrag() {
  const indicator = els.captionZoneIndicator;
  if (!indicator) return;

  let dragging = false;
  let startClientY = 0;
  let startIndicatorTopPx = 0; // indicator's top edge relative to canvas-wrap, in px

  indicator.addEventListener('pointerdown', (e) => {
    if (e.button !== 0) return;
    dragging = true;
    startClientY = e.clientY;
    const wrap = els.canvasWrap.getBoundingClientRect();
    const indRect = indicator.getBoundingClientRect();
    startIndicatorTopPx = indRect.top - wrap.top;
    indicator.setPointerCapture(e.pointerId);
    indicator.classList.add('dragging');
    e.preventDefault();
  });

  indicator.addEventListener('pointermove', (e) => {
    if (!dragging) return;
    const frame = getSafeZoneRect();
    if (!frame.height) return;
    const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());

    const deltaY = e.clientY - startClientY;
    const zoneHeight = captionZoneTargetHeightPx(style, els.liveTranscriptOverlay);
    // Center of indicator relative to top of frame
    const centerInFrame = (startIndicatorTopPx + deltaY + zoneHeight / 2) - frame.top;
    const relPct = clamp((centerInFrame / frame.height) * 100, 3, 97);
    applyCaptionPlacementFromRelativePct(relPct);
    updateCaptionOverlayGeometry();
  });

  const endDrag = () => {
    if (!dragging) return;
    dragging = false;
    indicator.classList.remove('dragging');
    saveToLocalStorage();
  };
  indicator.addEventListener('pointerup', endDrag);
  indicator.addEventListener('pointercancel', endDrag);
}

function setupCaptionOverlayDrag() {
  const overlay = els.liveTranscriptOverlay;
  if (!overlay) {
    return;
  }

  let dragging = false;
  let startClientY = 0;
  let startOverlayTopPx = 0;

  overlay.addEventListener('pointerdown', (e) => {
    if (e.button !== 0) {
      return;
    }
    if (state.timeline.playing || !captionPlacement.zoneVisible || !overlay.classList.contains('active')) {
      return;
    }
    dragging = true;
    startClientY = e.clientY;
    const wrap = els.canvasWrap.getBoundingClientRect();
    const overlayRect = overlay.getBoundingClientRect();
    startOverlayTopPx = overlayRect.top - wrap.top;
    overlay.setPointerCapture(e.pointerId);
    overlay.classList.add('dragging');
    e.preventDefault();
  });

  overlay.addEventListener('pointermove', (e) => {
    if (!dragging) {
      return;
    }
    const frame = getSafeZoneRect();
    if (!frame.height) {
      return;
    }
    const overlayHeight = Math.max(24, Math.round(overlay.getBoundingClientRect().height || 0));
    const deltaY = e.clientY - startClientY;
    const centerInFrame = (startOverlayTopPx + deltaY + overlayHeight / 2) - frame.top;
    const relPct = clamp((centerInFrame / frame.height) * 100, 3, 97);
    applyCaptionPlacementFromRelativePct(relPct);
    updateCaptionOverlayGeometry();
    e.preventDefault();
  });

  const endDrag = () => {
    if (!dragging) {
      return;
    }
    dragging = false;
    overlay.classList.remove('dragging');
    saveToLocalStorage();
  };
  overlay.addEventListener('pointerup', endDrag);
  overlay.addEventListener('pointercancel', endDrag);
}

function isFormControlTarget(target) {
  return !!(target && (
    target.tagName === 'INPUT' ||
    target.tagName === 'TEXTAREA' ||
    target.tagName === 'SELECT' ||
    target.isContentEditable
  ));
}

function formatTimelineTime(seconds) {
  const safe = Math.max(0, Number(seconds) || 0);
  const mins = Math.floor(safe / 60);
  const secs = safe - mins * 60;
  return `${String(mins).padStart(2, '0')}:${secs.toFixed(2).padStart(5, '0')}`;
}

function easeTimelineProgress(t, name) {
  const p = clamp(Number(t) || 0, 0, 1);
  if (name === 'linear') {
    return p;
  }
  if (name === 'easeOutCubic') {
    return 1 - Math.pow(1 - p, 3);
  }
  if (name === 'easeInCubic') {
    return p * p * p;
  }
  if (p < 0.5) {
    return 4 * p * p * p;
  }
  return 1 - Math.pow(-2 * p + 2, 3) / 2;
}

function normalizeMotionEase(rawEase, fallbackEase = 'easeInOutCubic') {
  const base = String(rawEase || fallbackEase || 'easeInOutCubic');
  if (base === 'linear' || base === 'easeOutCubic' || base === 'easeInCubic' || base === 'easeInOutCubic') {
    return base;
  }
  return 'easeInOutCubic';
}

function normalizeTrimWindow(trimStartRaw, trimEndRaw) {
  const rawStart = Number(trimStartRaw);
  const rawEnd = Number(trimEndRaw);
  let start = Number.isFinite(rawStart) ? rawStart : 0;
  let end = Number.isFinite(rawEnd) ? rawEnd : 1;
  start = clamp(start, 0, 1);
  end = clamp(end, 0, 1);
  if (end < start) {
    const tmp = start;
    start = end;
    end = tmp;
  }
  return { trimStart: start, trimEnd: end };
}

function readMotionNumber(raw, base, key, defaultValue) {
  const primary = Number(raw[key]);
  if (Number.isFinite(primary)) {
    return primary;
  }
  const secondary = Number(base[key]);
  if (Number.isFinite(secondary)) {
    return secondary;
  }
  return Number(defaultValue) || 0;
}

function normalizedMotionKeyframe(rawKeyframe, defaults = {}) {
  const raw = rawKeyframe && typeof rawKeyframe === 'object' ? rawKeyframe : {};
  const atValue = Number(raw.at ?? raw.time ?? raw.t ?? defaults.at ?? 0);
  const at = Math.max(0, Number.isFinite(atValue) ? atValue : 0);
  const dxValue = Number(raw.dx ?? raw.x ?? raw.offsetX ?? defaults.dx ?? 0);
  const dyValue = Number(raw.dy ?? raw.y ?? raw.offsetY ?? defaults.dy ?? 0);
  const opacityValue = Number(raw.opacity ?? raw.alpha ?? defaults.opacity ?? 1);
  const trim = normalizeTrimWindow(
    raw.trimStart ?? raw.trim_start ?? defaults.trimStart ?? 0,
    raw.trimEnd ?? raw.trim_end ?? defaults.trimEnd ?? 1
  );
  return {
    at: roundTimelineSeconds(at),
    dx: clamp(Number.isFinite(dxValue) ? dxValue : 0, -6000, 6000),
    dy: clamp(Number.isFinite(dyValue) ? dyValue : 0, -6000, 6000),
    opacity: clamp(Number.isFinite(opacityValue) ? opacityValue : 1, 0, 1),
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    ease: normalizeMotionEase(raw.ease, defaults.ease)
  };
}

function dedupeAndSortMotionKeyframes(keyframes) {
  const sorted = (Array.isArray(keyframes) ? keyframes.slice() : [])
    .filter((item) => !!item && typeof item === 'object')
    .sort((a, b) => (Number(a.at) || 0) - (Number(b.at) || 0));
  if (!sorted.length) {
    return [];
  }
  const deduped = [sorted[0]];
  for (let i = 1; i < sorted.length; i += 1) {
    const current = sorted[i];
    const prev = deduped[deduped.length - 1];
    if (Math.abs((Number(current.at) || 0) - (Number(prev.at) || 0)) <= 1e-3) {
      deduped[deduped.length - 1] = current;
    } else {
      deduped.push(current);
    }
  }
  return deduped;
}

function normalizeShapeMotion(rawMotion, fallback = {}) {
  const raw = rawMotion && typeof rawMotion === 'object' ? rawMotion : {};
  const base = fallback && typeof fallback === 'object' ? fallback : {};
  const read = (key, defaultValue) => readMotionNumber(raw, base, key, defaultValue);
  const ease = normalizeMotionEase(raw.ease, base.ease);
  const baseTrim = normalizeTrimWindow(
    read('trimStart', 0),
    read('trimEnd', 1)
  );
  const baseStart = Math.max(0, read('start', state.timeline.currentTime));
  const baseDuration = clamp(read('duration', 1.2), 0.05, 600);
  const fromX = clamp(read('fromX', 0), -6000, 6000);
  const fromY = clamp(read('fromY', 0), -6000, 6000);
  const toX = clamp(read('toX', 140), -6000, 6000);
  const toY = clamp(read('toY', 0), -6000, 6000);
  const fromOpacity = clamp(read('fromOpacity', 1), 0, 1);
  const toOpacity = clamp(read('toOpacity', 1), 0, 1);
  const rawKeyframes = Array.isArray(raw.keyframes)
    ? raw.keyframes
    : Array.isArray(base.keyframes)
      ? base.keyframes
      : null;
  let keyframes = [];
  if (rawKeyframes && rawKeyframes.length) {
    keyframes = rawKeyframes.map((item) => normalizedMotionKeyframe(item, {
      at: baseStart,
      dx: fromX,
      dy: fromY,
      opacity: fromOpacity,
      trimStart: baseTrim.trimStart,
      trimEnd: baseTrim.trimEnd,
      ease
    }));
  } else {
    keyframes = [
      normalizedMotionKeyframe({
        at: baseStart,
        dx: fromX,
        dy: fromY,
        opacity: fromOpacity,
        trimStart: baseTrim.trimStart,
        trimEnd: baseTrim.trimEnd,
        ease
      }, { ease }),
      normalizedMotionKeyframe({
        at: baseStart + baseDuration,
        dx: toX,
        dy: toY,
        opacity: toOpacity,
        trimStart: baseTrim.trimStart,
        trimEnd: baseTrim.trimEnd,
        ease
      }, { ease })
    ];
  }
  keyframes = dedupeAndSortMotionKeyframes(keyframes);
  if (!keyframes.length) {
    keyframes = [normalizedMotionKeyframe({
      at: baseStart,
      dx: fromX,
      dy: fromY,
      opacity: fromOpacity,
      trimStart: baseTrim.trimStart,
      trimEnd: baseTrim.trimEnd,
      ease
    }, { ease })];
  }
  const first = keyframes[0];
  const last = keyframes[keyframes.length - 1];
  const start = first.at;
  const end = Math.max(start + 0.05, last.at);
  return {
    enabled: raw.enabled !== false,
    start,
    duration: clamp(end - start, 0.05, 600),
    fromX: first.dx,
    fromY: first.dy,
    toX: last.dx,
    toY: last.dy,
    fromOpacity: first.opacity,
    toOpacity: last.opacity,
    trimStart: first.trimStart,
    trimEnd: first.trimEnd,
    ease,
    keyframes
  };
}

function shapeMotionAtTime(shape, timeSec) {
  if (!shape || !shape.motion || !shape.motion.enabled) {
    return { enabled: false, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, progress: 1 };
  }
  const motion = normalizeShapeMotion(shape.motion);
  const keyframes = motion.keyframes || [];
  if (!keyframes.length) {
    return { enabled: false, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, progress: 1 };
  }
  if (keyframes.length === 1) {
    const only = keyframes[0];
    return {
      enabled: true,
      dx: only.dx,
      dy: only.dy,
      opacity: only.opacity,
      trimStart: only.trimStart,
      trimEnd: only.trimEnd,
      progress: 1
    };
  }
  const t = Number(timeSec) || 0;
  const first = keyframes[0];
  const last = keyframes[keyframes.length - 1];
  if (t <= first.at) {
    return {
      enabled: true,
      dx: first.dx,
      dy: first.dy,
      opacity: first.opacity,
      trimStart: first.trimStart,
      trimEnd: first.trimEnd,
      progress: 0
    };
  }
  if (t >= last.at) {
    return {
      enabled: true,
      dx: last.dx,
      dy: last.dy,
      opacity: last.opacity,
      trimStart: last.trimStart,
      trimEnd: last.trimEnd,
      progress: 1
    };
  }
  let left = first;
  let right = last;
  for (let i = 0; i < keyframes.length - 1; i += 1) {
    const a = keyframes[i];
    const b = keyframes[i + 1];
    if (t >= a.at && t <= b.at) {
      left = a;
      right = b;
      break;
    }
  }
  const span = Math.max(1e-6, right.at - left.at);
  const rawProgress = clamp((t - left.at) / span, 0, 1);
  const p = easeTimelineProgress(rawProgress, left.ease || motion.ease);
  const trim = normalizeTrimWindow(
    left.trimStart + (right.trimStart - left.trimStart) * p,
    left.trimEnd + (right.trimEnd - left.trimEnd) * p
  );
  const totalProgress = clamp((t - first.at) / Math.max(1e-6, last.at - first.at), 0, 1);
  return {
    enabled: true,
    dx: left.dx + (right.dx - left.dx) * p,
    dy: left.dy + (right.dy - left.dy) * p,
    opacity: left.opacity + (right.opacity - left.opacity) * p,
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    progress: totalProgress
  };
}

function shapeMotionOffsetExtents(shape) {
  if (!shape || !shape.motion || !shape.motion.enabled) {
    return { minDx: 0, maxDx: 0, minDy: 0, maxDy: 0 };
  }
  const motion = normalizeShapeMotion(shape.motion);
  if (!motion.keyframes || !motion.keyframes.length) {
    return { minDx: 0, maxDx: 0, minDy: 0, maxDy: 0 };
  }
  let minDx = Infinity;
  let maxDx = -Infinity;
  let minDy = Infinity;
  let maxDy = -Infinity;
  for (const keyframe of motion.keyframes) {
    minDx = Math.min(minDx, keyframe.dx);
    maxDx = Math.max(maxDx, keyframe.dx);
    minDy = Math.min(minDy, keyframe.dy);
    maxDy = Math.max(maxDy, keyframe.dy);
  }
  if (!Number.isFinite(minDx) || !Number.isFinite(maxDx) || !Number.isFinite(minDy) || !Number.isFinite(maxDy)) {
    return { minDx: 0, maxDx: 0, minDy: 0, maxDy: 0 };
  }
  return {
    minDx,
    maxDx,
    minDy,
    maxDy
  };
}

function shapeMotionEndTime(shape) {
  if (!shape || !shape.motion || !shape.motion.enabled) {
    return 0;
  }
  const motion = normalizeShapeMotion(shape.motion);
  const keyframes = motion.keyframes || [];
  if (keyframes.length) {
    return keyframes[keyframes.length - 1].at;
  }
  return motion.start + motion.duration;
}

function shapeMotionStartTime(shape) {
  if (!shape || !shape.motion || !shape.motion.enabled) {
    return 0;
  }
  const motion = normalizeShapeMotion(shape.motion);
  const keyframes = motion.keyframes || [];
  if (keyframes.length) {
    return keyframes[0].at;
  }
  return motion.start;
}

function timelineDurationFromSegments(segments) {
  let maxEnd = 0;
  for (const seg of segments || []) {
    maxEnd = Math.max(maxEnd, Number(seg.end) || 0);
  }
  return Math.max(1, maxEnd);
}

function synthesizeSegmentWords(tokens, segStart, segEnd) {
  const clean = Array.isArray(tokens)
    ? tokens.map((token) => String(token || '').trim()).filter(Boolean)
    : [];
  if (!clean.length) {
    return [];
  }
  const duration = Math.max(0.06, segEnd - segStart);
  const slice = duration / clean.length;
  const words = [];
  for (let i = 0; i < clean.length; i += 1) {
    const start = segStart + (slice * i);
    const end = i === clean.length - 1 ? segEnd : segStart + (slice * (i + 1));
    words.push({
      text: clean[i],
      start: roundTimelineSeconds(start),
      end: roundTimelineSeconds(Math.max(start + 0.02, end))
    });
  }
  return words;
}

function normalizeSegmentWords(rawWords, segmentText, segStart, segEnd) {
  const source = Array.isArray(rawWords) ? rawWords : [];
  const parsed = [];

  for (const rawWord of source) {
    const isObj = !!rawWord && typeof rawWord === 'object';
    const text = String(
      isObj
        ? (rawWord.text ?? rawWord.word ?? '')
        : rawWord
    ).trim();
    if (!text) {
      continue;
    }
    const startRaw = isObj ? Number(rawWord.start ?? rawWord.start_time ?? rawWord.t0) : NaN;
    const endRaw = isObj ? Number(rawWord.end ?? rawWord.end_time ?? rawWord.t1) : NaN;
    parsed.push({
      text,
      start: Number.isFinite(startRaw) ? startRaw : null,
      end: Number.isFinite(endRaw) ? endRaw : null
    });
  }

  const hasTimedWords = parsed.length > 0 && parsed.every((word) => word.start !== null && word.end !== null);
  if (!hasTimedWords) {
    const fallbackTokens = parsed.length
      ? parsed.map((word) => word.text)
      : captionWords(segmentText);
    return synthesizeSegmentWords(fallbackTokens, segStart, segEnd);
  }

  const sorted = parsed
    .map((word) => ({
      text: word.text,
      start: Number(word.start),
      end: Number(word.end)
    }))
    .sort((a, b) => a.start - b.start || a.end - b.end);
  const normalized = [];
  let cursor = segStart;
  for (const word of sorted) {
    let start = clamp(word.start, segStart, segEnd);
    let end = clamp(word.end, segStart, segEnd);
    if (end <= start) {
      continue;
    }
    if (start < cursor) {
      start = cursor;
    }
    if (end <= start) {
      end = start + 0.02;
    }
    if (start >= segEnd) {
      break;
    }
    normalized.push({
      text: word.text,
      start: roundTimelineSeconds(start),
      end: roundTimelineSeconds(Math.min(segEnd, end))
    });
    cursor = end;
  }

  if (!normalized.length) {
    return synthesizeSegmentWords(captionWords(segmentText), segStart, segEnd);
  }
  if (normalized[normalized.length - 1].end < segEnd) {
    normalized[normalized.length - 1].end = roundTimelineSeconds(segEnd);
  }
  return normalized;
}

function normalizeTranscriptPayload(payload) {
  const rawSegments = Array.isArray(payload) ? payload : Array.isArray(payload && payload.segments) ? payload.segments : [];
  const normalized = [];
  for (const item of rawSegments) {
    if (!item || typeof item !== 'object') {
      continue;
    }
    const start = Math.max(0, Number(item.start));
    const end = Math.max(0, Number(item.end));
    const text = String(item.text || '').trim();
    if (!Number.isFinite(start) || !Number.isFinite(end) || end <= start || !text) {
      continue;
    }
    const segmentId = typeof item.id === 'string' && item.id ? item.id : uid();
    const style = item.style && typeof item.style === 'object'
      ? normalizeCaptionStyle(item.style, captionDefaultStyle())
      : null;
    const normalizedItem = { id: segmentId, start, end, text };
    const words = normalizeSegmentWords(item.words, text, start, end);
    if (style) {
      normalizedItem.style = style;
    }
    if (words.length) {
      normalizedItem.words = words;
    }
    // Preserve one-block display width (words per view)
    const displayWords = Math.round(Number(item.displayWords) || 0);
    if (displayWords > 0) {
      normalizedItem.displayWords = displayWords;
    }
    normalized.push(normalizedItem);
  }
  normalized.sort((a, b) => (a.start - b.start) || (a.end - b.end));
  return normalized;
}

function timelineZoomPxPerSec() {
  return clamp(Number(state.timeline.zoomPxPerSec) || 120, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC);
}

function timelineSnapSec() {
  const raw = Math.max(0, Number(state.timeline.snapSec) || 0);
  if (raw < 0.001) return 0;
  return raw;
}

function setTimelineZoomPxPerSec(value, options = {}) {
  state.timeline.zoomPxPerSec = clamp(Number(value) || 120, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC);
  if (els.timelineZoom) {
    els.timelineZoom.value = String(state.timeline.zoomPxPerSec);
  }
  if (!options.skipRender) {
    renderTimelineEditor({ keepScroll: true });
  }
}

function setTimelineSnapSec(value) {
  const allowed = [0, 0.1, 0.25, 0.5, 1];
  const raw = Math.max(0, Number(value) || 0);
  let next = allowed[0];
  let bestDelta = Infinity;
  for (const candidate of allowed) {
    const d = Math.abs(candidate - raw);
    if (d < bestDelta) {
      bestDelta = d;
      next = candidate;
    }
  }
  state.timeline.snapSec = next;
  if (els.timelineSnap) {
    const option = Array.from(els.timelineSnap.options).find((opt) => Math.abs(Number(opt.value) - next) < 1e-6);
    if (option) {
      els.timelineSnap.value = option.value;
    } else {
      els.timelineSnap.value = String(next);
    }
  }
}

function snapTimelineTime(timeSec) {
  const snap = timelineSnapSec();
  if (!snap) {
    return Number(timeSec) || 0;
  }
  return Math.round((Number(timeSec) || 0) / snap) * snap;
}

function timelineAudioDurationSec() {
  if (!timelineRuntime.audio) {
    return 0;
  }
  const dur = Number(timelineRuntime.audio.duration);
  return Number.isFinite(dur) && dur > 0 ? dur : 0;
}

function timelineContentWidthPx() {
  const width = Math.ceil(Math.max(1, state.timeline.duration) * timelineZoomPxPerSec());
  return Math.max(720, width + 20);
}

function timelineTimeToPx(timeSec) {
  return (Math.max(0, Number(timeSec) || 0)) * timelineZoomPxPerSec();
}

function timelinePxToTime(px) {
  return Math.max(0, (Number(px) || 0) / timelineZoomPxPerSec());
}

function ensureTranscriptSegmentIds() {
  for (const seg of state.timeline.transcriptSegments || []) {
    if (!seg.id) {
      seg.id = uid();
    }
  }
}

function sortTimelineSegmentsInPlace() {
  state.timeline.transcriptSegments.sort((a, b) => (a.start - b.start) || (a.end - b.end));
}

function selectedTimelineSegmentIds() {
  return Array.from(timelineEditRuntime.selectedIds);
}

function selectedTimelineSegments() {
  const selected = timelineEditRuntime.selectedIds;
  if (!selected.size) {
    return [];
  }
  return state.timeline.transcriptSegments.filter((seg) => selected.has(seg.id));
}

function pruneTimelineClipSelection() {
  const validIds = new Set((state.timeline.transcriptSegments || []).map((seg) => seg.id));
  for (const id of Array.from(timelineEditRuntime.selectedIds)) {
    if (!validIds.has(id)) {
      timelineEditRuntime.selectedIds.delete(id);
    }
  }
}

function setTimelineClipSelection(ids, options = {}) {
  const next = new Set();
  const validIds = new Set((state.timeline.transcriptSegments || []).map((seg) => seg.id));
  for (const id of ids || []) {
    if (validIds.has(id)) {
      next.add(id);
    }
  }
  timelineEditRuntime.selectedIds = next;
  updateTimelineClipActionButtons();
  if (!options.skipRender) {
    updateTimelineClipVisualState();
  }
}

function clearTimelineClipSelection(options = {}) {
  if (!timelineEditRuntime.selectedIds.size) {
    return;
  }
  timelineEditRuntime.selectedIds.clear();
  updateTimelineClipActionButtons();
  if (!options.skipRender) {
    updateTimelineClipVisualState();
  }
}

function updateTimelineClipActionButtons() {
  const count = timelineEditRuntime.selectedIds.size;
  if (els.btnTimelineDeleteClip) {
    els.btnTimelineDeleteClip.disabled = count < 1;
  }
  if (els.btnTimelineSplit) {
    els.btnTimelineSplit.disabled = count !== 1;
  }
}

function renderTimelineRuler() {
  if (!els.timelineRulerBody || !els.timelineEditorContent) {
    return;
  }
  const duration = Math.max(1, state.timeline.duration);
  const pxPerSec = timelineZoomPxPerSec();
  const widthPx = timelineContentWidthPx();

  els.timelineEditorContent.style.width = `${TIMELINE_LABEL_WIDTH + widthPx}px`;
  if (els.timelineTrackAudioBody) {
    els.timelineTrackAudioBody.style.width = `${widthPx}px`;
  }
  if (els.timelineTrackShapeBody) {
    els.timelineTrackShapeBody.style.width = `${widthPx}px`;
  }
  if (els.timelineTrackCaptionBody) {
    els.timelineTrackCaptionBody.style.width = `${widthPx}px`;
  }
  els.timelineRulerBody.style.width = `${widthPx}px`;

  const minorStep = pxPerSec >= 180 ? 0.25 : pxPerSec >= 100 ? 0.5 : 1;
  const frag = document.createDocumentFragment();
  for (let t = 0; t <= duration + 1e-6; t += minorStep) {
    const x = timelineTimeToPx(t);
    const major = Math.abs(t - Math.round(t)) < 1e-6;
    const tick = document.createElement('div');
    tick.className = `timeline-ruler-tick${major ? ' major' : ''}`;
    tick.style.left = `${x}px`;
    frag.appendChild(tick);
    if (major) {
      const label = document.createElement('div');
      label.className = 'timeline-ruler-tick-label';
      label.style.left = `${x}px`;
      label.textContent = `${Math.round(t)}s`;
      frag.appendChild(label);
    }
  }
  els.timelineRulerBody.innerHTML = '';
  els.timelineRulerBody.appendChild(frag);
}

function updateTimelinePlayhead() {
  if (!els.timelinePlayhead || !els.timelineEditorContent) {
    return;
  }
  const top = 0;
  const height = els.timelineEditorContent.clientHeight || 120;
  els.timelinePlayhead.style.top = `${top}px`;
  els.timelinePlayhead.style.height = `${height}px`;
  const x = timelineTimeToPx(state.timeline.currentTime);
  els.timelinePlayhead.style.left = `${TIMELINE_LABEL_WIDTH + x}px`;
}

function updateTimelineClipVisualState() {
  if (!els.timelineTrackCaptionBody) {
    return;
  }
  const activeSeg = timelineRuntime.activeIndex >= 0 ? (state.timeline.transcriptSegments[timelineRuntime.activeIndex] || null) : null;
  const activeId = activeSeg && activeSeg.id ? activeSeg.id : '';
  const selected = timelineEditRuntime.selectedIds;
  const clips = els.timelineTrackCaptionBody.querySelectorAll('.timeline-clip');
  for (const clip of clips) {
    const segId = clip.dataset.segId || '';
    clip.classList.toggle('active', selected.has(segId));
    clip.classList.toggle('current', !!activeId && segId === activeId);
  }
}

function updateTimelineShapeMotionVisualState() {
  if (!els.timelineTrackShapeBody) {
    return;
  }
  const selectedShapeSet = new Set(selectedShapeIds());
  const clips = els.timelineTrackShapeBody.querySelectorAll('.timeline-shape-clip.motion');
  for (const clip of clips) {
    const shapeId = clip.dataset.shapeId || '';
    clip.classList.toggle('active', !!shapeId && selectedShapeSet.has(shapeId));
  }
  const markers = els.timelineTrackShapeBody.querySelectorAll('.timeline-kf-marker');
  for (const marker of markers) {
    const at = Number(marker.dataset.at);
    marker.classList.toggle('current', Number.isFinite(at) && Math.abs(at - state.timeline.currentTime) <= MOTION_KEYFRAME_EPSILON_SEC);
  }
}

function renderTimelineTracks() {
  if (!els.timelineTrackCaptionBody || !els.timelineTrackAudioBody || !els.timelineTrackShapeBody) {
    return;
  }
  ensureTranscriptSegmentIds();
  pruneTimelineClipSelection();

  const captionFrag = document.createDocumentFragment();
  const audioFrag = document.createDocumentFragment();
  const shapeFrag = document.createDocumentFragment();
  const audioDuration = timelineAudioDurationSec();

  if (audioDuration > 0) {
    const clip = document.createElement('div');
    clip.className = 'timeline-audio-clip';
    clip.style.left = '0px';
    clip.style.width = `${Math.max(10, timelineTimeToPx(Math.min(audioDuration, state.timeline.duration)))}px`;
    clip.textContent = state.timeline.audioName || 'Audio';
    audioFrag.appendChild(clip);
  }

  if (state.shapes.length) {
    const animated = [];
    for (const shape of state.shapes) {
      if (!shape || isShapeHidden(shape) || !shape.motion || !shape.motion.enabled) {
        continue;
      }
      animated.push(shape);
    }

    const staticCount = Math.max(0, state.shapes.length - animated.length);
    const motionRowHeight = 18;
    const motionRows = Math.max(1, animated.length);
    const motionTrackMinHeight = Math.max(42, 12 + motionRows * motionRowHeight);
    els.timelineTrackShapeBody.style.minHeight = `${motionTrackMinHeight}px`;
    if (staticCount > 0) {
      const shapeClip = document.createElement('div');
      shapeClip.className = 'timeline-shape-clip';
      shapeClip.style.left = '0px';
      shapeClip.style.width = `${Math.max(10, timelineTimeToPx(state.timeline.duration))}px`;
      shapeClip.textContent = `${staticCount} static shape(s)`;
      shapeFrag.appendChild(shapeClip);
    }

    for (let i = 0; i < animated.length; i += 1) {
      const shape = animated[i];
      const motion = normalizeShapeMotion(shape.motion);
      const motionStart = shapeMotionStartTime(shape);
      const motionEnd = shapeMotionEndTime(shape);
      const clipDuration = Math.max(TIMELINE_MIN_CLIP_SEC, motionEnd - motionStart);
      const clip = document.createElement('div');
      clip.className = 'timeline-shape-clip motion';
      clip.dataset.shapeId = shape.id;
      clip.style.left = `${timelineTimeToPx(motionStart)}px`;
      clip.style.width = `${Math.max(10, timelineTimeToPx(clipDuration))}px`;
      clip.style.top = `${4 + i * motionRowHeight}px`;
      clip.textContent = `${shapeLayerLabel(shape)} move`;
      if (selectedShapeIds().includes(shape.id)) {
        clip.classList.add('active');
      }
      for (const keyframe of motion.keyframes || []) {
        const marker = document.createElement('span');
        marker.className = 'timeline-kf-marker';
        marker.dataset.shapeId = shape.id;
        marker.dataset.at = String(keyframe.at);
        marker.title = `Keyframe ${formatTimelineTime(keyframe.at)}`;
        marker.style.left = `${timelineTimeToPx(keyframe.at - motionStart)}px`;
        if (Math.abs((Number(keyframe.at) || 0) - state.timeline.currentTime) <= MOTION_KEYFRAME_EPSILON_SEC) {
          marker.classList.add('current');
        }
        clip.appendChild(marker);
      }
      shapeFrag.appendChild(clip);
    }
  } else {
    els.timelineTrackShapeBody.style.minHeight = '42px';
  }

  for (const seg of state.timeline.transcriptSegments) {
    const clip = document.createElement('div');
    const isOneBlock = Array.isArray(seg.words) && seg.words.length > 0;
    clip.className = isOneBlock ? 'timeline-clip oneblock' : 'timeline-clip';
    clip.dataset.segId = seg.id;
    const clipLeft = timelineTimeToPx(seg.start);
    const clipWidth = Math.max(12, timelineTimeToPx(seg.end - seg.start));
    clip.style.left = `${clipLeft}px`;
    clip.style.width = `${clipWidth}px`;
    const leftHandle = document.createElement('span');
    leftHandle.className = 'timeline-clip-handle left';
    leftHandle.dataset.edge = 'left';
    const label = document.createElement('span');
    label.className = 'timeline-clip-label';
    label.textContent = String(seg.text || '');
    const rightHandle = document.createElement('span');
    rightHandle.className = 'timeline-clip-handle right';
    rightHandle.dataset.edge = 'right';
    clip.append(leftHandle, label, rightHandle);

    // Word markers for one-block segments (word tick marks inside the clip)
    if (isOneBlock && clipWidth > 40) {
      const segDur = Math.max(0.001, seg.end - seg.start);
      const winSize = seg.displayWords || 0;
      for (let wi = 0; wi < seg.words.length; wi += 1) {
        const word = seg.words[wi];
        const isPageBoundary = winSize > 0 && wi > 0 && wi % winSize === 0;
        const isActiveWord = Math.abs(word.start - state.timeline.currentTime) < (60 / 130 / 2);
        if (!isPageBoundary && !isActiveWord) {
          continue; // only show page-boundary and active markers to avoid clutter
        }
        const marker = document.createElement('span');
        marker.className = isActiveWord ? 'timeline-word-marker active' : 'timeline-word-marker';
        const markerPct = ((word.start - seg.start) / segDur) * clipWidth;
        marker.style.left = `${Math.max(8, markerPct)}px`;
        clip.appendChild(marker);
      }
    }

    captionFrag.appendChild(clip);
  }

  els.timelineTrackAudioBody.innerHTML = '';
  els.timelineTrackAudioBody.appendChild(audioFrag);
  els.timelineTrackShapeBody.innerHTML = '';
  els.timelineTrackShapeBody.appendChild(shapeFrag);
  els.timelineTrackCaptionBody.innerHTML = '';
  els.timelineTrackCaptionBody.appendChild(captionFrag);
  updateTimelineShapeMotionVisualState();
  updateTimelineClipVisualState();
}

function renderTimelineEditor(options = {}) {
  const keepScroll = !!options.keepScroll;
  const prevScroll = keepScroll && els.timelineEditorScroll ? els.timelineEditorScroll.scrollLeft : 0;
  renderTimelineRuler();
  renderTimelineTracks();
  updateTimelinePlayhead();
  if (keepScroll && els.timelineEditorScroll) {
    els.timelineEditorScroll.scrollLeft = prevScroll;
  }
}

function updateTimelineButtons() {
  const playing = !!state.timeline.playing;
  const label = playing ? 'Pause' : 'Play';
  const icon = playing ? '⏸' : '▶';
  if (els.btnTimelinePlay) {
    setButtonIconLabel(els.btnTimelinePlay, icon, label);
  }
  if (els.btnTimelineDockPlay) {
    setButtonIconLabel(els.btnTimelineDockPlay, icon, label);
  }
}

function updateTimelineTimeUI() {
  const now = state.timeline.currentTime;
  const duration = Math.max(1, state.timeline.duration);
  const ratio = clamp(now / duration, 0, 1);
  const sliderVal = Math.round(ratio * 1000);
  const timeText = `${formatTimelineTime(now)} / ${formatTimelineTime(duration)}`;

  if (els.timelineScrub && Number(els.timelineScrub.value) !== sliderVal) {
    els.timelineScrub.value = String(sliderVal);
  }
  if (els.timelineDockScrub && Number(els.timelineDockScrub.value) !== sliderVal) {
    els.timelineDockScrub.value = String(sliderVal);
  }
  if (els.timelineTimeLabel) {
    els.timelineTimeLabel.textContent = timeText;
  }
  if (els.timelineDockTime) {
    els.timelineDockTime.textContent = timeText;
  }
  updateTimelinePlayhead();
  updateTimelineShapeMotionVisualState();
}

function activeTranscriptIndexAt(timeSec) {
  const t = Number(timeSec) || 0;
  const segments = state.timeline.transcriptSegments || [];
  for (let i = 0; i < segments.length; i += 1) {
    const seg = segments[i];
    if (t >= seg.start && t < seg.end) {
      return i;
    }
  }
  if (!segments.length) {
    return -1;
  }
  if (t >= segments[segments.length - 1].end) {
    return segments.length - 1;
  }
  return -1;
}

function timelineSeekToTranscriptPoint(segment, timeSec) {
  if (!segment) {
    return;
  }
  const segStart = Number(segment.start) || 0;
  const segEnd = Number(segment.end) || segStart;
  const seekTime = clamp(Number(timeSec), segStart, segEnd);
  setTimelineClipSelection([segment.id], { skipRender: true });
  setTimelineTime(seekTime, { updateAudio: true });
  renderTimelineEditor({ keepScroll: true });
  if (!state.timeline.playing) {
    render();
  }
}

function segmentWordTimings(segment) {
  if (!segment) {
    return [];
  }
  return normalizeSegmentWords(segment.words, segment.text, Number(segment.start) || 0, Number(segment.end) || 0);
}

function activeWordIndexAtTime(words, timeSec, fallbackProgress = 0) {
  if (!Array.isArray(words) || !words.length) {
    return -1;
  }
  const t = Number(timeSec);
  if (Number.isFinite(t)) {
    for (let i = 0; i < words.length; i += 1) {
      const start = Number(words[i].start);
      const end = Number(words[i].end);
      if (!Number.isFinite(start) || !Number.isFinite(end)) {
        continue;
      }
      if (t >= start && t < end) {
        return i;
      }
    }
    const firstStart = Number(words[0].start);
    if (Number.isFinite(firstStart) && t <= firstStart) {
      return 0;
    }
    return words.length - 1;
  }
  return clamp(Math.floor(clamp(fallbackProgress, 0, 0.999999) * words.length), 0, words.length - 1);
}

function renderTranscriptList() {
  if (!els.transcriptList) {
    updateTimelineClipVisualState();
    return;
  }
  els.transcriptList.innerHTML = '';
  const segments = state.timeline.transcriptSegments || [];
  if (!segments.length) {
    const empty = document.createElement('div');
    empty.className = 'transcript-item';
    empty.innerHTML = '<span class="time">--:--</span> <span class="label">No transcript loaded.</span>';
    els.transcriptList.appendChild(empty);
    return;
  }

  for (let i = 0; i < segments.length; i += 1) {
    const seg = segments[i];
    const item = document.createElement('div');
    item.className = 'transcript-item';
    item.dataset.segIndex = String(i);
    item.dataset.segId = seg.id;

    // ── header row: timestamp + text + edit button ──
    const header = document.createElement('div');
    header.className = 'transcript-item-header';

    const seekBtn = document.createElement('button');
    seekBtn.type = 'button';
    seekBtn.className = 'transcript-seek';
    seekBtn.title = 'Click to seek · Double-click to edit';
    seekBtn.setAttribute('aria-label', `Seek to ${formatTimelineTime(seg.start)} segment`);
    const time = document.createElement('span');
    time.className = 'time';
    time.textContent = formatTimelineTime(seg.start);
    const textSpan = document.createElement('span');
    textSpan.className = 'label';
    textSpan.textContent = seg.text;
    seekBtn.append(time, textSpan);
    seekBtn.addEventListener('click', () => {
      timelineSeekToTranscriptPoint(seg, seg.start);
    });
    seekBtn.addEventListener('dblclick', (e) => {
      e.preventDefault();
      beginTranscriptSegmentEdit(i);
    });

    const editBtn = document.createElement('button');
    editBtn.type = 'button';
    editBtn.className = 'transcript-edit-btn';
    editBtn.title = 'Edit caption text';
    editBtn.textContent = '✎';
    editBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      beginTranscriptSegmentEdit(i);
    });

    header.append(seekBtn, editBtn);
    item.appendChild(header);

    // ── word chips row ──
    const words = segmentWordTimings(seg);
    if (words.length) {
      const wordRow = document.createElement('div');
      wordRow.className = 'transcript-word-row';
      for (let wordIdx = 0; wordIdx < words.length; wordIdx += 1) {
        const word = words[wordIdx];
        const wordBtn = document.createElement('button');
        wordBtn.type = 'button';
        wordBtn.className = 'transcript-word';
        wordBtn.dataset.segIndex = String(i);
        wordBtn.dataset.wordIndex = String(wordIdx);
        wordBtn.dataset.start = String(word.start);
        wordBtn.dataset.end = String(word.end);
        wordBtn.textContent = word.text;
        wordBtn.title = `Seek ${formatTimelineTime(word.start)}`;
        wordBtn.addEventListener('click', (event) => {
          event.preventDefault();
          event.stopPropagation();
          timelineSeekToTranscriptPoint(seg, word.start);
        });
        wordRow.appendChild(wordBtn);
      }
      item.appendChild(wordRow);
    }

    // ── inline edit area (hidden by default) ──
    const editArea = document.createElement('div');
    editArea.className = 'transcript-edit-area';

    const textarea = document.createElement('textarea');
    textarea.className = 'transcript-edit-input';
    textarea.value = seg.text;
    textarea.rows = 2;
    textarea.setAttribute('aria-label', 'Edit caption text');
    textarea.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') {
        e.preventDefault();
        cancelTranscriptSegmentEdit(i);
      } else if (e.key === 'Enter' && !e.shiftKey) {
        e.preventDefault();
        commitTranscriptSegmentEdit(i, textarea.value);
      }
    });

    const hint = document.createElement('div');
    hint.className = 'transcript-edit-hint';
    hint.textContent = 'Enter to save · Shift+Enter for new line · Esc to cancel';

    const actions = document.createElement('div');
    actions.className = 'transcript-edit-actions';

    const saveBtn = document.createElement('button');
    saveBtn.type = 'button';
    saveBtn.className = 'btn-save';
    saveBtn.textContent = 'Save';
    saveBtn.addEventListener('click', () => commitTranscriptSegmentEdit(i, textarea.value));

    const cancelBtn = document.createElement('button');
    cancelBtn.type = 'button';
    cancelBtn.className = 'btn-cancel';
    cancelBtn.textContent = 'Cancel';
    cancelBtn.addEventListener('click', () => cancelTranscriptSegmentEdit(i));

    actions.append(saveBtn, cancelBtn);
    editArea.append(textarea, hint, actions);
    item.appendChild(editArea);

    els.transcriptList.appendChild(item);
  }
}

function beginTranscriptSegmentEdit(segIndex) {
  // Cancel any currently open edit first
  const prev = els.transcriptList ? els.transcriptList.querySelector('.transcript-item.editing') : null;
  if (prev) {
    const prevIdx = Number(prev.dataset.segIndex);
    if (prevIdx === segIndex) {
      // Already editing this one — focus the textarea
      const ta = prev.querySelector('.transcript-edit-input');
      if (ta) ta.focus();
      return;
    }
    prev.classList.remove('editing');
  }
  const item = els.transcriptList ? els.transcriptList.querySelector(`.transcript-item[data-seg-index="${segIndex}"]`) : null;
  if (!item) return;
  const seg = (state.timeline.transcriptSegments || [])[segIndex];
  if (!seg) return;
  const textarea = item.querySelector('.transcript-edit-input');
  if (textarea) {
    textarea.value = seg.text;
    // Place cursor at end
    textarea.setSelectionRange(textarea.value.length, textarea.value.length);
  }
  item.classList.add('editing');
  if (textarea) {
    textarea.focus();
    // Auto-resize to content
    textarea.style.height = 'auto';
    textarea.style.height = `${Math.min(textarea.scrollHeight, 160)}px`;
    textarea.addEventListener('input', () => {
      textarea.style.height = 'auto';
      textarea.style.height = `${Math.min(textarea.scrollHeight, 160)}px`;
    }, { once: false });
  }
}

function cancelTranscriptSegmentEdit(segIndex) {
  const item = els.transcriptList ? els.transcriptList.querySelector(`.transcript-item[data-seg-index="${segIndex}"]`) : null;
  if (!item) return;
  item.classList.remove('editing');
  // Restore textarea value in case user typed something
  const seg = (state.timeline.transcriptSegments || [])[segIndex];
  const textarea = item.querySelector('.transcript-edit-input');
  if (textarea && seg) textarea.value = seg.text;
}

function commitTranscriptSegmentEdit(segIndex, rawText) {
  const newText = String(rawText || '').trim();
  if (!newText) {
    setStatus('Caption text cannot be empty.');
    return;
  }
  const segs = state.timeline.transcriptSegments || [];
  const seg = segs[segIndex];
  if (!seg) return;

  const textChanged = seg.text !== newText;
  seg.text = newText;

  // Re-synthesize word timings from new text, preserving segment bounds
  const newTokens = newText.split(/\s+/).filter(Boolean);
  if (newTokens.length) {
    seg.words = synthesizeSegmentWords(newTokens, seg.start, seg.end);
    if (seg.displayWords > 0) {
      // Keep displayWords, clamp to new word count
      seg.displayWords = Math.min(seg.displayWords, newTokens.length);
    }
  } else {
    delete seg.words;
  }

  // Update the label in the DOM without full re-render for smoothness
  const item = els.transcriptList ? els.transcriptList.querySelector(`.transcript-item[data-seg-index="${segIndex}"]`) : null;
  if (item) {
    item.classList.remove('editing');
    const labelSpan = item.querySelector('.transcript-seek .label');
    if (labelSpan) labelSpan.textContent = newText;
    // Rebuild word chips
    const existingWordRow = item.querySelector('.transcript-word-row');
    if (existingWordRow) existingWordRow.remove();
    const words = segmentWordTimings(seg);
    if (words.length) {
      const wordRow = document.createElement('div');
      wordRow.className = 'transcript-word-row';
      for (let wi = 0; wi < words.length; wi += 1) {
        const word = words[wi];
        const btn = document.createElement('button');
        btn.type = 'button';
        btn.className = 'transcript-word';
        btn.dataset.segIndex = String(segIndex);
        btn.dataset.wordIndex = String(wi);
        btn.dataset.start = String(word.start);
        btn.dataset.end = String(word.end);
        btn.textContent = word.text;
        btn.title = `Seek ${formatTimelineTime(word.start)}`;
        btn.addEventListener('click', (event) => {
          event.preventDefault();
          event.stopPropagation();
          timelineSeekToTranscriptPoint(seg, word.start);
        });
        wordRow.appendChild(btn);
      }
      // Insert before edit area
      const editArea = item.querySelector('.transcript-edit-area');
      item.insertBefore(wordRow, editArea);
    }
  }

  if (textChanged) {
    pushHistory();
    renderTimelineEditor({ keepScroll: true });
    updateTranscriptDisplayForTime(true);
    setStatus(`Caption updated: "${newText.slice(0, 40)}${newText.length > 40 ? '…' : ''}"`);
  }
}

function effectiveCaptionStyle(segment) {
  const base = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  if (!segment || !segment.style) {
    return base;
  }
  return normalizeCaptionStyle(segment.style, base);
}

function activeSegmentProgress(segment) {
  if (!segment) {
    return 0;
  }
  const duration = Math.max(1e-6, Number(segment.end) - Number(segment.start));
  const progress = (state.timeline.currentTime - Number(segment.start)) / duration;
  return clamp(progress, 0, 0.999999);
}

function captionWords(text) {
  return String(text || '').trim().split(/\s+/).filter(Boolean);
}

function renderKaraokeHtml(segment, progress) {
  const words = segmentWordTimings(segment);
  if (!words.length) {
    return '';
  }
  const activeWord = activeWordIndexAtTime(words, state.timeline.currentTime, progress);

  // Windowed display: show only N words at a time, grouped into pages
  const style = effectiveCaptionStyle(segment);
  const windowSize = Math.max(0, segment.displayWords || style.wordsPerLine || 0);
  if (windowSize > 0 && words.length > windowSize) {
    const pageIndex = Math.floor(Math.max(0, activeWord) / windowSize);
    const windowStart = pageIndex * windowSize;
    const windowEnd = Math.min(words.length, windowStart + windowSize);
    const visibleWords = words.slice(windowStart, windowEnd);
    const localActive = activeWord - windowStart;
    return visibleWords
      .map((word, idx) => `<span class="word${idx === localActive ? ' active' : ''}">${escapeHtml(word.text)}</span>`)
      .join(' ');
  }

  return words
    .map((word, idx) => `<span class="word${idx === activeWord ? ' active' : ''}">${escapeHtml(word.text)}</span>`)
    .join(' ');
}

function renderTypewriterHtml(text, progress) {
  const value = String(text || '');
  if (!value) {
    return '';
  }
  const shown = Math.max(1, Math.ceil(value.length * clamp(progress, 0, 1)));
  return escapeHtml(value.slice(0, shown));
}

function applyOverlayCaptionStyle(style) {
  if (!els.liveTranscriptOverlay) {
    return;
  }
  els.liveTranscriptOverlay.style.color = style.textColor;
  // capcut_black uses per-word pill backgrounds; overlay itself is transparent
  els.liveTranscriptOverlay.style.background = style.mode === 'capcut_black' ? 'transparent' : toOverlayBg(style);
  els.liveTranscriptOverlay.style.fontFamily = style.fontFamily ? `"${style.fontFamily}", sans-serif` : '"Fira Sans", "Noto Sans", sans-serif';
  els.liveTranscriptOverlay.style.fontSize = `${style.fontSize}px`;
  els.liveTranscriptOverlay.style.fontWeight = String(style.fontWeight);
  els.liveTranscriptOverlay.style.lineHeight = String(style.lineHeight);
  els.liveTranscriptOverlay.style.letterSpacing = `${style.letterSpacing}px`;
  els.liveTranscriptOverlay.style.textShadow = style.mode === 'capcut_black' ? 'none' : captionStrokeShadow(style);
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-bg', style.highlightColor);
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-color', style.highlightTextColor);
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-font-family',
    style.highlightFontFamily ? `"${style.highlightFontFamily}", sans-serif` : 'inherit');
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-font-weight', String(style.highlightFontWeight));
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-pad-y', `${style.highlightPadY}px`);
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-pad-x', `${style.highlightPadX}px`);
  els.liveTranscriptOverlay.style.setProperty('--caption-highlight-radius', `${style.highlightRadius}px`);
}

const KARAOKE_LIKE_MODES = new Set(['karaoke', 'capcut_black', 'word_pop', 'slide_up']);

function renderLiveCaptionOverlay(segment) {
  if (!els.liveTranscriptOverlay) {
    return;
  }
  if (!segment) {
    els.liveTranscriptOverlay.textContent = '';
    els.liveTranscriptOverlay.classList.remove('active', 'caption-entering');
    els.liveTranscriptOverlay.removeAttribute('data-caption-mode');
    els.liveTranscriptOverlay.removeAttribute('data-seg-id');
    return;
  }
  const style = effectiveCaptionStyle(segment);
  const progress = activeSegmentProgress(segment);

  // Detect segment change (or page change in one-block segments) for slide-up animation
  const prevSegId = els.liveTranscriptOverlay.dataset.segId;
  const isNewSeg = prevSegId !== String(segment.id);
  els.liveTranscriptOverlay.dataset.segId = String(segment.id);
  els.liveTranscriptOverlay.dataset.captionMode = style.mode;

  // For one-block segments, also detect word-page changes
  let isNewPage = false;
  const windowSize = Math.max(0, segment.displayWords || style.wordsPerLine || 0);
  if (windowSize > 0 && Array.isArray(segment.words) && segment.words.length > 0) {
    const words = segmentWordTimings(segment);
    const activeWord = activeWordIndexAtTime(words, state.timeline.currentTime, progress);
    const currentPage = Math.floor(Math.max(0, activeWord) / windowSize);
    const prevPage = Number(els.liveTranscriptOverlay.dataset.captionPage || -1);
    if (currentPage !== prevPage) {
      els.liveTranscriptOverlay.dataset.captionPage = String(currentPage);
      isNewPage = true;
    }
  } else if (isNewSeg) {
    els.liveTranscriptOverlay.dataset.captionPage = '0';
  }

  applyOverlayCaptionStyle(style);

  if (style.mode === 'slide_up' && (isNewSeg || isNewPage)) {
    els.liveTranscriptOverlay.classList.remove('caption-entering');
    // Force reflow to re-trigger animation
    void els.liveTranscriptOverlay.offsetWidth;
    els.liveTranscriptOverlay.classList.add('caption-entering');
    els.liveTranscriptOverlay.addEventListener('animationend', () => {
      els.liveTranscriptOverlay.classList.remove('caption-entering');
    }, { once: true });
  }

  let html = '';
  if (style.mode === 'typewriter') {
    html = renderTypewriterHtml(segment.text, progress);
  } else if (KARAOKE_LIKE_MODES.has(style.mode) && style.wordHighlight) {
    html = renderKaraokeHtml(segment, progress);
  } else {
    html = escapeHtml(segment.text);
  }
  els.liveTranscriptOverlay.innerHTML = html;
  els.liveTranscriptOverlay.classList.add('active');
}

function updateTranscriptWordHighlights(activeSegIndex, timeSec) {
  if (!els.transcriptList) {
    return;
  }
  const rows = els.transcriptList.querySelectorAll('.transcript-item[data-seg-index]');
  for (const row of rows) {
    const rowSegIndex = Number(row.dataset.segIndex);
    const wordButtons = Array.from(row.querySelectorAll('.transcript-word'));
    if (!wordButtons.length) {
      continue;
    }
    if (!Number.isFinite(rowSegIndex) || rowSegIndex !== activeSegIndex) {
      for (const btn of wordButtons) {
        btn.classList.remove('active');
      }
      continue;
    }
    const words = wordButtons.map((btn) => ({
      start: Number(btn.dataset.start),
      end: Number(btn.dataset.end)
    }));
    const activeWord = activeWordIndexAtTime(words, timeSec);
    for (let i = 0; i < wordButtons.length; i += 1) {
      wordButtons[i].classList.toggle('active', i === activeWord);
    }
  }
}

function setTimelineCaptionStyle(nextStyle, options = {}) {
  state.timeline.captionStyle = normalizeCaptionStyle(nextStyle, captionDefaultStyle());
  if (options.save !== false) {
    saveToLocalStorage();
  }
  if (options.syncUi !== false) {
    syncCaptionStyleControls();
  }
  updateTranscriptDisplayForTime(true);
}

function setCaptionTemplate(templateId, options = {}) {
  if (templateId === 'custom') {
    const custom = cloneCaptionStyle(state.timeline.captionStyle);
    custom.template = 'custom';
    setTimelineCaptionStyle(custom, options);
    return;
  }
  const preset = presetCaptionStyle(templateId);
  if (!preset) {
    return;
  }
  setTimelineCaptionStyle(preset, options);
}

function setCaptionCustomPanelVisible(visible) {
  if (!els.captionCustomControls) {
    return;
  }
  els.captionCustomControls.classList.toggle('active', !!visible);
}

function syncCaptionStyleControls() {
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  state.timeline.captionStyle = style;

  if (els.captionTemplateList) {
    const buttons = els.captionTemplateList.querySelectorAll('button[data-caption-template]');
    for (const btn of buttons) {
      btn.classList.toggle('active', btn.dataset.captionTemplate === style.template);
    }
  }
  setCaptionCustomPanelVisible(style.template === 'custom');

  if (els.captionTextColor) {
    els.captionTextColor.value = style.textColor;
  }
  if (els.captionStrokeColor) {
    els.captionStrokeColor.value = style.strokeColor;
  }
  if (els.captionHighlightColor) {
    els.captionHighlightColor.value = style.highlightColor;
  }
  if (els.captionHighlightTextColor) {
    els.captionHighlightTextColor.value = style.highlightTextColor;
  }
  if (els.captionHighlightFontFamily) {
    els.captionHighlightFontFamily.value = style.highlightFontFamily || '';
  }
  if (els.captionHighlightFontWeight) {
    els.captionHighlightFontWeight.value = String(style.highlightFontWeight);
  }
  if (els.captionHighlightPadY) {
    els.captionHighlightPadY.value = String(style.highlightPadY);
  }
  if (els.captionHighlightPadX) {
    els.captionHighlightPadX.value = String(style.highlightPadX);
  }
  if (els.captionHighlightRadius) {
    els.captionHighlightRadius.value = String(style.highlightRadius);
  }
  if (els.captionOverlayBg) {
    els.captionOverlayBg.value = style.overlayBgColor;
  }
  if (els.captionFontSize) {
    els.captionFontSize.value = String(style.fontSize);
  }
  if (els.captionStrokeWidth) {
    els.captionStrokeWidth.value = String(style.strokeWidth);
  }
  if (els.captionLineHeight) {
    els.captionLineHeight.value = String(style.lineHeight);
  }
  if (els.captionLetterSpacing) {
    els.captionLetterSpacing.value = String(style.letterSpacing);
  }
  if (els.captionOverlayOpacity) {
    els.captionOverlayOpacity.value = String(style.overlayBgOpacity);
  }
  if (els.captionFontFamily) {
    els.captionFontFamily.value = style.fontFamily || '';
  }
  if (els.captionActionMode) {
    els.captionActionMode.value = style.mode;
  }
  if (els.captionFontWeight) {
    els.captionFontWeight.value = String(style.fontWeight);
  }
  if (els.captionWordHighlight) {
    els.captionWordHighlight.checked = !!style.wordHighlight;
  }
  syncCfpActiveCard();
}

function updateCaptionStyleFromCustomControls() {
  const prev = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  const next = normalizeCaptionStyle({
    template: 'custom',
    mode: els.captionActionMode ? els.captionActionMode.value : prev.mode,
    fontFamily: els.captionFontFamily ? els.captionFontFamily.value : prev.fontFamily,
    textColor: els.captionTextColor ? els.captionTextColor.value : prev.textColor,
    strokeColor: els.captionStrokeColor ? els.captionStrokeColor.value : prev.strokeColor,
    highlightColor: els.captionHighlightColor ? els.captionHighlightColor.value : prev.highlightColor,
    highlightTextColor: els.captionHighlightTextColor ? els.captionHighlightTextColor.value : prev.highlightTextColor,
    highlightFontFamily: els.captionHighlightFontFamily ? els.captionHighlightFontFamily.value : prev.highlightFontFamily,
    highlightFontWeight: els.captionHighlightFontWeight ? Number(els.captionHighlightFontWeight.value) : prev.highlightFontWeight,
    highlightPadY: els.captionHighlightPadY ? Number(els.captionHighlightPadY.value) : prev.highlightPadY,
    highlightPadX: els.captionHighlightPadX ? Number(els.captionHighlightPadX.value) : prev.highlightPadX,
    highlightRadius: els.captionHighlightRadius ? Number(els.captionHighlightRadius.value) : prev.highlightRadius,
    overlayBgColor: els.captionOverlayBg ? els.captionOverlayBg.value : prev.overlayBgColor,
    overlayBgOpacity: els.captionOverlayOpacity ? Number(els.captionOverlayOpacity.value) : prev.overlayBgOpacity,
    fontSize: els.captionFontSize ? Number(els.captionFontSize.value) : prev.fontSize,
    fontWeight: els.captionFontWeight ? Number(els.captionFontWeight.value) : prev.fontWeight,
    lineHeight: els.captionLineHeight ? Number(els.captionLineHeight.value) : prev.lineHeight,
    letterSpacing: els.captionLetterSpacing ? Number(els.captionLetterSpacing.value) : prev.letterSpacing,
    strokeWidth: els.captionStrokeWidth ? Number(els.captionStrokeWidth.value) : prev.strokeWidth,
    wordHighlight: els.captionWordHighlight ? !!els.captionWordHighlight.checked : prev.wordHighlight
  }, prev);
  setTimelineCaptionStyle(next, { syncUi: true, save: true });
}

function applyCaptionStyleToSelectedTimelineSegments() {
  const ids = selectedTimelineSegmentIds();
  if (!ids.length) {
    setStatus('Select caption clip(s) in timeline, then apply style.');
    return;
  }
  const selectedSet = new Set(ids);
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  let count = 0;
  for (const seg of state.timeline.transcriptSegments) {
    if (!selectedSet.has(seg.id)) {
      continue;
    }
    seg.style = cloneCaptionStyle(style);
    count += 1;
  }
  if (count > 0) {
    pushHistory();
    updateTranscriptDisplayForTime(true);
    setStatus(`Applied caption style to ${count} selected clip(s).`);
  }
}

function applyCaptionStyleToAllTimelineSegments() {
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  let count = 0;
  for (const seg of state.timeline.transcriptSegments) {
    seg.style = cloneCaptionStyle(style);
    count += 1;
  }
  pushHistory();
  updateTranscriptDisplayForTime(true);
  setStatus(count ? `Applied caption style to all ${count} clips.` : 'Caption style saved as default.');
}

function updateTranscriptDisplayForTime(forceListRefresh) {
  const nextIndex = activeTranscriptIndexAt(state.timeline.currentTime);
  const changed = nextIndex !== timelineRuntime.activeIndex;
  const segments = state.timeline.transcriptSegments || [];
  const active = nextIndex >= 0 ? segments[nextIndex] : null;
  const needsContinuousRefresh = !!active;
  if (!changed && !forceListRefresh && !needsContinuousRefresh) {
    return;
  }
  timelineRuntime.activeIndex = nextIndex;

  if (els.transcriptNow) {
    if (active) {
      const style = effectiveCaptionStyle(active);
      const modeLabel = style.mode === 'karaoke' ? 'karaoke' : style.mode === 'typewriter' ? 'typewriter' : 'plain';
      els.transcriptNow.textContent = `[${modeLabel}] ${active.text}`;
    } else {
      els.transcriptNow.textContent = 'No active transcript text at this time.';
    }
  }
  renderLiveCaptionOverlay(active);

  if (!els.transcriptList) {
    return;
  }
  if (changed || forceListRefresh) {
    const rows = els.transcriptList.querySelectorAll('.transcript-item');
    for (const row of rows) {
      const idx = Number(row.dataset.segIndex);
      row.classList.toggle('active', Number.isFinite(idx) && idx === nextIndex);
    }

    if (changed && nextIndex >= 0) {
      const activeBtn = els.transcriptList.querySelector(`.transcript-item[data-seg-index="${nextIndex}"]`);
      if (activeBtn && typeof activeBtn.scrollIntoView === 'function') {
        activeBtn.scrollIntoView({ block: 'nearest' });
      }
    }
  }
  updateTranscriptWordHighlights(nextIndex, state.timeline.currentTime);
  updateTimelineClipVisualState();
  updateTeleprompter();
}

function clampTimelineTime(timeSec) {
  const duration = Math.max(1, Number(state.timeline.duration) || 1);
  return clamp(Number(timeSec) || 0, 0, duration);
}

function setTimelineDuration(durationSec) {
  state.timeline.duration = Math.max(1, Number(durationSec) || 1);
  state.timeline.currentTime = clampTimelineTime(state.timeline.currentTime);
  updateTimelineTimeUI();
  updateTranscriptDisplayForTime(true);
  renderTimelineEditor({ keepScroll: true });
}

function setTimelineTime(timeSec, options = {}) {
  state.timeline.currentTime = clampTimelineTime(timeSec);
  updateTimelineTimeUI();
  updateTranscriptDisplayForTime(false);
  if (!state.timeline.playing && options.syncMotionUi !== false && selectedShapeIds().length) {
    syncSelectedShapeMotionControls();
  }

  if (options.updateAudio && timelineRuntime.audio && timelineRuntime.audio.readyState >= 1) {
    const drift = Math.abs((timelineRuntime.audio.currentTime || 0) - state.timeline.currentTime);
    if (drift > 0.08) {
      timelineRuntime.audio.currentTime = state.timeline.currentTime;
    }
  }
}

function refreshTimelineDurationFromSources(options = {}) {
  const preserveExisting = !!options.preserveExisting;
  const segmentDuration = timelineDurationFromSegments(state.timeline.transcriptSegments);
  const audioDuration = timelineAudioDurationSec();
  const motionDuration = state.shapes.reduce((maxEnd, shape) => Math.max(maxEnd, shapeMotionEndTime(shape)), 0);
  let nextDuration = Math.max(1, segmentDuration, audioDuration, motionDuration, state.timeline.currentTime);
  if (preserveExisting) {
    nextDuration = Math.max(nextDuration, state.timeline.duration);
  }
  if (Math.abs(nextDuration - state.timeline.duration) > 1e-6) {
    setTimelineDuration(nextDuration);
  }
}

function findTimelineSegmentById(segId) {
  for (const seg of state.timeline.transcriptSegments) {
    if (seg.id === segId) {
      return seg;
    }
  }
  return null;
}

function roundTimelineSeconds(value) {
  return Math.round((Number(value) || 0) * 1000) / 1000;
}

function timelineContentXFromEvent(event, bodyEl) {
  if (!bodyEl) {
    return 0;
  }
  const rect = bodyEl.getBoundingClientRect();
  const scrollLeft = els.timelineEditorScroll ? els.timelineEditorScroll.scrollLeft : 0;
  return clamp(event.clientX - rect.left + scrollLeft, 0, timelineContentWidthPx());
}

function releaseTimelineDrag() {
  window.removeEventListener('pointermove', onTimelineDragMove);
  window.removeEventListener('pointerup', onTimelineDragEnd);
  window.removeEventListener('pointercancel', onTimelineDragEnd);
}

function beginTimelineDragSession(drag) {
  releaseTimelineDrag();
  timelineEditRuntime.drag = drag;
  window.addEventListener('pointermove', onTimelineDragMove);
  window.addEventListener('pointerup', onTimelineDragEnd);
  window.addEventListener('pointercancel', onTimelineDragEnd);
}

function beginTimelineScrubDrag(event, bodyEl, options = {}) {
  if (event.button !== 0) {
    return;
  }
  if (state.timeline.playing) {
    setTimelinePlaying(false);
  }
  if (!options.keepSelection) {
    clearTimelineClipSelection({ skipRender: true });
  }
  const clickX = timelineContentXFromEvent(event, bodyEl);
  const clickTime = clampTimelineTime(timelinePxToTime(clickX));
  setTimelineTime(clickTime, { updateAudio: true });
  render();
  renderTimelineEditor({ keepScroll: true });
  beginTimelineDragSession({
    mode: 'scrub',
    pointerId: event.pointerId,
    startClientX: event.clientX,
    startTime: clickTime,
    mutated: false
  });
  event.preventDefault();
}

function beginTimelineMotionKeyframeDrag(event, shapeId, keyframeAt) {
  if (!event || event.button !== 0) {
    return false;
  }
  if (!shapeId || !Number.isFinite(Number(keyframeAt))) {
    return false;
  }
  const shape = state.shapes.find((item) => item && item.id === shapeId);
  if (!shape || !shape.motion || !shape.motion.enabled || isShapeHidden(shape) || isShapeLocked(shape)) {
    return false;
  }
  if (state.timeline.playing) {
    setTimelinePlaying(false);
  }
  const at = roundTimelineSeconds(Math.max(0, Number(keyframeAt) || 0));
  const motion = normalizeShapeMotion(shape.motion, { start: at, duration: 0.2 });
  const keyframes = (motion.keyframes || []).map((keyframe) => ({ ...keyframe }));
  if (!keyframes.length) {
    return false;
  }
  const keyIndex = findMotionKeyframeIndexNearTime(keyframes, at, 0.02);
  if (keyIndex < 0) {
    return false;
  }
  const minAt = keyIndex > 0
    ? roundTimelineSeconds((Number(keyframes[keyIndex - 1].at) || 0) + 0.01)
    : 0;
  const maxAt = keyIndex < keyframes.length - 1
    ? roundTimelineSeconds((Number(keyframes[keyIndex + 1].at) || 0) - 0.01)
    : 7200;
  beginTimelineDragSession({
    mode: 'motion-kf',
    pointerId: event.pointerId,
    startClientX: event.clientX,
    shapeId,
    keyIndex,
    startAt: at,
    minAt,
    maxAt,
    baseMotion: motion,
    baseKeyframes: keyframes,
    lastAt: at,
    mutated: false
  });
  return true;
}

function onTimelineDragMove(event) {
  const drag = timelineEditRuntime.drag;
  if (!drag) {
    return;
  }
  if (drag.pointerId !== undefined && event.pointerId !== undefined && drag.pointerId !== event.pointerId) {
    return;
  }
  if (drag.mode === 'motion-kf') {
    const shape = state.shapes.find((item) => item && item.id === drag.shapeId);
    if (!shape) {
      return;
    }
    const deltaSec = (event.clientX - drag.startClientX) / timelineZoomPxPerSec();
    let nextAt = drag.startAt + deltaSec;
    const snapSec = timelineSnapSec();
    if (snapSec) {
      nextAt = Math.round(nextAt / snapSec) * snapSec;
    }
    nextAt = clamp(nextAt, drag.minAt, drag.maxAt);
    nextAt = roundTimelineSeconds(nextAt);
    if (Math.abs(nextAt - drag.lastAt) <= 1e-4) {
      return;
    }
    const keyframes = (drag.baseKeyframes || []).map((keyframe, idx) => (
      idx === drag.keyIndex
        ? { ...keyframe, at: nextAt }
        : { ...keyframe }
    ));
    shape.motion = normalizeShapeMotion({
      ...(drag.baseMotion || {}),
      enabled: true,
      keyframes
    }, { start: nextAt, duration: 0.2 });
    drag.lastAt = nextAt;
    drag.mutated = true;
    invalidateShapeCaches();
    markLayerPanelDirty();
    refreshTimelineDurationFromSources({ preserveExisting: true });
    setTimelineTime(nextAt, { updateAudio: false, syncMotionUi: true });
    renderTimelineEditor({ keepScroll: true });
    render();
    return;
  }
  if (drag.mode === 'scrub') {
    const deltaSec = (event.clientX - drag.startClientX) / timelineZoomPxPerSec();
    const next = clampTimelineTime(drag.startTime + deltaSec);
    setTimelineTime(next, { updateAudio: true });
    render();
    return;
  }

  const deltaRaw = (event.clientX - drag.startClientX) / timelineZoomPxPerSec();
  const snapSec = timelineSnapSec();
  let delta = snapSec ? Math.round(deltaRaw / snapSec) * snapSec : deltaRaw;
  let changed = false;

  if (drag.mode === 'move') {
    let minStart = Infinity;
    for (const id of drag.activeIds) {
      const base = drag.baseById.get(id);
      if (base) {
        minStart = Math.min(minStart, base.start);
      }
    }
    if (Number.isFinite(minStart) && minStart + delta < 0) {
      delta = -minStart;
    }
    for (const id of drag.activeIds) {
      const seg = findTimelineSegmentById(id);
      const base = drag.baseById.get(id);
      if (!seg || !base) {
        continue;
      }
      const length = Math.max(TIMELINE_MIN_CLIP_SEC, base.end - base.start);
      seg.start = roundTimelineSeconds(Math.max(0, base.start + delta));
      seg.end = roundTimelineSeconds(seg.start + length);
      changed = true;
    }
  } else if (drag.mode === 'trim-left' || drag.mode === 'trim-right') {
    const seg = findTimelineSegmentById(drag.anchorId);
    const base = drag.baseById.get(drag.anchorId);
    if (seg && base) {
      if (drag.mode === 'trim-left') {
        let nextStart = base.start + delta;
        nextStart = snapSec ? snapTimelineTime(nextStart) : nextStart;
        nextStart = clamp(nextStart, 0, base.end - TIMELINE_MIN_CLIP_SEC);
        seg.start = roundTimelineSeconds(nextStart);
      } else {
        let nextEnd = base.end + delta;
        nextEnd = snapSec ? snapTimelineTime(nextEnd) : nextEnd;
        nextEnd = Math.max(base.start + TIMELINE_MIN_CLIP_SEC, nextEnd);
        seg.end = roundTimelineSeconds(nextEnd);
      }
      changed = true;
    }
  }

  if (!changed) {
    return;
  }

  drag.mutated = true;
  sortTimelineSegmentsInPlace();
  refreshTimelineDurationFromSources({ preserveExisting: true });
  renderTimelineEditor({ keepScroll: true });
  updateTranscriptDisplayForTime(true);
}

function onTimelineDragEnd(event) {
  const drag = timelineEditRuntime.drag;
  if (!drag) {
    return;
  }
  if (drag.pointerId !== undefined && event.pointerId !== undefined && drag.pointerId !== event.pointerId) {
    return;
  }
  releaseTimelineDrag();
  timelineEditRuntime.drag = null;
  if (drag.mode === 'motion-kf') {
    if (drag.mutated) {
      refreshTimelineDurationFromSources({ preserveExisting: false });
      renderTimelineEditor({ keepScroll: true });
      syncSelectedShapeMotionControls();
      pushHistory();
      setStatus(`Moved keyframe to ${formatTimelineTime(drag.lastAt)}.`);
    }
    return;
  }
  if (drag.mutated) {
    sortTimelineSegmentsInPlace();
    refreshTimelineDurationFromSources({ preserveExisting: false });
    renderTranscriptList();
    updateTranscriptDisplayForTime(true);
    renderTimelineEditor({ keepScroll: true });
    pushHistory();
  }
}

function deleteSelectedTimelineClips() {
  const selectedIds = selectedTimelineSegmentIds();
  if (!selectedIds.length) {
    setStatus('Select one or more timeline caption clips first.');
    return false;
  }
  const selectedSet = new Set(selectedIds);
  const before = state.timeline.transcriptSegments.length;
  state.timeline.transcriptSegments = state.timeline.transcriptSegments.filter((seg) => !selectedSet.has(seg.id));
  clearTimelineClipSelection({ skipRender: true });
  if (state.timeline.transcriptSegments.length === before) {
    return false;
  }
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTranscriptList();
  updateTranscriptDisplayForTime(true);
  renderTimelineEditor({ keepScroll: true });
  pushHistory();
  setStatus(`Deleted ${before - state.timeline.transcriptSegments.length} timeline clip(s).`);
  return true;
}

function splitSelectedTimelineClipAtPlayhead() {
  const selected = selectedTimelineSegments();
  if (selected.length !== 1) {
    setStatus('Select exactly one caption clip to split.');
    return false;
  }
  const seg = selected[0];
  const t = clamp(state.timeline.currentTime, seg.start, seg.end);
  if (t <= seg.start + TIMELINE_MIN_CLIP_SEC || t >= seg.end - TIMELINE_MIN_CLIP_SEC) {
    setStatus('Move playhead inside the clip before splitting.');
    return false;
  }
  const oldEnd = seg.end;
  const existingWords = segmentWordTimings(seg);
  const newSeg = {
    id: uid(),
    start: roundTimelineSeconds(t),
    end: oldEnd,
    text: seg.text
  };
  seg.end = roundTimelineSeconds(t);
  const leftWords = normalizeSegmentWords(existingWords, seg.text, seg.start, seg.end);
  const rightWords = normalizeSegmentWords(existingWords, seg.text, newSeg.start, newSeg.end);
  if (leftWords.length) {
    seg.words = leftWords;
  } else {
    delete seg.words;
  }
  if (rightWords.length) {
    newSeg.words = rightWords;
  }
  state.timeline.transcriptSegments.push(newSeg);
  sortTimelineSegmentsInPlace();
  setTimelineClipSelection([newSeg.id], { skipRender: true });
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTranscriptList();
  updateTranscriptDisplayForTime(true);
  renderTimelineEditor({ keepScroll: true });
  pushHistory();
  setStatus('Split caption clip at playhead.');
  return true;
}

function findMotionKeyframeIndexNearTime(keyframes, timeSec, toleranceSec = MOTION_KEYFRAME_EPSILON_SEC) {
  const list = Array.isArray(keyframes) ? keyframes : [];
  const t = Number(timeSec);
  if (!Number.isFinite(t) || !list.length) {
    return -1;
  }
  let bestIndex = -1;
  let bestDelta = Infinity;
  for (let i = 0; i < list.length; i += 1) {
    const at = Number(list[i] && list[i].at);
    if (!Number.isFinite(at)) {
      continue;
    }
    const delta = Math.abs(at - t);
    if (delta <= toleranceSec && delta < bestDelta) {
      bestDelta = delta;
      bestIndex = i;
    }
  }
  return bestIndex;
}

function readMotionKeyframeControls(timeSec) {
  const trim = normalizeTrimWindow(
    (Number(els.motionTrimStart && els.motionTrimStart.value) || 0) / 100,
    (Number(els.motionTrimEnd && els.motionTrimEnd.value) || 100) / 100
  );
  return normalizedMotionKeyframe({
    at: timeSec,
    dx: Number(els.motionFromX && els.motionFromX.value) || 0,
    dy: Number(els.motionFromY && els.motionFromY.value) || 0,
    opacity: Number(els.motionFromOpacity && els.motionFromOpacity.value),
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    ease: els.motionEase ? els.motionEase.value : 'easeInOutCubic'
  }, {
    at: timeSec,
    dx: 0,
    dy: 0,
    opacity: 1,
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    ease: els.motionEase ? els.motionEase.value : 'easeInOutCubic'
  });
}

function evaluatedMotionKeyframeAtTime(shape, timeSec) {
  const motionSample = shapeMotionAtTime(shape, timeSec);
  const trim = normalizeTrimWindow(motionSample.trimStart, motionSample.trimEnd);
  return normalizedMotionKeyframe({
    at: timeSec,
    dx: motionSample.dx,
    dy: motionSample.dy,
    opacity: motionSample.opacity,
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    ease: normalizeShapeMotion(shape && shape.motion, {}).ease
  }, {
    at: timeSec,
    dx: 0,
    dy: 0,
    opacity: 1,
    trimStart: 0,
    trimEnd: 1,
    ease: 'easeInOutCubic'
  });
}

function upsertShapeMotionKeyframe(shape, timeSec, options = {}) {
  if (!shape || isShapeLocked(shape) || isShapeHidden(shape)) {
    return false;
  }
  const at = roundTimelineSeconds(Math.max(0, Number(timeSec) || 0));
  const current = shape.motion
    ? normalizeShapeMotion(shape.motion, { start: at, duration: 1.2 })
    : normalizeShapeMotion({
      enabled: true,
      start: at,
      duration: 0.2,
      fromX: 0,
      fromY: 0,
      toX: 0,
      toY: 0,
      fromOpacity: 1,
      toOpacity: 1,
      trimStart: 0,
      trimEnd: 1,
      keyframes: [{ at, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease: 'easeInOutCubic' }]
    }, { start: at, duration: 0.2 });
  const keyframes = (current.keyframes || []).map((keyframe) => ({ ...keyframe }));
  const nextKeyframe = options.fromControls
    ? readMotionKeyframeControls(at)
    : evaluatedMotionKeyframeAtTime(shape, at);
  const nearIndex = findMotionKeyframeIndexNearTime(keyframes, at);
  if (nearIndex >= 0) {
    const prev = keyframes[nearIndex];
    if (prev &&
      Math.abs(prev.at - nextKeyframe.at) < 1e-6 &&
      Math.abs(prev.dx - nextKeyframe.dx) < 1e-6 &&
      Math.abs(prev.dy - nextKeyframe.dy) < 1e-6 &&
      Math.abs(prev.opacity - nextKeyframe.opacity) < 1e-6 &&
      Math.abs(prev.trimStart - nextKeyframe.trimStart) < 1e-6 &&
      Math.abs(prev.trimEnd - nextKeyframe.trimEnd) < 1e-6 &&
      prev.ease === nextKeyframe.ease) {
      return false;
    }
    keyframes[nearIndex] = nextKeyframe;
  } else {
    keyframes.push(nextKeyframe);
  }
  shape.motion = normalizeShapeMotion({
    ...current,
    enabled: true,
    keyframes
  }, { start: at, duration: 1.2 });
  return true;
}

function removeShapeMotionKeyframe(shape, timeSec) {
  if (!shape || !shape.motion) {
    return false;
  }
  const motion = normalizeShapeMotion(shape.motion, { start: timeSec, duration: 1.2 });
  const keyframes = (motion.keyframes || []).map((keyframe) => ({ ...keyframe }));
  const idx = findMotionKeyframeIndexNearTime(keyframes, timeSec);
  if (idx < 0) {
    return false;
  }
  keyframes.splice(idx, 1);
  if (!keyframes.length) {
    delete shape.motion;
    return true;
  }
  shape.motion = normalizeShapeMotion({
    ...motion,
    keyframes
  }, { start: timeSec, duration: 1.2 });
  return true;
}

function resolveMotionTargetShapes(options = {}) {
  const targets = [];
  const seen = new Set();
  const requestedId = typeof options.shapeId === 'string' ? options.shapeId : '';
  if (requestedId) {
    const hit = state.shapes.find((shape) => shape && shape.id === requestedId);
    if (hit && !isShapeLocked(hit) && !isShapeHidden(hit)) {
      targets.push(hit);
      seen.add(hit.id);
    }
  }
  for (const shape of selectedUnlockedShapes()) {
    if (!shape || isShapeHidden(shape) || seen.has(shape.id)) {
      continue;
    }
    targets.push(shape);
    seen.add(shape.id);
  }
  return targets;
}

function commitMotionKeyframeEdit(changed, statusMessage) {
  if (!changed) {
    return false;
  }
  invalidateShapeCaches();
  markLayerPanelDirty();
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTimelineEditor({ keepScroll: true });
  syncSelectedShapeMotionControls();
  pushHistory();
  render();
  if (statusMessage) {
    setStatus(statusMessage);
  }
  return true;
}

function insertMotionKeyframeAtTimeline(timeSec, options = {}) {
  const at = clampTimelineTime(timeSec);
  const targets = resolveMotionTargetShapes(options);
  if (!targets.length) {
    setStatus('Select at least one unlocked shape, or right-click a shape motion clip.');
    return false;
  }
  if (options.selectPrimary && targets[0] && targets[0].id) {
    setSelection([targets[0].id]);
  }
  let changed = 0;
  for (const shape of targets) {
    if (upsertShapeMotionKeyframe(shape, at, { fromControls: !!options.fromControls })) {
      changed += 1;
    }
  }
  if (!changed) {
    setStatus('Keyframe already matches at this time.');
    return false;
  }
  const modeLabel = options.fromControls ? 'from controls' : 'from current pose';
  return commitMotionKeyframeEdit(changed, `Inserted/updated ${changed} keyframe(s) @ ${formatTimelineTime(at)} (${modeLabel}).`);
}

function deleteMotionKeyframeAtTimeline(timeSec, options = {}) {
  const at = clampTimelineTime(timeSec);
  const targets = resolveMotionTargetShapes(options);
  if (!targets.length) {
    setStatus('Select at least one unlocked shape first.');
    return false;
  }
  let changed = 0;
  for (const shape of targets) {
    if (removeShapeMotionKeyframe(shape, at)) {
      changed += 1;
    }
  }
  if (!changed) {
    setStatus('No keyframe near playhead to delete.');
    return false;
  }
  return commitMotionKeyframeEdit(changed, `Deleted ${changed} keyframe(s) near ${formatTimelineTime(at)}.`);
}

function motionKeyframeTimesForTargets(targets) {
  const list = [];
  const seen = new Set();
  for (const shape of targets || []) {
    if (!shape || !shape.motion || !shape.motion.enabled) {
      continue;
    }
    const motion = normalizeShapeMotion(shape.motion, { start: state.timeline.currentTime, duration: 0.2 });
    for (const keyframe of motion.keyframes || []) {
      const at = roundTimelineSeconds(Number(keyframe.at) || 0);
      const key = at.toFixed(3);
      if (seen.has(key)) {
        continue;
      }
      seen.add(key);
      list.push(at);
    }
  }
  list.sort((a, b) => a - b);
  return list;
}

function stepToAdjacentMotionKeyframe(direction) {
  const dir = direction < 0 ? -1 : 1;
  const selected = selectedUnlockedShapes();
  const candidateTargets = selected.length
    ? selected
    : state.shapes.filter((shape) => !!(shape && shape.motion && shape.motion.enabled && !isShapeHidden(shape) && !isShapeLocked(shape)));
  const times = motionKeyframeTimesForTargets(candidateTargets);
  if (!times.length) {
    setStatus('No motion keyframes found.');
    return false;
  }
  const now = state.timeline.currentTime;
  let target = null;
  if (dir > 0) {
    target = times.find((time) => time > now + MOTION_KEYFRAME_EPSILON_SEC) ?? times[times.length - 1];
  } else {
    for (let i = times.length - 1; i >= 0; i -= 1) {
      if (times[i] < now - MOTION_KEYFRAME_EPSILON_SEC) {
        target = times[i];
        break;
      }
    }
    if (target === null) {
      target = times[0];
    }
  }
  setTimelineTime(target, { updateAudio: true });
  render();
  setStatus(`${dir > 0 ? 'Next' : 'Previous'} keyframe: ${formatTimelineTime(target)}.`);
  return true;
}

function onTimelineShapePointerDown(event) {
  if (!els.timelineTrackShapeBody || event.button !== 0) {
    return;
  }
  const clipEl = event.target && event.target.closest
    ? event.target.closest('.timeline-shape-clip.motion')
    : null;
  const markerEl = event.target && event.target.closest
    ? event.target.closest('.timeline-kf-marker')
    : null;
  if (clipEl && clipEl.dataset.shapeId) {
    setSelection([clipEl.dataset.shapeId]);
  }
  if (markerEl) {
    const at = Number(markerEl.dataset.at);
    const shapeId = markerEl.dataset.shapeId || (clipEl && clipEl.dataset.shapeId) || '';
    if (shapeId && Number.isFinite(at)) {
      event.preventDefault();
      setTimelineTime(at, { updateAudio: true });
      render();
      beginTimelineMotionKeyframeDrag(event, shapeId, at);
      return;
    }
  }
  if (clipEl) {
    event.preventDefault();
    const x = timelineContentXFromEvent(event, els.timelineTrackShapeBody);
    setTimelineTime(timelinePxToTime(x), { updateAudio: true });
    render();
    return;
  }
  beginTimelineScrubDrag(event, els.timelineTrackShapeBody);
}

function onTimelineShapeContextMenu(event) {
  if (!els.timelineTrackShapeBody) {
    return;
  }
  event.preventDefault();
  const clipEl = event.target && event.target.closest
    ? event.target.closest('.timeline-shape-clip.motion')
    : null;
  const shapeId = clipEl && clipEl.dataset.shapeId ? clipEl.dataset.shapeId : '';
  const x = timelineContentXFromEvent(event, els.timelineTrackShapeBody);
  const time = timelinePxToTime(x);
  insertMotionKeyframeAtTimeline(time, {
    shapeId,
    fromControls: false,
    selectPrimary: true
  });
}

function onTimelineCaptionPointerDown(event) {
  if (!els.timelineTrackCaptionBody || event.button !== 0) {
    return;
  }
  const clipEl = event.target && event.target.closest ? event.target.closest('.timeline-clip') : null;
  if (!clipEl) {
    beginTimelineScrubDrag(event, els.timelineTrackCaptionBody, { keepSelection: !!event.shiftKey });
    return;
  }

  const segId = clipEl.dataset.segId || '';
  if (!segId) {
    return;
  }
  const handle = event.target && event.target.closest ? event.target.closest('.timeline-clip-handle') : null;

  if (event.shiftKey && !handle) {
    const next = new Set(timelineEditRuntime.selectedIds);
    if (next.has(segId)) {
      next.delete(segId);
    } else {
      next.add(segId);
    }
    setTimelineClipSelection(Array.from(next));
    event.preventDefault();
    return;
  }

  if (!timelineEditRuntime.selectedIds.has(segId)) {
    if (event.shiftKey) {
      const next = Array.from(timelineEditRuntime.selectedIds);
      next.push(segId);
      setTimelineClipSelection(next);
    } else {
      setTimelineClipSelection([segId]);
    }
  } else if (!event.shiftKey && timelineEditRuntime.selectedIds.size > 1 && !handle) {
    setTimelineClipSelection([segId]);
  }

  const activeIds = handle ? [segId] : selectedTimelineSegmentIds();
  const baseById = new Map();
  for (const id of activeIds) {
    const seg = findTimelineSegmentById(id);
    if (!seg) {
      continue;
    }
    baseById.set(id, { start: seg.start, end: seg.end });
  }
  const mode = !handle ? 'move' : (handle.dataset.edge === 'left' ? 'trim-left' : 'trim-right');
  beginTimelineDragSession({
    mode,
    pointerId: event.pointerId,
    startClientX: event.clientX,
    baseById,
    activeIds,
    anchorId: segId,
    startTime: state.timeline.currentTime,
    mutated: false
  });
  event.preventDefault();
}

function stopTimelineAudio() {
  if (timelineRuntime.audio) {
    timelineRuntime.audio.pause();
  }
}

function setTimelinePlaying(playing) {
  const next = !!playing;
  if (state.timeline.playing === next) {
    return;
  }
  state.timeline.playing = next;
  state.timeline.lastTickMs = 0;
  updateTimelineButtons();

  if (timelineRuntime.audio) {
    if (next) {
      timelineRuntime.audio.currentTime = state.timeline.currentTime;
      const tryPlay = timelineRuntime.audio.play();
      if (tryPlay && typeof tryPlay.catch === 'function') {
        tryPlay.catch(() => {});
      }
    } else {
      timelineRuntime.audio.pause();
    }
  }

  // Hide zone indicator during playback; restore when stopped
  if (typeof updateCaptionOverlayGeometry === 'function') {
    updateCaptionOverlayGeometry();
  }

  // REC badge
  if (presentMode) {
    document.body.classList.toggle('is-playing', next);
    if (next) {
      startRecTimer();
    } else {
      stopRecTimer();
    }
  }

  if (!next) {
    saveToLocalStorage();
  } else {
    ensureRenderLoop();
  }
}

function toggleTimelinePlaying() {
  setTimelinePlaying(!state.timeline.playing);
}

function stepTimeline(nowMs) {
  if (!state.timeline.playing) {
    state.timeline.lastTickMs = nowMs;
    return;
  }

  if (timelineRuntime.audio && !timelineRuntime.audio.paused && Number.isFinite(timelineRuntime.audio.currentTime)) {
    setTimelineTime(timelineRuntime.audio.currentTime, { updateAudio: false });
  } else {
    const last = Number(state.timeline.lastTickMs) || nowMs;
    const deltaSec = Math.max(0, (nowMs - last) / 1000);
    if (deltaSec > 0) {
      setTimelineTime(state.timeline.currentTime + deltaSec, { updateAudio: false });
    }
  }
  state.timeline.lastTickMs = nowMs;

  if (state.timeline.currentTime >= state.timeline.duration - 1e-5) {
    setTimelinePlaying(false);
  }
}

function teardownTimelineAudio() {
  if (timelineRuntime.audio) {
    timelineRuntime.audio.pause();
    timelineRuntime.audio.src = '';
    timelineRuntime.audio.load();
  }
  if (timelineRuntime.audioUrl) {
    URL.revokeObjectURL(timelineRuntime.audioUrl);
    timelineRuntime.audioUrl = '';
  }
}

function stopTimelineMicCaptureTracks() {
  if (!timelineRuntime.micStream) {
    return;
  }
  for (const track of timelineRuntime.micStream.getTracks()) {
    track.stop();
  }
  timelineRuntime.micStream = null;
}

function updateTimelineMicRecordButton() {
  if (!els.btnTimelineMicRecord) {
    return;
  }
  const recording = !!(timelineRuntime.micRecorder && timelineRuntime.micRecorder.state === 'recording');
  if (recording) {
    setButtonIconLabel(els.btnTimelineMicRecord, { icon: '●', label: 'Stop Mic' });
    els.btnTimelineMicRecord.classList.add('warn');
  } else {
    setButtonIconLabel(els.btnTimelineMicRecord, { icon: '◉', label: 'Record Mic' });
    els.btnTimelineMicRecord.classList.remove('warn');
  }
}

function pickTimelineMicMimeType() {
  if (typeof MediaRecorder === 'undefined' || typeof MediaRecorder.isTypeSupported !== 'function') {
    return '';
  }
  const candidates = [
    'audio/webm;codecs=opus',
    'audio/webm',
    'audio/ogg;codecs=opus',
    'audio/ogg'
  ];
  for (const type of candidates) {
    if (MediaRecorder.isTypeSupported(type)) {
      return type;
    }
  }
  return '';
}

async function startTimelineMicRecording() {
  if (typeof MediaRecorder === 'undefined' || !navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
    setStatus('Mic recording is not supported in this browser. Use "Load Audio" instead.');
    return;
  }
  if (timelineRuntime.micRecorder && timelineRuntime.micRecorder.state === 'recording') {
    return;
  }
  try {
    const stream = await navigator.mediaDevices.getUserMedia({ audio: true, video: false });
    const mimeType = pickTimelineMicMimeType();
    const options = mimeType ? { mimeType } : undefined;
    const recorder = options ? new MediaRecorder(stream, options) : new MediaRecorder(stream);
    timelineRuntime.micStream = stream;
    timelineRuntime.micRecorder = recorder;
    timelineRuntime.micChunks = [];
    timelineRuntime.micFileExt = recorder.mimeType && recorder.mimeType.includes('ogg') ? 'ogg' : 'webm';

    recorder.addEventListener('dataavailable', (event) => {
      if (event.data && event.data.size > 0) {
        timelineRuntime.micChunks.push(event.data);
      }
    });
    recorder.addEventListener('error', (event) => {
      const msg = event && event.error && event.error.message ? event.error.message : 'unknown recorder error';
      setStatus(`Mic recording error: ${msg}`);
    });
    recorder.addEventListener('stop', () => {
      const chunks = timelineRuntime.micChunks.slice();
      const mime = recorder.mimeType || mimeType || 'audio/webm';
      const ext = timelineRuntime.micFileExt || (mime.includes('ogg') ? 'ogg' : 'webm');
      timelineRuntime.micChunks = [];
      timelineRuntime.micRecorder = null;
      stopTimelineMicCaptureTracks();
      updateTimelineMicRecordButton();
      if (!chunks.length) {
        setStatus('Mic recording stopped (no audio captured).');
        return;
      }
      const stamp = new Date().toISOString().replace(/[:.]/g, '-');
      const file = new File([new Blob(chunks, { type: mime })], `mic-${stamp}.${ext}`, { type: mime });
      loadTimelineAudio(file);
      setStatus(`Mic recorded and loaded: ${file.name}`);
    });

    recorder.start(200);
    updateTimelineMicRecordButton();
    setStatus('Mic recording started. Click "Stop Mic" when done.');
  } catch (err) {
    stopTimelineMicCaptureTracks();
    timelineRuntime.micRecorder = null;
    timelineRuntime.micChunks = [];
    updateTimelineMicRecordButton();
    setStatus(`Mic permission/error: ${err && err.message ? err.message : 'failed to start mic recording'}`);
  }
}

function stopTimelineMicRecording() {
  const recorder = timelineRuntime.micRecorder;
  if (!recorder) {
    return;
  }
  if (recorder.state !== 'inactive') {
    recorder.stop();
  } else {
    timelineRuntime.micRecorder = null;
    timelineRuntime.micChunks = [];
    stopTimelineMicCaptureTracks();
    updateTimelineMicRecordButton();
  }
}

function toggleTimelineMicRecording() {
  if (timelineRuntime.micRecorder && timelineRuntime.micRecorder.state === 'recording') {
    stopTimelineMicRecording();
  } else {
    startTimelineMicRecording();
  }
}

function loadTimelineAudio(file) {
  if (!file) {
    return;
  }
  teardownTimelineAudio();
  const url = URL.createObjectURL(file);
  const audio = new Audio(url);
  audio.preload = 'auto';
  timelineRuntime.audio = audio;
  timelineRuntime.audioUrl = url;
  state.timeline.audioName = file.name || 'audio';

  audio.addEventListener('loadedmetadata', () => {
    const audioDuration = Number(audio.duration) || 0;
    if (audioDuration > state.timeline.duration) {
      setTimelineDuration(audioDuration);
    } else {
      updateTimelineTimeUI();
      renderTimelineEditor({ keepScroll: true });
    }
    saveToLocalStorage();
    setStatus(`Loaded audio: ${state.timeline.audioName}`);
  });

  audio.addEventListener('ended', () => {
    setTimelinePlaying(false);
  });
}

function loadTranscriptFromFile(file) {
  if (!file) {
    return;
  }
  const reader = new FileReader();
  reader.onload = () => {
    try {
      const payload = JSON.parse(String(reader.result || '{}'));
      const segments = normalizeTranscriptPayload(payload);
      if (!segments.length) {
        setStatus('Transcript JSON has no valid segments.');
        return;
      }
      state.timeline.transcriptSegments = segments;
      state.timeline.transcriptName = file.name || 'transcript.json';
      clearTimelineClipSelection({ skipRender: true });
      setTimelineDuration(timelineDurationFromSegments(segments));
      setTimelineTime(0, { updateAudio: true });
      renderTranscriptList();
      updateTranscriptDisplayForTime(true);
      renderTimelineEditor({ keepScroll: false });
      pushHistory();
      setStatus(`Loaded transcript: ${segments.length} segment(s).`);
    } catch (_err) {
      setStatus('Invalid transcript JSON.');
    }
  };
  reader.readAsText(file);
}

function addCaptionShapeAtTimeline() {
  const idx = activeTranscriptIndexAt(state.timeline.currentTime);
  if (idx < 0) {
    setStatus('No active transcript text at the current time.');
    return;
  }
  const seg = state.timeline.transcriptSegments[idx];
  const center = snapPoint(viewportCenterWorld(), { showGuides: false });
  const shape = {
    id: uid(),
    type: 'text',
    x: center.x,
    y: center.y,
    text: seg.text,
    size: state.textSize,
    color: state.strokeColor,
    opacity: state.opacity
  };
  state.shapes.push(shape);
  setSelection([shape.id]);
  trackRecentColor(shape.color);
  pushHistory();
  render();
  setStatus('Added caption text from current timeline segment.');
}

function scrubTimelineFromSliderValue(value, options = {}) {
  const ratio = clamp((Number(value) || 0) / 1000, 0, 1);
  const nextTime = ratio * state.timeline.duration;
  setTimelineTime(nextTime, { updateAudio: true });
  if (!options.keepPlaying && state.timeline.playing) {
    setTimelinePlaying(false);
  }
}

function initTimelineUI() {
  ensureTranscriptSegmentIds();
  state.timeline.zoomPxPerSec = clamp(Number(state.timeline.zoomPxPerSec) || 120, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC);
  setTimelineSnapSec(Number.isFinite(Number(state.timeline.snapSec)) ? state.timeline.snapSec : 0.25);
  state.timeline.captionStyle = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  if (els.timelineZoom) {
    els.timelineZoom.value = String(state.timeline.zoomPxPerSec);
  }
  syncCaptionStyleControls();
  updateTimelineButtons();
  updateTimelineTimeUI();
  renderTranscriptList();
  updateTranscriptDisplayForTime(true);
  updateTimelineClipActionButtons();
  renderTimelineEditor();
}

function refreshTimelineViews() {
  renderTranscriptList();
  updateTimelineButtons();
  updateTimelineTimeUI();
  updateTranscriptDisplayForTime(true);
  updateTimelineClipActionButtons();
  renderTimelineEditor({ keepScroll: true });
}

// ── Script Editor ──

function updateScriptModeUI() {
  if (!els.scriptSplitMode || !els.scriptWordCountRow) {
    return;
  }
  const mode = els.scriptSplitMode.value;
  const showWordCount = mode === 'words' || mode === 'oneblock';
  els.scriptWordCountRow.style.display = showWordCount ? '' : 'none';
  const label = document.getElementById('scriptWordsPerCapLabel');
  if (label) {
    label.textContent = mode === 'oneblock' ? 'Words per view' : 'Words per caption';
  }
  const input = els.scriptWordsPerCap;
  if (input) {
    if (mode === 'oneblock') {
      input.min = '1';
      input.max = '12';
      if (Number(input.value) < 1 || Number(input.value) > 12) {
        input.value = '4';
      }
    } else {
      input.min = '1';
      input.max = '8';
      if (Number(input.value) > 8) {
        input.value = '3';
      }
    }
  }
}

function scriptEditorStat() {
  if (!els.scriptEditorText || !els.scriptEditorStat) {
    return;
  }
  const text = els.scriptEditorText.value.trim();
  if (!text) {
    els.scriptEditorStat.textContent = '';
    return;
  }
  const wordCount = text.split(/\s+/).filter(Boolean).length;
  const splitMode = els.scriptSplitMode ? els.scriptSplitMode.value : 'oneblock';
  const wordsPerCap = els.scriptWordsPerCap ? Number(els.scriptWordsPerCap.value) : 4;
  const wpm = els.scriptWPM ? Number(els.scriptWPM.value) : 130;
  if (splitMode === 'oneblock') {
    const secPerWord = 60 / Math.max(60, wpm);
    const totalDur = wordCount * secPerWord + 0.5;
    const pages = wordsPerCap > 0 ? Math.ceil(wordCount / wordsPerCap) : 1;
    els.scriptEditorStat.textContent = `${wordCount} words · 1 clip · ${pages} views · ~${totalDur.toFixed(1)}s`;
  } else {
    const preview = splitScriptToSegments(text, splitMode, wordsPerCap, wpm);
    const totalDur = preview.length ? preview[preview.length - 1].end : 0;
    els.scriptEditorStat.textContent = `${wordCount} words · ~${preview.length} captions · ~${totalDur.toFixed(1)}s`;
  }
}

function splitScriptToOneBlock(text, wpm, wordsPerDisplay) {
  const trimmed = String(text || '').trim();
  if (!trimmed) {
    return [];
  }
  const WPM = Math.max(60, Number(wpm) || 130);
  const N = Math.max(1, Math.min(20, Math.round(Number(wordsPerDisplay)) || 4));
  const secPerWord = 60 / WPM;
  const tokens = trimmed.split(/\s+/).filter(Boolean);
  if (!tokens.length) {
    return [];
  }
  let t = 0;
  const words = tokens.map((w) => {
    const start = roundTimelineSeconds(t);
    t += secPerWord;
    return { text: w, start, end: roundTimelineSeconds(t) };
  });
  return [{
    start: 0,
    end: roundTimelineSeconds(t + 0.5),
    text: trimmed,
    displayWords: N,
    words
  }];
}

function splitScriptToSegments(text, splitMode, wordsPerCap, wpm) {
  if (splitMode === 'oneblock') {
    return splitScriptToOneBlock(text, wpm, wordsPerCap);
  }
  const trimmed = String(text || '').trim();
  if (!trimmed) {
    return [];
  }
  const WPM = Math.max(60, Number(wpm) || 130);
  const N = Math.max(1, Math.min(8, Math.round(Number(wordsPerCap)) || 3));

  let chunks = [];
  if (splitMode === 'sentence') {
    chunks = trimmed
      .replace(/([.!?])\s+/g, '$1\n')
      .split('\n')
      .map((s) => s.trim())
      .filter(Boolean);
    if (!chunks.length) {
      chunks = [trimmed];
    }
  } else if (splitMode === 'line') {
    chunks = trimmed.split(/\n+/).map((s) => s.trim()).filter(Boolean);
    if (!chunks.length) {
      chunks = [trimmed];
    }
  } else {
    // N-words mode
    const words = trimmed.split(/\s+/).filter(Boolean);
    for (let i = 0; i < words.length; i += N) {
      chunks.push(words.slice(i, i + N).join(' '));
    }
  }

  const GAP = 0.06;
  let time = 0;
  const segments = [];
  for (const chunk of chunks) {
    if (!chunk) {
      continue;
    }
    // Duration = word count / WPM * 60s + 0.2s buffer
    const wordCount = chunk.trim().split(/\s+/).filter(Boolean).length;
    const segDur = Math.max(0.4, (wordCount / WPM) * 60 + 0.2);
    segments.push({ start: time, end: time + segDur, text: chunk });
    time += segDur + GAP;
  }
  return segments;
}

function generateCaptionsFromScript(appendMode) {
  const text = els.scriptEditorText ? els.scriptEditorText.value : '';
  if (!text.trim()) {
    setStatus('Script is empty. Type some text first.');
    return;
  }
  const splitMode = els.scriptSplitMode ? els.scriptSplitMode.value : 'words';
  const wordsPerCap = els.scriptWordsPerCap ? Number(els.scriptWordsPerCap.value) : 3;
  const wpm = els.scriptWPM ? Number(els.scriptWPM.value) : 130;

  let rawSegments = splitScriptToSegments(text, splitMode, wordsPerCap, wpm);

  if (appendMode && state.timeline.transcriptSegments.length) {
    const lastEnd = Math.max(...state.timeline.transcriptSegments.map((s) => s.end));
    const offset = lastEnd + 0.1;
    rawSegments = rawSegments.map((seg) => ({
      ...seg,
      start: seg.start + offset,
      end: seg.end + offset
    }));
  }

  const normalized = normalizeTranscriptPayload({ segments: rawSegments });
  if (!normalized.length) {
    setStatus('Could not parse script into segments.');
    return;
  }

  if (appendMode) {
    state.timeline.transcriptSegments.push(...normalized);
    state.timeline.transcriptSegments.sort((a, b) => (a.start - b.start) || (a.end - b.end));
  } else {
    state.timeline.transcriptSegments = normalized;
  }

  // Extend timeline duration if needed
  const lastEnd = Math.max(...state.timeline.transcriptSegments.map((s) => s.end));
  if (lastEnd + 2 > state.timeline.duration) {
    state.timeline.duration = Math.ceil(lastEnd) + 3;
    if (els.timelineScrub) {
      els.timelineScrub.max = String(state.timeline.duration * 100);
    }
    if (els.timelineDockScrub) {
      els.timelineDockScrub.max = String(state.timeline.duration * 100);
    }
  }

  saveToLocalStorage();
  refreshTimelineViews();
  const action = appendMode ? 'Appended' : 'Generated';
  setStatus(`${action} ${normalized.length} caption segment(s) from script.`);
}

function pullScriptFromTranscript() {
  const segs = state.timeline.transcriptSegments;
  if (!segs.length) {
    setStatus('No transcript segments loaded yet.');
    return;
  }
  const text = segs.map((s) => s.text).join('\n');
  if (els.scriptEditorText) {
    els.scriptEditorText.value = text;
    scriptEditorStat();
  }
  setStatus(`Pulled ${segs.length} segment(s) into script editor.`);
}

function normalizeCollapsedPanels(raw) {
  const normalized = {};
  for (const key of COLLAPSIBLE_PANEL_KEYS) {
    if (raw && Object.prototype.hasOwnProperty.call(raw, key)) {
      normalized[key] = !!raw[key];
    } else {
      normalized[key] = !!PANEL_COLLAPSE_DEFAULTS[key];
    }
  }
  return normalized;
}

function panelSection(key) {
  return document.querySelector(`.group.collapsible[data-panel="${key}"]`);
}

function panelToggleButton(key) {
  return document.querySelector(`button.group-toggle[data-toggle-panel="${key}"]`);
}

function setPanelCollapsed(key, collapsed, options = {}) {
  const section = panelSection(key);
  const toggle = panelToggleButton(key);
  if (!section || !toggle) {
    return;
  }
  section.classList.toggle('collapsed', collapsed);
  toggle.textContent = collapsed ? 'Show' : 'Hide';
  toggle.setAttribute('aria-expanded', collapsed ? 'false' : 'true');
  if (!options.skipState) {
    state.panelCollapsed[key] = collapsed;
  }
  if (!options.skipSave) {
    saveToLocalStorage();
  }
}

function initPanelToggles() {
  state.panelCollapsed = normalizeCollapsedPanels(state.panelCollapsed);
  for (const key of COLLAPSIBLE_PANEL_KEYS) {
    const toggle = panelToggleButton(key);
    if (!toggle) {
      continue;
    }
    setPanelCollapsed(key, state.panelCollapsed[key], { skipSave: true });
    toggle.addEventListener('click', () => {
      const next = !state.panelCollapsed[key];
      setPanelCollapsed(key, next);
      const label = COLLAPSIBLE_PANEL_LABELS[key] || 'Panel';
      setStatus(next ? `${label} panel hidden.` : `${label} panel shown.`);
    });
  }
}

function isShapeHidden(shape) {
  return !!(shape && shape.hidden);
}

function isShapeLocked(shape) {
  return !!(shape && shape.locked);
}

function isShapeSelectable(shape) {
  return !!shape && !isShapeHidden(shape) && !isShapeLocked(shape);
}

function groupMemberIds(groupId) {
  if (!groupId) {
    return [];
  }
  const ids = [];
  for (const shape of state.shapes) {
    if (shape && shape.groupId === groupId) {
      ids.push(shape.id);
    }
  }
  return ids;
}

function selectionIdsForShape(shape) {
  if (!shape || !shape.id) {
    return [];
  }
  if (!shape.groupId) {
    return [shape.id];
  }
  const groupIds = groupMemberIds(shape.groupId);
  return groupIds.length ? groupIds : [shape.id];
}

function selectedUnlockedShapes() {
  return selectedShapes().filter((shape) => !isShapeLocked(shape));
}

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

function shapeLayerLabel(shape) {
  if (!shape) return 'Shape';
  if (shape.type === 'text') {
    const text = String(shape.text || 'Text').split('\n')[0].trim();
    return text ? `Text: ${text.slice(0, 28)}` : 'Text';
  }
  if (shape.type === 'icon') {
    return `Icon: ${shape.icon || 'icon'}`;
  }
  if (shape.type === 'line') return 'Line';
  if (shape.type === 'arrow') return 'Arrow';
  if (shape.type === 'rect') return 'Rectangle';
  if (shape.type === 'ellipse') return 'Ellipse';
  if (shape.type === 'pen') return 'Pen Stroke';
  return shape.type || 'Shape';
}

function renderLayerPanel() {
  if (!els.layerList || !layerPanelDirty) return;
  const selected = new Set(typeof selectedShapeIds === 'function' ? selectedShapeIds() : []);
  const board = currentArtboard();
  const shapes = state.shapes;
  const groupOrder = new Map();
  let groupCount = 1;

  for (const shape of shapes) {
    if (shape && shape.groupId && !groupOrder.has(shape.groupId)) {
      groupOrder.set(shape.groupId, groupCount);
      groupCount += 1;
    }
  }

  els.layerList.innerHTML = '';

  const artboardItem = document.createElement('div');
  artboardItem.className = 'layer-item layer-artboard';
  const artboardLabel = document.createElement('span');
  artboardLabel.textContent = `Artboard: ${board.name}`;
  const artboardMeta = document.createElement('span');
  artboardMeta.className = 'meta';
  artboardMeta.textContent = 'base';
  artboardItem.append(artboardLabel, artboardMeta);
  els.layerList.appendChild(artboardItem);

  if (!shapes.length) {
    const empty = document.createElement('div');
    empty.className = 'layer-item layer-artboard';
    const emptyLabel = document.createElement('span');
    emptyLabel.textContent = 'No shapes yet';
    const emptyMeta = document.createElement('span');
    emptyMeta.className = 'meta';
    emptyMeta.textContent = 'empty';
    empty.append(emptyLabel, emptyMeta);
    els.layerList.appendChild(empty);
    layerPanelDirty = false;
    return;
  }

  for (let idx = shapes.length - 1; idx >= 0; idx -= 1) {
    const shape = shapes[idx];
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'layer-item layer-shape';
    btn.dataset.shapeId = shape.id;
    if (selected.has(shape.id)) {
      btn.classList.add('active');
    }
    const level = idx === shapes.length - 1 ? 'top' : idx === 0 ? 'bottom' : `z${idx}`;
    const flags = [level];
    if (shape.groupId && groupOrder.has(shape.groupId)) {
      flags.push(`G${groupOrder.get(shape.groupId)}`);
    }
    if (isShapeHidden(shape)) {
      flags.push('hidden');
    }
    if (isShapeLocked(shape)) {
      flags.push('locked');
    }
    const label = document.createElement('span');
    label.textContent = shapeLayerLabel(shape);
    const meta = document.createElement('span');
    meta.className = 'meta';
    meta.textContent = flags.join(' · ');
    btn.append(label, meta);
    els.layerList.appendChild(btn);
  }
  layerPanelDirty = false;
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
  invalidateShapeCaches();
  markLayerPanelDirty();
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

function clearSnapGuides() {
  if (!state.snapGuides) {
    state.snapGuides = { x: null, y: null };
    return false;
  }
  const hadGuide = typeof state.snapGuides.x === 'number' || typeof state.snapGuides.y === 'number';
  state.snapGuides.x = null;
  state.snapGuides.y = null;
  return hadGuide;
}

function setSnapGuides(xGuide, yGuide) {
  if (!state.snapGuides) {
    state.snapGuides = { x: null, y: null };
  }
  state.snapGuides.x = Number.isFinite(xGuide) ? xGuide : null;
  state.snapGuides.y = Number.isFinite(yGuide) ? yGuide : null;
}

function collectReferenceAnchors(excludeIds) {
  const excludes = excludeIds instanceof Set ? excludeIds : new Set();
  const xs = [];
  const ys = [];
  for (const shape of state.shapes) {
    if (!shape || isShapeHidden(shape) || excludes.has(shape.id)) {
      continue;
    }
    const box = getBounds(shape);
    if (!box) {
      continue;
    }
    xs.push(box.x, box.x + box.w / 2, box.x + box.w);
    ys.push(box.y, box.y + box.h / 2, box.y + box.h);
  }
  return { xs, ys };
}

function snapAxisToReferences(value, refs, tolerance) {
  let best = null;
  for (const ref of refs) {
    const delta = ref - value;
    const abs = Math.abs(delta);
    if (abs > tolerance) {
      continue;
    }
    if (!best || abs < best.abs) {
      best = { value: ref, delta, abs };
    }
  }
  return best;
}

function snapPoint(p, options = {}) {
  const point = {
    x: Number(p && p.x) || 0,
    y: Number(p && p.y) || 0
  };
  const showGuides = options.showGuides !== false;
  if (!state.snapGrid) {
    if (showGuides) {
      clearSnapGuides();
    }
    return point;
  }

  if (options.gridSnap !== false) {
    point.x = Math.round(point.x / GRID_SNAP) * GRID_SNAP;
    point.y = Math.round(point.y / GRID_SNAP) * GRID_SNAP;
  }

  let guideX = null;
  let guideY = null;
  if (options.shapeSnap !== false) {
    const refs = collectReferenceAnchors(options.excludeIds || new Set());
    const tolerance = 10 / Math.max(0.2, state.camera.scale || 1);
    const xSnap = snapAxisToReferences(point.x, refs.xs, tolerance);
    const ySnap = snapAxisToReferences(point.y, refs.ys, tolerance);
    if (xSnap) {
      point.x = xSnap.value;
      guideX = xSnap.value;
    }
    if (ySnap) {
      point.y = ySnap.value;
      guideY = ySnap.value;
    }
  }

  if (showGuides) {
    setSnapGuides(guideX, guideY);
  }
  return point;
}

function toWorld(px, py) {
  const rect = getStageRect();
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
    opacity: state.opacity,
    fill: state.fillEnabled,
    fillColor: state.fillColor
  };
}

function viewportCenterWorld() {
  const worldW = (els.stage.width / DPR) / state.camera.scale;
  const worldH = (els.stage.height / DPR) / state.camera.scale;
  return {
    x: state.camera.x + worldW / 2,
    y: state.camera.y + worldH / 2
  };
}

function quickAddShape(type) {
  const center = snapPoint(viewportCenterWorld(), { showGuides: false });
  const style = styleFromControls();
  const id = uid();
  const size = Math.max(60 / state.camera.scale, 1);
  let shape = null;

  if (type === 'rect') {
    shape = {
      id,
      type: 'rect',
      x1: center.x - size * 0.9,
      y1: center.y - size * 0.65,
      x2: center.x + size * 0.9,
      y2: center.y + size * 0.65,
      color: style.color,
      width: style.width,
      style: style.style,
      animated: style.animated,
      opacity: style.opacity,
      fill: style.fill,
      fillColor: style.fillColor,
      radius: 0,
      rotation: 0
    };
  } else if (type === 'ellipse') {
    shape = {
      id,
      type: 'ellipse',
      x1: center.x - size * 0.9,
      y1: center.y - size * 0.65,
      x2: center.x + size * 0.9,
      y2: center.y + size * 0.65,
      color: style.color,
      width: style.width,
      style: style.style,
      animated: style.animated,
      opacity: style.opacity,
      fill: style.fill,
      fillColor: style.fillColor,
      rotation: 0
    };
  } else if (type === 'line' || type === 'arrow') {
    shape = {
      id,
      type,
      x1: center.x - size,
      y1: center.y,
      x2: center.x + size,
      y2: center.y,
      color: style.color,
      width: style.width,
      style: style.style,
      animated: style.animated,
      opacity: style.opacity
    };
  } else if (type === 'text') {
    const rawText = String(state.textContent || 'Text').trim();
    shape = {
      id,
      type: 'text',
      x: center.x,
      y: center.y,
      text: rawText || 'Text',
      size: state.textSize,
      color: style.color,
      opacity: style.opacity
    };
  } else if (type === 'icon') {
    shape = {
      id,
      type: 'icon',
      x: center.x,
      y: center.y,
      size: state.iconSize,
      icon: state.iconType,
      color: style.color,
      opacity: style.opacity
    };
  }

  if (!shape) {
    return;
  }

  if (shouldRevealShape(shape)) {
    shape.revealStartMs = Date.now();
    shape.revealDurationMs = state.drawAnimDurationMs;
  }
  state.shapes.push(shape);
  setSelection([shape.id]);
  trackRecentColor(shape.color);
  pushHistory();
  if (type === 'line' || type === 'arrow' || type === 'rect' || type === 'ellipse' || type === 'text' || type === 'icon') {
    setTool(type);
  } else {
    render();
  }
  const label = {
    rect: 'rectangle',
    ellipse: 'ellipse',
    line: 'line',
    arrow: 'arrow',
    text: 'text',
    icon: 'icon'
  }[type] || 'shape';
  setStatus(`Added ${label}.`);
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
    const length = Math.hypot(dx, dy);
    if (length < 1e-6) {
      return { x: tempShape.x1, y: tempShape.y1 };
    }
    const step = Math.PI / 4; // Snap lines/arrows to 45° increments.
    const angle = Math.atan2(dy, dx);
    const snapped = Math.round(angle / step) * step;
    x = tempShape.x1 + Math.cos(snapped) * length;
    y = tempShape.y1 + Math.sin(snapped) * length;
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

function drawSnapGuides() {
  if (!state.snapGuides) {
    return;
  }
  const hasX = typeof state.snapGuides.x === 'number';
  const hasY = typeof state.snapGuides.y === 'number';
  if (!hasX && !hasY) {
    return;
  }
  const left = state.camera.x;
  const top = state.camera.y;
  const right = left + els.stage.width / DPR / state.camera.scale;
  const bottom = top + els.stage.height / DPR / state.camera.scale;
  ctx.save();
  ctx.strokeStyle = 'rgba(136, 231, 203, 0.95)';
  ctx.lineWidth = 1.25 / state.camera.scale;
  ctx.setLineDash([8 / state.camera.scale, 6 / state.camera.scale]);
  if (hasX) {
    ctx.beginPath();
    ctx.moveTo(state.snapGuides.x, top);
    ctx.lineTo(state.snapGuides.x, bottom);
    ctx.stroke();
  }
  if (hasY) {
    ctx.beginPath();
    ctx.moveTo(left, state.snapGuides.y);
    ctx.lineTo(right, state.snapGuides.y);
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

function shouldRenderShapeSelectionHighlight() {
  if (state.tool !== 'select') {
    return false;
  }
  if (presentMode || state.timeline.playing) {
    return false;
  }
  return true;
}

function drawShape(shape, highlight) {
  if (isShapeHidden(shape)) {
    return;
  }
  const motion = shapeMotionAtTime(shape, state.timeline.currentTime);
  const baseOpacity = typeof shape.opacity === 'number' ? shape.opacity : 1;
  const effectiveOpacity = clamp(baseOpacity * motion.opacity, 0, 1);
  if (effectiveOpacity <= 0.001) {
    return;
  }
  ctx.save();
  ctx.lineCap = 'round';
  ctx.lineJoin = 'round';
  ctx.globalAlpha = effectiveOpacity;
  if (Math.abs(motion.dx) > 1e-6 || Math.abs(motion.dy) > 1e-6) {
    ctx.translate(motion.dx, motion.dy);
  }
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
    const trimStart = clamp(Number(motion.trimStart), 0, 1);
    const trimEnd = clamp(Number(motion.trimEnd), trimStart, 1);
    const revealEnd = trimStart + (trimEnd - trimStart) * revealProgress;
    const sx = shape.x1 + (shape.x2 - shape.x1) * trimStart;
    const sy = shape.y1 + (shape.y2 - shape.y1) * trimStart;
    const ex = shape.x1 + (shape.x2 - shape.x1) * revealEnd;
    const ey = shape.y1 + (shape.y2 - shape.y1) * revealEnd;
    if (revealEnd <= trimStart + 1e-6) {
      ctx.restore();
      return;
    }
    ctx.beginPath();
    ctx.moveTo(sx, sy);
    ctx.lineTo(ex, ey);
    ctx.stroke();
    if (shape.type === 'arrow' && Math.hypot(ex - sx, ey - sy) > 0.5 / state.camera.scale) {
      const prevRatio = Math.max(trimStart, revealEnd - Math.min(0.08, (revealEnd - trimStart) * 0.6));
      const px = shape.x1 + (shape.x2 - shape.x1) * prevRatio;
      const py = shape.y1 + (shape.y2 - shape.y1) * prevRatio;
      drawArrowHead(px, py, ex, ey, color, width);
    }
  } else if (shape.type === 'rect') {
    const rotation = Number(shape.rotation) || 0;
    const x2 = shape.x1 + (shape.x2 - shape.x1) * revealProgress;
    const y2 = shape.y1 + (shape.y2 - shape.y1) * revealProgress;
    const x = Math.min(shape.x1, x2);
    const y = Math.min(shape.y1, y2);
    const w = Math.abs(x2 - shape.x1);
    const h = Math.abs(y2 - shape.y1);
    const radius = Math.max(0, Number(shape.radius) || 0);
    if (Math.abs(rotation) > 1e-4) {
      const rcx = x + w / 2;
      const rcy = y + h / 2;
      ctx.translate(rcx, rcy);
      ctx.rotate(rotation);
      ctx.translate(-rcx, -rcy);
    }
    const hasFill = !!shape.fill && shape.fillColor;
    if (hasFill) {
      ctx.fillStyle = shape.fillColor;
    }
    if (radius > 0.1 && w > 0.5 && h > 0.5) {
      roundRect(x, y, w, h, radius, hasFill, true);
    } else {
      if (hasFill) {
        ctx.fillRect(x, y, w, h);
      }
      ctx.strokeRect(x, y, w, h);
    }
  } else if (shape.type === 'ellipse') {
    const rotation = Number(shape.rotation) || 0;
    const x2 = shape.x1 + (shape.x2 - shape.x1) * revealProgress;
    const y2 = shape.y1 + (shape.y2 - shape.y1) * revealProgress;
    const cx = (shape.x1 + x2) / 2;
    const cy = (shape.y1 + y2) / 2;
    const rx = Math.abs(x2 - shape.x1) / 2;
    const ry = Math.abs(y2 - shape.y1) / 2;
    ctx.beginPath();
    ctx.ellipse(cx, cy, rx, ry, rotation, 0, Math.PI * 2);
    if (shape.fill && shape.fillColor) {
      ctx.fillStyle = shape.fillColor;
      ctx.fill();
    }
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

  if (highlight && shouldRenderShapeSelectionHighlight()) {
    const box = getBounds(shape, { includeMotion: false });
    if (box) {
      const pad = 4 / state.camera.scale;
      ctx.setLineDash([6 / state.camera.scale, 5 / state.camera.scale]);
      ctx.lineWidth = 1.0 / state.camera.scale;
      ctx.strokeStyle = 'rgba(176, 204, 255, 0.45)';
      ctx.strokeRect(box.x - pad, box.y - pad, box.w + pad * 2, box.h + pad * 2);
    }
  }

  ctx.restore();
}

function shapeBoundsCacheKey(shape) {
  if (!shape || !shape.id) {
    return '';
  }
  return `${state.activeArtboardId}:${shape.id}`;
}

function computeBounds(shape) {
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

function getBounds(shape, options = {}) {
  const includeMotion = options.includeMotion !== false;
  const key = shapeBoundsCacheKey(shape);
  if (key && shapeBoundsCache.has(key)) {
    const cached = shapeBoundsCache.get(key);
    if (!includeMotion) {
      return cached;
    }
    const motion = shapeMotionAtTime(shape, state.timeline.currentTime);
    if (!motion.enabled) {
      return cached;
    }
    return {
      x: cached.x + motion.dx,
      y: cached.y + motion.dy,
      w: cached.w,
      h: cached.h
    };
  }
  const bounds = computeBounds(shape);
  if (key && bounds) {
    shapeBoundsCache.set(key, bounds);
  }
  if (bounds && includeMotion) {
    const motion = shapeMotionAtTime(shape, state.timeline.currentTime);
    if (motion.enabled) {
      return {
        x: bounds.x + motion.dx,
        y: bounds.y + motion.dy,
        w: bounds.w,
        h: bounds.h
      };
    }
  }
  return bounds;
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
  if (!shape || isShapeHidden(shape) || isShapeLocked(shape)) {
    return false;
  }
  const motion = shapeMotionAtTime(shape, state.timeline.currentTime);
  if (motion.opacity <= 0.001) {
    return false;
  }
  const localX = x - motion.dx;
  const localY = y - motion.dy;
  const tol = Math.max(8 / state.camera.scale, (shape.width || 3) + 3);
  if (shape.type === 'pen') {
    const pts = shape.points || [];
    for (let i = 1; i < pts.length; i += 1) {
      if (distToSegment(localX, localY, pts[i - 1].x, pts[i - 1].y, pts[i].x, pts[i].y) <= tol) {
        return true;
      }
    }
    return false;
  }

  if (shape.type === 'line' || shape.type === 'arrow') {
    return distToSegment(localX, localY, shape.x1, shape.y1, shape.x2, shape.y2) <= tol;
  }

  if (shape.type === 'rect' || shape.type === 'ellipse' || shape.type === 'text' || shape.type === 'icon') {
    const box = getBounds(shape, { includeMotion: false });
    if (!box) {
      return false;
    }
    return localX >= box.x - tol && localX <= box.x + box.w + tol && localY >= box.y - tol && localY <= box.y + box.h + tol;
  }
  return false;
}

function spatialCellKey(xCell, yCell) {
  return `${xCell}:${yCell}`;
}

function buildSpatialIndex() {
  // Coarse cell index narrows hit-tests before precise geometry checks.
  const cells = new Map();
  for (const shape of state.shapes) {
    if (!isShapeSelectable(shape)) {
      continue;
    }
    const box = getBounds(shape, { includeMotion: false });
    if (!box) {
      continue;
    }
    const extents = shapeMotionOffsetExtents(shape);
    const grow = Math.max(SPATIAL_INDEX_GROW, (shape.width || 3) + 10);
    const minX = box.x + extents.minDx - grow;
    const minY = box.y + extents.minDy - grow;
    const maxX = box.x + box.w + extents.maxDx + grow;
    const maxY = box.y + box.h + extents.maxDy + grow;
    const minCellX = Math.floor(minX / SPATIAL_INDEX_CELL);
    const minCellY = Math.floor(minY / SPATIAL_INDEX_CELL);
    const maxCellX = Math.floor(maxX / SPATIAL_INDEX_CELL);
    const maxCellY = Math.floor(maxY / SPATIAL_INDEX_CELL);
    for (let cy = minCellY; cy <= maxCellY; cy += 1) {
      for (let cx = minCellX; cx <= maxCellX; cx += 1) {
        const key = spatialCellKey(cx, cy);
        let bucket = cells.get(key);
        if (!bucket) {
          bucket = new Set();
          cells.set(key, bucket);
        }
        bucket.add(shape.id);
      }
    }
  }
  spatialIndexCache = {
    artboardId: state.activeArtboardId,
    shapeCount: state.shapes.length,
    cells
  };
  spatialIndexDirty = false;
  return spatialIndexCache;
}

function ensureSpatialIndex() {
  if (
    !spatialIndexDirty &&
    spatialIndexCache &&
    spatialIndexCache.artboardId === state.activeArtboardId &&
    spatialIndexCache.shapeCount === state.shapes.length
  ) {
    return spatialIndexCache;
  }
  return buildSpatialIndex();
}

function spatialCandidatesAt(x, y) {
  const index = ensureSpatialIndex();
  if (!index) {
    return null;
  }
  const cellX = Math.floor(x / SPATIAL_INDEX_CELL);
  const cellY = Math.floor(y / SPATIAL_INDEX_CELL);
  const direct = index.cells.get(spatialCellKey(cellX, cellY));
  return direct || null;
}

function findShapeAt(x, y) {
  const candidates = spatialCandidatesAt(x, y);
  for (let i = state.shapes.length - 1; i >= 0; i -= 1) {
    const shape = state.shapes[i];
    if (!isShapeSelectable(shape)) {
      continue;
    }
    if (candidates && !candidates.has(shape.id)) {
      continue;
    }
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

function snapshotShapeForDrag(shape) {
  if (!shape) {
    return null;
  }
  const base = { type: shape.type };
  if (shape.type === 'pen') {
    base.points = (shape.points || []).map((p) => ({ x: p.x, y: p.y }));
  } else if (shape.type === 'text' || shape.type === 'icon') {
    base.x = Number(shape.x) || 0;
    base.y = Number(shape.y) || 0;
  } else if (typeof shape.x1 === 'number') {
    base.x1 = Number(shape.x1) || 0;
    base.y1 = Number(shape.y1) || 0;
    base.x2 = Number(shape.x2) || 0;
    base.y2 = Number(shape.y2) || 0;
  } else {
    return null;
  }
  const box = getBounds(shape);
  if (!box) {
    return null;
  }
  base.bounds = { x: box.x, y: box.y, w: box.w, h: box.h };
  return base;
}

function applyShapeFromSnapshot(shape, snapshot, dx, dy) {
  if (!shape || !snapshot) {
    return;
  }
  if (snapshot.type === 'pen' && Array.isArray(snapshot.points)) {
    if (!Array.isArray(shape.points) || shape.points.length !== snapshot.points.length) {
      shape.points = snapshot.points.map((p) => ({ x: p.x + dx, y: p.y + dy }));
      return;
    }
    for (let i = 0; i < snapshot.points.length; i += 1) {
      shape.points[i].x = snapshot.points[i].x + dx;
      shape.points[i].y = snapshot.points[i].y + dy;
    }
    return;
  }
  if (snapshot.type === 'text' || snapshot.type === 'icon') {
    shape.x = snapshot.x + dx;
    shape.y = snapshot.y + dy;
    return;
  }
  if (typeof snapshot.x1 === 'number') {
    shape.x1 = snapshot.x1 + dx;
    shape.y1 = snapshot.y1 + dy;
    shape.x2 = snapshot.x2 + dx;
    shape.y2 = snapshot.y2 + dy;
  }
}

function buildSelectionDragState(shapes, startWorld) {
  const ids = [];
  const snapshots = new Map();
  let minX = Infinity;
  let minY = Infinity;
  let maxX = -Infinity;
  let maxY = -Infinity;
  for (const shape of shapes || []) {
    if (!shape || isShapeLocked(shape) || isShapeHidden(shape)) {
      continue;
    }
    const snap = snapshotShapeForDrag(shape);
    if (!snap || !snap.bounds) {
      continue;
    }
    ids.push(shape.id);
    snapshots.set(shape.id, snap);
    minX = Math.min(minX, snap.bounds.x);
    minY = Math.min(minY, snap.bounds.y);
    maxX = Math.max(maxX, snap.bounds.x + snap.bounds.w);
    maxY = Math.max(maxY, snap.bounds.y + snap.bounds.h);
  }
  if (!ids.length) {
    return null;
  }
  return {
    ids,
    snapshots,
    startWorld: { x: Number(startWorld.x) || 0, y: Number(startWorld.y) || 0 },
    appliedDx: 0,
    appliedDy: 0,
    moved: false,
    bounds: {
      x: minX,
      y: minY,
      w: Math.max(0, maxX - minX),
      h: Math.max(0, maxY - minY)
    }
  };
}

function snapSelectionDragDelta(drag, desiredDx, desiredDy) {
  let dx = desiredDx;
  let dy = desiredDy;
  if (!state.snapGrid || !drag) {
    clearSnapGuides();
    return { dx, dy };
  }

  if (drag.bounds) {
    const centerX = drag.bounds.x + drag.bounds.w / 2 + dx;
    const centerY = drag.bounds.y + drag.bounds.h / 2 + dy;
    const gridX = Math.round(centerX / GRID_SNAP) * GRID_SNAP;
    const gridY = Math.round(centerY / GRID_SNAP) * GRID_SNAP;
    dx += gridX - centerX;
    dy += gridY - centerY;
  }

  const references = collectReferenceAnchors(new Set(drag.ids));
  const tolerance = 12 / Math.max(0.2, state.camera.scale || 1);
  let bestX = null;
  let bestY = null;

  for (const id of drag.ids) {
    const snap = drag.snapshots.get(id);
    if (!snap || !snap.bounds) {
      continue;
    }
    const anchorsX = [
      snap.bounds.x + dx,
      snap.bounds.x + snap.bounds.w / 2 + dx,
      snap.bounds.x + snap.bounds.w + dx
    ];
    const anchorsY = [
      snap.bounds.y + dy,
      snap.bounds.y + snap.bounds.h / 2 + dy,
      snap.bounds.y + snap.bounds.h + dy
    ];
    for (const anchor of anchorsX) {
      const found = snapAxisToReferences(anchor, references.xs, tolerance);
      if (found && (!bestX || found.abs < bestX.abs)) {
        bestX = found;
      }
    }
    for (const anchor of anchorsY) {
      const found = snapAxisToReferences(anchor, references.ys, tolerance);
      if (found && (!bestY || found.abs < bestY.abs)) {
        bestY = found;
      }
    }
  }

  if (bestX) {
    dx += bestX.delta;
  }
  if (bestY) {
    dy += bestY.delta;
  }
  setSnapGuides(bestX ? bestX.value : null, bestY ? bestY.value : null);
  return { dx, dy };
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
  const lockedIds = new Set(state.shapes.filter((shape) => isShapeLocked(shape)).map((shape) => shape.id));
  const deletableIds = ids.filter((id) => !lockedIds.has(id));
  if (!deletableIds.length) {
    setStatus('Selected shapes are locked.');
    return;
  }
  const countBefore = state.shapes.length;
  const keepSelected = ids.filter((id) => lockedIds.has(id));
  state.shapes = state.shapes.filter((s) => !deletableIds.includes(s.id));
  setSelection(keepSelected);
  if (state.shapes.length !== countBefore) {
    pushHistory();
    const lockedMsg = keepSelected.length ? ` (${keepSelected.length} locked kept)` : '';
    setStatus(`Deleted ${deletableIds.length} shape(s)${lockedMsg}.`);
  }
}

function clearAll() {
  state.shapes = [];
  clearSelection();
  pushHistory();
  setStatus('Artboard cleared.');
}

function buildAutosavePayload() {
  return {
    version: 2,
    artboards: state.artboards,
    activeArtboardId: state.activeArtboardId,
    grid: state.grid,
    timeline: {
      currentTime: state.timeline.currentTime,
      duration: state.timeline.duration,
      zoomPxPerSec: state.timeline.zoomPxPerSec,
      snapSec: state.timeline.snapSec,
      captionStyle: state.timeline.captionStyle,
      transcriptSegments: state.timeline.transcriptSegments,
      transcriptName: state.timeline.transcriptName,
      audioName: state.timeline.audioName
    },
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
      recentColors: state.recentColors,
      panelCollapsed: state.panelCollapsed,
      canvasFormat,
      captionPlacementVertical: captionPlacement.vertical,
      captionPlacementOffsetPct: captionPlacement.offsetPct,
      captionPlacementZoneVisible: captionPlacement.zoneVisible,
      webcamPosition: webcamState.position,
      webcamShape: webcamState.shape,
      webcamSize: webcamState.size
    }
  };
}

function persistAutosaveNow() {
  if (!autosaveDirty) {
    return;
  }
  autosaveDirty = false;
  try {
    localStorage.setItem(STORAGE_KEY, JSON.stringify(buildAutosavePayload()));
  } catch (err) {
    autosaveDirty = true;
    const quotaExceeded = !!(err && (err.name === 'QuotaExceededError' || err.code === 22 || err.code === 1014));
    const nowMs = Date.now();
    // Rate-limit status spam when a slider repeatedly saves.
    if (nowMs - autosaveErrorStampMs > 1600) {
      setStatus(quotaExceeded
        ? 'Autosave storage is full. Export JSON to keep your work safe.'
        : 'Autosave failed. Check browser storage permissions.');
      autosaveErrorStampMs = nowMs;
    }
  }
}

function saveToLocalStorage(options = {}) {
  autosaveDirty = true;
  if (options.immediate) {
    if (autosaveTimer) {
      clearTimeout(autosaveTimer);
      autosaveTimer = null;
    }
    persistAutosaveNow();
    return;
  }
  if (autosaveTimer) {
    clearTimeout(autosaveTimer);
  }
  autosaveTimer = setTimeout(() => {
    autosaveTimer = null;
    persistAutosaveNow();
  }, AUTOSAVE_DEBOUNCE_MS);
}

function flushAutosaveOnExit() {
  if (autosaveTimer) {
    clearTimeout(autosaveTimer);
    autosaveTimer = null;
  }
  persistAutosaveNow();
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
    if (payload.timeline) {
      state.timeline.currentTime = Math.max(0, Number(payload.timeline.currentTime) || 0);
      state.timeline.duration = Math.max(1, Number(payload.timeline.duration) || state.timeline.duration);
      state.timeline.playing = false;
      state.timeline.lastTickMs = 0;
      const rawZoom = Number(payload.timeline.zoomPxPerSec);
      const rawSnap = Number(payload.timeline.snapSec);
      state.timeline.zoomPxPerSec = clamp(Number.isFinite(rawZoom) ? rawZoom : state.timeline.zoomPxPerSec, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC);
      state.timeline.snapSec = Number.isFinite(rawSnap) ? Math.max(0, rawSnap) : state.timeline.snapSec;
      state.timeline.captionStyle = normalizeCaptionStyle(payload.timeline.captionStyle, captionDefaultStyle());
      state.timeline.transcriptSegments = normalizeTranscriptPayload({ segments: payload.timeline.transcriptSegments });
      state.timeline.transcriptName = payload.timeline.transcriptName || '';
      state.timeline.audioName = payload.timeline.audioName || '';
      state.timeline.currentTime = clamp(state.timeline.currentTime, 0, state.timeline.duration);
    }
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
      state.panelCollapsed = normalizeCollapsedPanels(payload.ui.panelCollapsed || state.panelCollapsed);
      if (['16:9', '9:16', '1:1'].includes(payload.ui.canvasFormat)) {
        canvasFormat = payload.ui.canvasFormat;
      }
      if (['top', 'center', 'bottom'].includes(payload.ui.captionPlacementVertical)) {
        captionPlacement.vertical = payload.ui.captionPlacementVertical;
      }
      if (Number.isFinite(Number(payload.ui.captionPlacementOffsetPct))) {
        captionPlacement.offsetPct = clamp(Number(payload.ui.captionPlacementOffsetPct), 2, 48);
      }
      if (typeof payload.ui.captionPlacementZoneVisible === 'boolean') {
        captionPlacement.zoneVisible = payload.ui.captionPlacementZoneVisible;
      }
      if (['br', 'bl', 'tr', 'tl'].includes(payload.ui.webcamPosition)) {
        webcamState.position = payload.ui.webcamPosition;
      }
      if (['circle', 'rounded'].includes(payload.ui.webcamShape)) {
        webcamState.shape = payload.ui.webcamShape;
      }
      if (Number.isFinite(Number(payload.ui.webcamSize))) {
        webcamState.size = clamp(Number(payload.ui.webcamSize), 60, 200);
      }
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
    shapes: board.shapes,
    timeline: {
      currentTime: state.timeline.currentTime,
      duration: state.timeline.duration,
      zoomPxPerSec: state.timeline.zoomPxPerSec,
      snapSec: state.timeline.snapSec,
      captionStyle: state.timeline.captionStyle,
      transcriptSegments: state.timeline.transcriptSegments,
      transcriptName: state.timeline.transcriptName
    }
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

  // Preserve canvas motion by emitting equivalent storyboard timeline actions.
  for (const shape of primitives) {
    if (!shape || !shape.id || !shape.motion || !shape.motion.enabled) {
      continue;
    }
    const motion = normalizeShapeMotion(shape.motion);
    const keyframes = Array.isArray(motion.keyframes) ? motion.keyframes : [];
    if (!keyframes.length) {
      continue;
    }
    const first = keyframes[0];
    if (Math.abs(first.dx) > 1e-6 || Math.abs(first.dy) > 1e-6) {
      timeline.push({
        at: roundTimelineSeconds(first.at),
        do: 'move',
        targets: [shape.id],
        duration: 0,
        dx: roundTimelineSeconds(first.dx),
        dy: roundTimelineSeconds(first.dy)
      });
    }
    if (Math.abs(first.opacity - 1) > 1e-6) {
      timeline.push({
        at: roundTimelineSeconds(first.at),
        do: 'show',
        targets: [shape.id],
        duration: 0,
        from: 1,
        to: roundTimelineSeconds(first.opacity),
        ease: motion.ease
      });
    }
    for (let i = 0; i < keyframes.length - 1; i += 1) {
      const left = keyframes[i];
      const right = keyframes[i + 1];
      const duration = Math.max(0, roundTimelineSeconds(right.at - left.at));
      if (duration <= 0) {
        continue;
      }
      const deltaX = right.dx - left.dx;
      const deltaY = right.dy - left.dy;
      if (Math.abs(deltaX) > 1e-6 || Math.abs(deltaY) > 1e-6) {
        timeline.push({
          at: roundTimelineSeconds(left.at),
          do: 'move',
          targets: [shape.id],
          duration,
          dx: roundTimelineSeconds(deltaX),
          dy: roundTimelineSeconds(deltaY),
          ease: left.ease || motion.ease
        });
      }
      if (Math.abs(left.opacity - right.opacity) > 1e-6) {
        timeline.push({
          at: roundTimelineSeconds(left.at),
          do: 'show',
          targets: [shape.id],
          duration,
          from: roundTimelineSeconds(left.opacity),
          to: roundTimelineSeconds(right.opacity),
          ease: left.ease || motion.ease
        });
      }
    }
  }

  timeline.push({ at: 3.0, do: 'zoom', duration: 0.65, to: 1.15, ease: 'easeInOutCubic' });
  timeline.push({ at: 3.8, do: 'pan', duration: 0.65, x: state.camera.x - 80, y: state.camera.y - 20, ease: 'easeInOutCubic' });
  timeline.sort((a, b) => (Number(a.at) || 0) - (Number(b.at) || 0));

  let endTime = 0;
  for (const action of timeline) {
    endTime = Math.max(endTime, (Number(action.at) || 0) + Math.max(0, Number(action.duration) || 0));
  }

  const payload = {
    version: 1,
    meta: {
      title: `${currentArtboard().name} Storyboard`,
      bg: state.bgColor,
      duration: Math.max(5.5, roundTimelineSeconds(endTime + 0.4)),
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
      if (payload.timeline) {
        state.timeline.currentTime = Math.max(0, Number(payload.timeline.currentTime) || 0);
        state.timeline.duration = Math.max(1, Number(payload.timeline.duration) || 1);
        const rawZoom = Number(payload.timeline.zoomPxPerSec);
        const rawSnap = Number(payload.timeline.snapSec);
        state.timeline.zoomPxPerSec = clamp(Number.isFinite(rawZoom) ? rawZoom : state.timeline.zoomPxPerSec, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC);
        state.timeline.snapSec = Number.isFinite(rawSnap) ? Math.max(0, rawSnap) : state.timeline.snapSec;
        state.timeline.captionStyle = normalizeCaptionStyle(payload.timeline.captionStyle, captionDefaultStyle());
        state.timeline.transcriptSegments = normalizeTranscriptPayload({ segments: payload.timeline.transcriptSegments });
        state.timeline.transcriptName = payload.timeline.transcriptName || '';
        state.timeline.audioName = '';
        state.timeline.playing = false;
        state.timeline.lastTickMs = 0;
        state.timeline.currentTime = clamp(state.timeline.currentTime, 0, state.timeline.duration);
      } else {
        state.timeline.playing = false;
      }
      clearSelection();
      pushHistory();
      renderArtboardTabs();
      syncControls();
      initTimelineUI();
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

function toSRTTime(sec) {
  const totalMs = Math.round(Math.max(0, sec) * 1000);
  const ms = totalMs % 1000;
  const s = Math.floor(totalMs / 1000) % 60;
  const m = Math.floor(totalMs / 60000) % 60;
  const h = Math.floor(totalMs / 3600000);
  return `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')},${String(ms).padStart(3, '0')}`;
}

function exportCaptionsAsSRT() {
  const segments = state.timeline.transcriptSegments;
  if (!segments || !segments.length) {
    setStatus('No captions to export. Load a transcript or generate captions first.');
    return;
  }
  const lines = [];
  segments.forEach((seg, idx) => {
    const start = typeof seg.start === 'number' ? seg.start : 0;
    const end = typeof seg.end === 'number' ? seg.end : start + 1;
    const text = String(seg.text || '').trim();
    if (!text) {
      return;
    }
    lines.push(`${idx + 1}`);
    lines.push(`${toSRTTime(start)} --> ${toSRTTime(end)}`);
    lines.push(text);
    lines.push('');
  });
  const srtContent = lines.join('\n');
  const blob = new Blob([srtContent], { type: 'text/plain' });
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = `${state.timeline.transcriptName || 'captions'}.srt`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
  setStatus(`Exported ${segments.length} caption(s) as SRT.`);
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
    if (isShapeHidden(shape)) {
      continue;
    }
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
  invalidateStageRectCache();
  const isMiddle = event.button === 1;
  const panWithSpace = state.spacePan;
  const shouldPan = isMiddle || panWithSpace || state.tool === 'pan';

  state.pointer.down = true;
  state.pointer.start = { x: event.clientX, y: event.clientY };
  state.pointer.current = { x: event.clientX, y: event.clientY };
  state.pointer.selectionDrag = null;
  clearSnapGuides();

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
      const singleOnly = !!event.ctrlKey || !!event.metaKey;
      const targetIds = (!singleOnly && hit.groupId) ? selectionIdsForShape(hit) : [hit.id];
      if (shift) {
        const allAlreadySelected = targetIds.every((id) => selected.includes(id));
        if (allAlreadySelected) {
          const removeSet = new Set(targetIds);
          setSelection(selected.filter((id) => !removeSet.has(id)));
        } else {
          const next = selected.slice();
          for (const id of targetIds) {
            if (!next.includes(id)) {
              next.push(id);
            }
          }
          setSelection(next);
        }
      } else if (alt) {
        const removeSet = new Set(targetIds);
        if (targetIds.some((id) => selected.includes(id))) {
          setSelection(selected.filter((id) => !removeSet.has(id)));
        }
      } else {
        setSelection(targetIds);
      }

      const nowSelected = selectedShapeIds();
      if (!alt && targetIds.some((id) => nowSelected.includes(id))) {
        state.draggingSelected = true;
        state.pointer.lastWorld = world;
        const dragShapes = nowSelected
          .map((id) => state.shapes.find((shape) => shape.id === id))
          .filter((shape) => !!shape && !isShapeLocked(shape) && !isShapeHidden(shape));
        state.pointer.selectionDrag = buildSelectionDragState(dragShapes, world);
      } else {
        state.draggingSelected = false;
        state.pointer.lastWorld = null;
        state.pointer.selectionDrag = null;
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
    clearSnapGuides();
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
    if (applyHandleDrag(world, event.shiftKey)) {
      invalidateShapeCaches();
    }
    render();
    return;
  }

  if (state.pointer.marquee) {
    state.pointer.marquee.current = world;
    render();
    return;
  }

  if (state.draggingSelected && selectedShapeIds().length) {
    const drag = state.pointer.selectionDrag;
    if (drag && drag.ids.length) {
      const desiredDx = world.x - drag.startWorld.x;
      const desiredDy = world.y - drag.startWorld.y;
      const snapped = snapSelectionDragDelta(drag, desiredDx, desiredDy);
      const changed = Math.abs(snapped.dx - drag.appliedDx) > 1e-6 || Math.abs(snapped.dy - drag.appliedDy) > 1e-6;
      if (changed) {
        for (const id of drag.ids) {
          const shape = state.shapes.find((item) => item.id === id);
          const snapshot = drag.snapshots.get(id);
          if (!shape || !snapshot) {
            continue;
          }
          applyShapeFromSnapshot(shape, snapshot, snapped.dx, snapped.dy);
        }
        drag.appliedDx = snapped.dx;
        drag.appliedDy = snapped.dy;
        drag.moved = true;
        invalidateShapeCaches();
      }
      render();
      return;
    }

    const selected = selectedShapes().filter((shape) => !isShapeLocked(shape));
    if (selected.length && state.pointer.lastWorld) {
      const dx = world.x - state.pointer.lastWorld.x;
      const dy = world.y - state.pointer.lastWorld.y;
      for (const shape of selected) {
        moveShape(shape, dx, dy);
      }
      state.pointer.lastWorld = world;
      invalidateShapeCaches();
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
    const movedBySnapshot = !!(state.pointer.selectionDrag && state.pointer.selectionDrag.moved);
    const movedByFallback = !state.pointer.selectionDrag &&
      !!state.pointer.start &&
      !!state.pointer.current &&
      Math.hypot(
        (state.pointer.current.x || 0) - (state.pointer.start.x || 0),
        (state.pointer.current.y || 0) - (state.pointer.start.y || 0)
      ) > 1;
    state.draggingSelected = false;
    if (movedBySnapshot || movedByFallback) {
      pushHistory();
      setStatus('Moved selected shape(s).');
    }
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
  state.pointer.selectionDrag = null;
  clearSnapGuides();
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
  invalidateStageRectCache();
  const rect = els.canvasWrap.getBoundingClientRect();
  els.stage.width = Math.max(1, Math.floor(rect.width * DPR));
  els.stage.height = Math.max(1, Math.floor(rect.height * DPR));
  ctx.setTransform(DPR, 0, 0, DPR, 0, 0);
  updateFormatGuide();
  updateCaptionOverlayGeometry();
  positionTeleprompter();
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
  requestAnimationFrame(resizeCanvas);
}

// ── Present / Recording Mode ────────────────────────────────────────────────

let presentMode = false;
let recTimerInterval = null;
let recStartTime = 0;

function enterPresentMode() {
  presentMode = true;
  document.body.classList.add('present-mode', 'focus');
  if (els.btnPresent) els.btnPresent.textContent = 'Exit Present';
  requestAnimationFrame(() => {
    resizeCanvas();
    positionTeleprompter();
    updateTeleprompter();
  });
}

function exitPresentMode() {
  presentMode = false;
  document.body.classList.remove('present-mode', 'is-playing');
  if (state.timeline.playing) setTimelinePlaying(false);
  stopRecTimer();
  if (els.btnPresent) els.btnPresent.textContent = 'Present';
  requestAnimationFrame(resizeCanvas);
}

function togglePresentMode() {
  if (presentMode) {
    exitPresentMode();
  } else {
    enterPresentMode();
  }
}

// ── Caption Styles Float Panel ───────────────────────────────────────────────

const CFP_LABELS = {
  typewriter:     'Typewriter',
  karaoke_classic:'Karaoke',
  impact_yellow:  'Impact: Yellow',
  clean_paragraph:'Classic',
  bold_two_words: 'Bold: Two Words',
  capcut_black:   'CapCut Pills',
  word_pop:       'Word Pop',
  slide_up:       'Slide Up',
  custom:         'Custom'
};

let cfpApplyMode = 'selected'; // 'selected' | 'all'

function buildCfpCardPreview(id, p, previewWords) {
  const [w0, w1, w2, w3] = previewWords && previewWords.length
    ? [...previewWords, ...['', '', '', '']]
    : ['how', 'can', 'i', 'make'];
  const bgHex = p.overlayBgColor || '#111';
  const alpha = Math.round(clamp(Number(p.overlayBgOpacity) || 0, 0, 1) * 255)
    .toString(16).padStart(2, '0');
  const bg = p.mode === 'capcut_black' ? 'transparent' : (bgHex + alpha);
  const ff = p.fontFamily ? `'${p.fontFamily}',sans-serif` : 'inherit';
  const sw = Number(p.strokeWidth) || 0;
  const stroke = sw > 0
    ? `${p.strokeColor} -${sw * 0.5}px -${sw * 0.5}px 0, ${p.strokeColor} ${sw * 0.5}px -${sw * 0.5}px 0, ${p.strokeColor} -${sw * 0.5}px ${sw * 0.5}px 0, ${p.strokeColor} ${sw * 0.5}px ${sw * 0.5}px 0`
    : 'none';
  const hff = p.highlightFontFamily ? `'${p.highlightFontFamily}',sans-serif` : ff;
  const hfw = Number(p.highlightFontWeight) || Number(p.fontWeight) || 700;
  const hpy = clamp(Number(p.highlightPadY) || 0, 0, 18);
  const hpx = clamp(Number(p.highlightPadX) || 3, 0, 18);
  const hrad = clamp(Number(p.highlightRadius) || 3, 0, 24);
  const esc = (s) => String(s || '').replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  const baseStyle = `color:${p.textColor};font-weight:${p.fontWeight};font-family:${ff};font-size:14px;line-height:1.3;text-shadow:${stroke}`;
  const hlStyle = `background:${p.highlightColor};color:${p.highlightTextColor};border-radius:${hrad}px;padding:${hpy}px ${hpx}px;font-family:${hff};font-weight:${hfw}`;

  let inner;
  if (id === 'capcut_black') {
    inner = `<span style="${baseStyle};background:rgba(0,0,0,0.84);border-radius:.28em;padding:2px 6px;margin:1px">${esc(w0)}</span>`
          + `<span style="${baseStyle};background:${p.highlightColor};color:${p.highlightTextColor};border-radius:${hrad}px;padding:${Math.max(2, hpy)}px ${Math.max(6, hpx)}px;margin:1px;font-family:${hff};font-weight:${hfw}">${esc(w1)}</span>`
          + (w2 ? `<span style="${baseStyle};background:rgba(0,0,0,0.84);border-radius:.28em;padding:2px 6px;margin:1px">${esc(w2)}</span>` : '');
  } else if (id === 'typewriter') {
    inner = `<span style="${baseStyle}">${esc(w0)} ${esc(w1)}▌</span>`;
  } else if (p.wordHighlight) {
    inner = `<span style="${baseStyle}">${esc(w0)} <mark style="${hlStyle}">${esc(w1)}</mark>${w2 ? ` ${esc(w2)}` : ''}</span>`;
  } else {
    const words = [w0, w1, w2, w3].filter(Boolean).join(' ');
    inner = `<span style="${baseStyle}">${esc(words)}</span>`;
  }
  return `<div class="cfp-card-preview" style="background:${bg}">${inner}</div>`;
}

function buildCfpCards() {
  if (!els.cfpScroll) return;
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  els.cfpScroll.innerHTML = '';

  // Use real transcript text for the preview so users see how their words look
  const segs = state.timeline.transcriptSegments || [];
  const activeSeg = segs[timelineRuntime.activeIndex >= 0 ? timelineRuntime.activeIndex : 0] || segs[0];
  let previewWords = ['how', 'can', 'i', 'make'];
  if (activeSeg && activeSeg.text) {
    const tokens = activeSeg.text.trim().split(/\s+/).filter(Boolean);
    if (tokens.length >= 2) {
      previewWords = tokens.slice(0, 4);
    }
  }

  for (const [id, preset] of Object.entries(CAPTION_TEMPLATE_PRESETS)) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'cfp-card';
    btn.dataset.template = id;
    btn.classList.toggle('active', style.template === id);
    btn.innerHTML = buildCfpCardPreview(id, preset, previewWords)
      + `<div class="cfp-card-name">${CFP_LABELS[id] || id}</div>`;
    btn.addEventListener('click', () => {
      setCaptionTemplate(id);
      if (cfpApplyMode === 'all') {
        applyCaptionStyleToAllTimelineSegments();
      } else {
        applyCaptionStyleToSelectedTimelineSegments();
      }
      syncCfpActiveCard();
    });
    els.cfpScroll.appendChild(btn);
  }
}

function syncCfpActiveCard() {
  if (!els.cfpScroll) return;
  const style = normalizeCaptionStyle(state.timeline.captionStyle, captionDefaultStyle());
  for (const btn of els.cfpScroll.querySelectorAll('.cfp-card')) {
    btn.classList.toggle('active', btn.dataset.template === style.template);
  }
}

function openCaptionFloatPanel() {
  buildCfpCards();
  if (els.captionFloatPanel) els.captionFloatPanel.classList.add('open');
  if (els.cfpBackdrop) els.cfpBackdrop.classList.add('open');
}

function closeCaptionFloatPanel() {
  if (els.captionFloatPanel) els.captionFloatPanel.classList.remove('open');
  if (els.cfpBackdrop) els.cfpBackdrop.classList.remove('open');
}

// ────────────────────────────────────────────────────────────────────────────

function stopRecTimer() {
  if (recTimerInterval) {
    clearInterval(recTimerInterval);
    recTimerInterval = null;
  }
  if (els.recTime) els.recTime.textContent = '0:00';
}

function startRecTimer() {
  stopRecTimer();
  recStartTime = Date.now();
  recTimerInterval = setInterval(() => {
    if (!els.recTime) return;
    const elapsed = Math.floor((Date.now() - recStartTime) / 1000);
    const m = Math.floor(elapsed / 60);
    const s = elapsed % 60;
    els.recTime.textContent = `${m}:${s.toString().padStart(2, '0')}`;
  }, 500);
}

function startPresentCountdown(cb) {
  const overlay = els.countdownOverlay;
  const numEl = els.countdownNum;
  if (!overlay || !numEl) { cb(); return; }

  let count = 3;
  overlay.classList.add('active');
  numEl.textContent = String(count);

  const tick = () => {
    count--;
    if (count <= 0) {
      overlay.classList.remove('active');
      cb();
      return;
    }
    // Re-trigger CSS animation
    numEl.style.animation = 'none';
    void numEl.offsetWidth;
    numEl.style.animation = '';
    numEl.textContent = String(count);
    setTimeout(tick, 920);
  };
  setTimeout(tick, 920);
}

function positionTeleprompter() {
  const tp = els.teleprompter;
  if (!tp) return;
  const frame = getSafeZoneRect();

  if (canvasFormat === '9:16' && frame.left > 50) {
    // Left pillarbox — outside the 9:16 safe zone, won't appear in cropped short
    tp.style.left = '0';
    tp.style.right = `calc(100% - ${frame.left - 6}px)`;
    tp.style.top = '50%';
    tp.style.bottom = '';
    tp.style.transform = 'translateY(-50%)';
    tp.style.width = '';
    tp.style.textAlign = 'right';
  } else if (canvasFormat === '1:1' && frame.top > 50) {
    // Bottom strip below the 1:1 frame
    tp.style.left = `${frame.left}px`;
    tp.style.right = `${frame.left}px`;
    tp.style.top = `${frame.top + frame.height + 8}px`;
    tp.style.bottom = '';
    tp.style.transform = '';
    tp.style.width = '';
    tp.style.textAlign = 'left';
  } else {
    // 16:9 — bottom strip above dock, semi-transparent background
    tp.style.left = '12px';
    tp.style.right = '12px';
    tp.style.bottom = '64px';
    tp.style.top = '';
    tp.style.transform = '';
    tp.style.width = '';
    tp.style.textAlign = 'left';
    tp.style.background = 'rgba(0,0,0,0.55)';
    tp.style.borderRadius = '8px';
  }
}

function updateTeleprompter() {
  if (!presentMode || !els.tpCurrent) return;
  const segs = state.timeline.transcriptSegments || [];
  const t = state.timeline.currentTime;
  let activeIdx = -1;
  let nextIdx = -1;
  for (let i = 0; i < segs.length; i++) {
    if (t >= segs[i].start && t < segs[i].end) { activeIdx = i; break; }
    if (segs[i].start > t && nextIdx === -1) nextIdx = i;
  }
  const coming = activeIdx >= 0 ? (segs[activeIdx + 1] || null) : (nextIdx >= 0 ? segs[nextIdx] : null);
  if (activeIdx >= 0) {
    els.tpCurrent.textContent = segs[activeIdx].text;
    els.tpNext.textContent = coming ? coming.text : '';
  } else if (nextIdx >= 0) {
    // Before any captions start — show upcoming as faint prompt
    els.tpCurrent.textContent = '▷ ' + segs[nextIdx].text;
    els.tpNext.textContent = segs[nextIdx + 1] ? segs[nextIdx + 1].text : '';
  } else {
    els.tpCurrent.textContent = '';
    els.tpNext.textContent = '';
  }
}

// ────────────────────────────────────────────────────────────────────────────

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
    const icon = (typeof tool.icon === 'string' && tool.icon.trim()) ? tool.icon.trim() : tool.label.slice(0, 1).toUpperCase();
    btn.innerHTML = `
      <span class="tool-row">
        <span class="tool-icon" aria-hidden="true">${icon}</span>
        <span class="tool-label">${tool.label}</span>
        <span class="tool-hotkey">${tool.hotkey}</span>
      </span>
    `;
    btn.setAttribute('aria-label', `${tool.label} tool (${tool.hotkey})`);
    btn.title = `${tool.label}  [${tool.hotkey}]`;
    btn.addEventListener('click', () => setTool(tool.id));
    els.toolGrid.appendChild(btn);
  }
}

function syncControls() {
  els.strokeColor.value = state.strokeColor;
  els.bgColor.value = state.bgColor;
  if (els.fillEnabled) els.fillEnabled.checked = state.fillEnabled;
  if (els.fillColor) els.fillColor.value = state.fillColor;
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
  updateTimelineButtons();
  updateTimelineTimeUI();
  updateTranscriptDisplayForTime(true);
  if (els.timelineZoom) {
    els.timelineZoom.value = String(clamp(Number(state.timeline.zoomPxPerSec) || 120, TIMELINE_MIN_PX_PER_SEC, TIMELINE_MAX_PX_PER_SEC));
  }
  setTimelineSnapSec(Number.isFinite(Number(state.timeline.snapSec)) ? state.timeline.snapSec : 0.25);
  syncCaptionStyleControls();
  syncSelectedShapeMotionControls();
  renderTimelineEditor({ keepScroll: true });
  setTool(state.tool);
  syncFormatWebcamControls();
  // Defer so canvas dimensions are known
  requestAnimationFrame(updateCaptionOverlayGeometry);
}

function syncFormatWebcamControls() {
  if (els.btnFormat16x9) els.btnFormat16x9.classList.toggle('active', canvasFormat === '16:9');
  if (els.btnFormat9x16) els.btnFormat9x16.classList.toggle('active', canvasFormat === '9:16');
  if (els.btnFormat1x1)  els.btnFormat1x1.classList.toggle('active', canvasFormat === '1:1');
  syncCaptionPlacementControls();
  if (els.captionZoneVisible) els.captionZoneVisible.checked = captionPlacement.zoneVisible;
  if (els.webcamPosition) els.webcamPosition.value = webcamState.position;
  if (els.webcamShape)    els.webcamShape.value = webcamState.shape;
  if (els.webcamSize) {
    els.webcamSize.value = String(webcamState.size);
    if (els.webcamSizeValue) els.webcamSizeValue.textContent = String(webcamState.size);
  }
}

function shapeNeedsAnimationFrame(shape, nowMs) {
  if (!shape || isShapeHidden(shape) || !shape.animated) {
    return false;
  }
  if (
    typeof shape.revealStartMs === 'number' &&
    typeof shape.revealDurationMs === 'number' &&
    shape.revealDurationMs > 0 &&
    nowMs - shape.revealStartMs < shape.revealDurationMs
  ) {
    return true;
  }
  return getDashPattern(shape).length > 0;
}

function canvasNeedsAnimationFrame(nowMs) {
  if (state.timeline.playing) {
    return true;
  }
  if (webcamState.enabled) {
    return true;
  }
  for (const shape of state.shapes) {
    if (shapeNeedsAnimationFrame(shape, nowMs)) {
      return true;
    }
  }
  if (shapeNeedsAnimationFrame(state.pointer.temp, nowMs)) {
    return true;
  }
  return false;
}

function runRenderLoop(nowMs) {
  renderLoopHandle = 0;
  stepTimeline(nowMs);
  const keepAnimating = canvasNeedsAnimationFrame(nowMs);
  if (keepAnimating) {
    state.dashTick += 1;
  }
  if (typeof renderNow === 'function') {
    renderNow();
  }
  if (keepAnimating) {
    renderLoopHandle = requestAnimationFrame(runRenderLoop);
  }
}

function ensureRenderLoop() {
  if (renderLoopHandle) {
    return;
  }
  if (!canvasNeedsAnimationFrame(performance.now())) {
    return;
  }
  renderLoopHandle = requestAnimationFrame(runRenderLoop);
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
  if (els.fillEnabled) {
    els.fillEnabled.addEventListener('change', () => {
      state.fillEnabled = els.fillEnabled.checked;
      saveToLocalStorage();
    });
  }
  if (els.fillColor) {
    els.fillColor.addEventListener('input', () => {
      state.fillColor = els.fillColor.value;
      saveToLocalStorage();
    });
  }
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
  if (els.btnPresent) els.btnPresent.addEventListener('click', togglePresentMode);
  els.btnFullscreen.addEventListener('click', toggleFullscreen);
  els.focusToggleBtn.addEventListener('click', () => toggleFocusMode(false));
  els.menuBtn.addEventListener('click', () => {
    document.body.classList.toggle('menu-open');
    requestAnimationFrame(resizeCanvas);
  });

  els.btnSaveJson.addEventListener('click', exportJson);
  els.btnLoadJson.addEventListener('click', () => els.fileLoader.click());
  els.fileLoader.addEventListener('change', () => {
    importJson(els.fileLoader.files[0]);
    els.fileLoader.value = '';
  });
  els.btnExportDsl.addEventListener('click', exportDslStarter);
  els.btnExportPng.addEventListener('click', exportPng);
  if (els.btnExportSRT) {
    els.btnExportSRT.addEventListener('click', exportCaptionsAsSRT);
  }

  if (els.canvasTextEditor) {
    els.canvasTextEditor.addEventListener('blur', commitCanvasTextEdit);
    els.canvasTextEditor.addEventListener('keydown', (event) => {
      if (event.key === 'Escape') {
        event.preventDefault();
        cancelCanvasTextEdit();
      } else if (event.key === 'Enter' && !event.shiftKey) {
        event.preventDefault();
        commitCanvasTextEdit();
      }
    });
  }

  if (els.btnTimelinePlay) {
    els.btnTimelinePlay.addEventListener('click', () => {
      toggleTimelinePlaying();
      setStatus(state.timeline.playing ? 'Timeline playback started.' : 'Timeline playback paused.');
    });
  }
  if (els.btnTimelineDockPlay) {
    els.btnTimelineDockPlay.addEventListener('click', () => {
      toggleTimelinePlaying();
      setStatus(state.timeline.playing ? 'Timeline playback started.' : 'Timeline playback paused.');
    });
  }
  if (els.btnTimelineStop) {
    els.btnTimelineStop.addEventListener('click', () => {
      setTimelinePlaying(false);
      setTimelineTime(0, { updateAudio: true });
      setStatus('Timeline stopped.');
    });
  }
  if (els.btnTimelineDockStop) {
    els.btnTimelineDockStop.addEventListener('click', () => {
      setTimelinePlaying(false);
      setTimelineTime(0, { updateAudio: true });
      setStatus('Timeline stopped.');
    });
  }
  if (els.btnTimelineInsertKeyframe) {
    els.btnTimelineInsertKeyframe.addEventListener('click', () => {
      insertMotionKeyframeAtTimeline(state.timeline.currentTime, { fromControls: false });
    });
  }
  if (els.btnTimelineLoadTranscript && els.timelineTranscriptLoader) {
    els.btnTimelineLoadTranscript.addEventListener('click', () => {
      els.timelineTranscriptLoader.click();
    });
    els.timelineTranscriptLoader.addEventListener('change', () => {
      const file = els.timelineTranscriptLoader.files && els.timelineTranscriptLoader.files[0];
      loadTranscriptFromFile(file || null);
      els.timelineTranscriptLoader.value = '';
    });
  }
  if (els.btnTimelineLoadAudio && els.timelineAudioLoader) {
    els.btnTimelineLoadAudio.addEventListener('click', () => {
      els.timelineAudioLoader.click();
    });
    els.timelineAudioLoader.addEventListener('change', () => {
      const file = els.timelineAudioLoader.files && els.timelineAudioLoader.files[0];
      if (file) {
        loadTimelineAudio(file);
      }
      els.timelineAudioLoader.value = '';
    });
  }
  if (els.btnTimelineMicRecord) {
    els.btnTimelineMicRecord.addEventListener('click', toggleTimelineMicRecording);
  }
  if (els.btnTimelineAddCaption) {
    els.btnTimelineAddCaption.addEventListener('click', addCaptionShapeAtTimeline);
  }
  if (els.timelineScrub) {
    els.timelineScrub.addEventListener('input', () => {
      scrubTimelineFromSliderValue(els.timelineScrub.value);
      render();
    });
  }
  if (els.timelineDockScrub) {
    els.timelineDockScrub.addEventListener('input', () => {
      scrubTimelineFromSliderValue(els.timelineDockScrub.value);
      render();
    });
  }
  if (els.timelineZoom) {
    els.timelineZoom.addEventListener('input', () => {
      setTimelineZoomPxPerSec(els.timelineZoom.value, { skipRender: false });
      saveToLocalStorage();
    });
  }
  if (els.timelineSnap) {
    els.timelineSnap.addEventListener('change', () => {
      setTimelineSnapSec(els.timelineSnap.value);
      saveToLocalStorage();
      setStatus(`Timeline snap ${timelineSnapSec() ? `set to ${timelineSnapSec().toFixed(2)}s` : 'disabled'}.`);
    });
  }
  if (els.btnTimelineSplit) {
    els.btnTimelineSplit.addEventListener('click', splitSelectedTimelineClipAtPlayhead);
  }
  if (els.btnTimelineDeleteClip) {
    els.btnTimelineDeleteClip.addEventListener('click', deleteSelectedTimelineClips);
  }
  if (els.timelineTrackCaptionBody) {
    els.timelineTrackCaptionBody.addEventListener('pointerdown', onTimelineCaptionPointerDown);
  }
  if (els.timelineTrackAudioBody) {
    els.timelineTrackAudioBody.addEventListener('pointerdown', (event) => {
      beginTimelineScrubDrag(event, els.timelineTrackAudioBody);
    });
  }
  if (els.timelineTrackShapeBody) {
    els.timelineTrackShapeBody.addEventListener('pointerdown', onTimelineShapePointerDown);
    els.timelineTrackShapeBody.addEventListener('contextmenu', onTimelineShapeContextMenu);
  }
  if (els.timelineRulerBody) {
    els.timelineRulerBody.addEventListener('pointerdown', (event) => {
      beginTimelineScrubDrag(event, els.timelineRulerBody);
    });
  }
  if (els.captionTemplateList) {
    els.captionTemplateList.addEventListener('click', (event) => {
      const button = event.target && event.target.closest
        ? event.target.closest('button[data-caption-template]')
        : null;
      if (!button || !els.captionTemplateList.contains(button)) {
        return;
      }
      const templateId = button.dataset.captionTemplate || '';
      if (!templateId) {
        return;
      }
      setCaptionTemplate(templateId, { syncUi: true, save: true });
      const templateLabel = button.querySelector('.name')
        ? button.querySelector('.name').textContent.trim()
        : button.textContent.trim();
      setStatus(`Caption template: ${templateLabel}.`);
    });
  }
  if (els.btnCaptionApplySelected) {
    els.btnCaptionApplySelected.addEventListener('click', applyCaptionStyleToSelectedTimelineSegments);
  }
  if (els.btnCaptionApplyAll) {
    els.btnCaptionApplyAll.addEventListener('click', applyCaptionStyleToAllTimelineSegments);
  }

  // ── Caption Styles Float Panel ──
  if (els.btnOpenCaptionStyles) {
    els.btnOpenCaptionStyles.addEventListener('click', openCaptionFloatPanel);
  }
  if (els.btnCaptionPanelClose) {
    els.btnCaptionPanelClose.addEventListener('click', closeCaptionFloatPanel);
  }
  if (els.cfpBackdrop) {
    els.cfpBackdrop.addEventListener('click', closeCaptionFloatPanel);
  }
  if (els.btnCfpSelected) {
    els.btnCfpSelected.addEventListener('click', () => {
      cfpApplyMode = 'selected';
      els.btnCfpSelected.classList.add('active');
      els.btnCfpAll.classList.remove('active');
    });
  }
  if (els.btnCfpAll) {
    els.btnCfpAll.addEventListener('click', () => {
      cfpApplyMode = 'all';
      els.btnCfpAll.classList.add('active');
      els.btnCfpSelected.classList.remove('active');
    });
  }
  const captionCustomInputs = [
    els.captionTextColor,
    els.captionStrokeColor,
    els.captionHighlightColor,
    els.captionHighlightTextColor,
    els.captionHighlightFontFamily,
    els.captionHighlightFontWeight,
    els.captionHighlightPadY,
    els.captionHighlightPadX,
    els.captionHighlightRadius,
    els.captionOverlayBg,
    els.captionFontSize,
    els.captionStrokeWidth,
    els.captionLineHeight,
    els.captionLetterSpacing,
    els.captionOverlayOpacity,
    els.captionFontFamily,
    els.captionActionMode,
    els.captionFontWeight,
    els.captionWordHighlight
  ];
  for (const input of captionCustomInputs) {
    if (!input) {
      continue;
    }
    const ev = input.type === 'checkbox' || input.tagName === 'SELECT' ? 'change' : 'input';
    input.addEventListener(ev, () => {
      updateCaptionStyleFromCustomControls();
    });
  }

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
  if (els.btnLayerUp) {
    els.btnLayerUp.addEventListener('click', bringForward);
  }
  if (els.btnLayerDown) {
    els.btnLayerDown.addEventListener('click', sendBackward);
  }
  if (els.btnGroupSelection) {
    els.btnGroupSelection.addEventListener('click', groupSelection);
  }
  if (els.btnUngroupSelection) {
    els.btnUngroupSelection.addEventListener('click', ungroupSelection);
  }
  if (els.btnHideSelection) {
    els.btnHideSelection.addEventListener('click', () => setSelectedVisibility(true));
  }
  if (els.btnShowSelection) {
    els.btnShowSelection.addEventListener('click', () => setSelectedVisibility(false));
  }
  if (els.btnLockSelection) {
    els.btnLockSelection.addEventListener('click', () => setSelectedLock(true));
  }
  if (els.btnUnlockSelection) {
    els.btnUnlockSelection.addEventListener('click', () => setSelectedLock(false));
  }
  if (els.layerList) {
    els.layerList.addEventListener('click', (event) => {
      const button = event.target.closest('button.layer-shape');
      if (!button || !els.layerList.contains(button)) {
        return;
      }
      const id = button.dataset.shapeId;
      if (!id) {
        return;
      }
      const shape = state.shapes.find((item) => item.id === id);
      if (!shape) {
        return;
      }
      const singleOnly = !!event.ctrlKey || !!event.metaKey;
      const targetIds = (!singleOnly && shape.groupId) ? selectionIdsForShape(shape) : [id];
      const current = selectedShapeIds();
      if (event.shiftKey) {
        const allAlreadySelected = targetIds.every((item) => current.includes(item));
        if (allAlreadySelected) {
          const removeSet = new Set(targetIds);
          setSelection(current.filter((item) => !removeSet.has(item)));
        } else {
          const next = current.slice();
          for (const item of targetIds) {
            if (!next.includes(item)) {
              next.push(item);
            }
          }
          setSelection(next);
        }
      } else if (event.altKey) {
        const removeSet = new Set(targetIds);
        setSelection(current.filter((item) => !removeSet.has(item)));
      } else {
        setSelection(targetIds);
      }
      render();
      const count = selectedShapeIds().length;
      setStatus(count ? `Selected ${count} layer(s).` : 'Selection cleared.');
    });
  }
  if (els.btnAddRect) els.btnAddRect.addEventListener('click', () => quickAddShape('rect'));
  if (els.btnAddEllipse) els.btnAddEllipse.addEventListener('click', () => quickAddShape('ellipse'));
  if (els.btnAddLine) els.btnAddLine.addEventListener('click', () => quickAddShape('line'));
  if (els.btnAddArrow) els.btnAddArrow.addEventListener('click', () => quickAddShape('arrow'));
  if (els.btnAddText) els.btnAddText.addEventListener('click', () => quickAddShape('text'));
  if (els.btnAddIcon) els.btnAddIcon.addEventListener('click', () => quickAddShape('icon'));

  // Selected-shape quick style edits
  els.btnLineSolid.addEventListener('click', () => setSelectedLineStyle('solid'));
  els.btnLineDashed.addEventListener('click', () => setSelectedLineStyle('dashed'));
  els.btnLineDotted.addEventListener('click', () => setSelectedLineStyle('dotted'));
  els.btnToggleArrow.addEventListener('click', toggleLineArrowSelection);
  els.btnRectRadius.addEventListener('click', editSelectedRectRadius);
  if (els.btnMotionApply) {
    els.btnMotionApply.addEventListener('click', applyMotionToSelectedShapes);
  }
  if (els.btnMotionInsertKeyframe) {
    els.btnMotionInsertKeyframe.addEventListener('click', () => {
      insertMotionKeyframeAtTimeline(state.timeline.currentTime, { fromControls: false });
    });
  }
  if (els.btnMotionSetKeyframe) {
    els.btnMotionSetKeyframe.addEventListener('click', () => {
      insertMotionKeyframeAtTimeline(state.timeline.currentTime, { fromControls: true });
    });
  }
  if (els.btnMotionDeleteKeyframe) {
    els.btnMotionDeleteKeyframe.addEventListener('click', () => {
      deleteMotionKeyframeAtTimeline(state.timeline.currentTime);
    });
  }
  if (els.btnMotionClear) {
    els.btnMotionClear.addEventListener('click', clearMotionFromSelectedShapes);
  }

  // Motion presets
  const motionPresetIds = ['btnPresetFadeIn', 'btnPresetFlyLeft', 'btnPresetFlyRight', 'btnPresetFlyUp', 'btnPresetFlyDown', 'btnPresetZoomIn', 'btnPresetFadeOut', 'btnPresetFlyOutLeft', 'btnPresetFlyOutRight'];
  for (const elId of motionPresetIds) {
    const presetBtn = document.getElementById(elId);
    if (presetBtn) {
      const presetKey = elId.replace('btnPreset', '').replace(/([A-Z])/g, (m) => `_${m.toLowerCase()}`).replace(/^_/, '');
      presetBtn.addEventListener('click', () => applyMotionPreset(presetKey));
    }
  }

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
    setStatus(state.snapGrid ? 'Snap enabled (grid + shape guides).' : 'Snap disabled.');
  });

  // Shortcut overlay
  els.shortcutOverlay.addEventListener('click', (e) => {
    if (e.target === els.shortcutOverlay) toggleShortcutOverlay(false);
  });

  els.stage.addEventListener('pointerdown', (event) => {
    event.preventDefault();
    clearTimelineClipSelection({ skipRender: true });
    renderTimelineEditor({ keepScroll: true });
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
    if (!hit) {
      return;
    }
    setSelection([hit.id]);
    if (hit.type === 'text') {
      beginCanvasTextEdit(hit);
    } else if (hit.type === 'rect') {
      editRectRadiusForShape(hit);
    }
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
  window.addEventListener('scroll', invalidateStageRectCache, true);
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
    if ((event.ctrlKey || event.metaKey) && event.shiftKey && event.key.toLowerCase() === 'g') {
      event.preventDefault();
      ungroupSelection();
      return;
    }
    if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'g') {
      event.preventDefault();
      groupSelection();
      return;
    }
    // ── Global guard: suppress ALL bare-key shortcuts when typing in any input / textarea / select.
    // Ctrl/Meta combos (handled above) and Escape always pass through.
    if (!event.ctrlKey && !event.metaKey && event.key !== 'Escape' && isFormControlTarget(event.target)) {
      return;
    }

    if (event.key.toLowerCase() === 'k') {
      event.preventDefault();
      toggleTimelinePlaying();
      return;
    }

    if (event.key.toLowerCase() === 'x') {
      if (selectedTimelineSegmentIds().length) {
        event.preventDefault();
        splitSelectedTimelineClipAtPlayhead();
        return;
      }
    }

    if (event.key.toLowerCase() === 'i') {
      event.preventDefault();
      if (event.shiftKey) {
        deleteMotionKeyframeAtTimeline(state.timeline.currentTime);
      } else {
        insertMotionKeyframeAtTimeline(state.timeline.currentTime, { fromControls: false });
      }
      return;
    }

    if (event.key === '[') {
      event.preventDefault();
      stepToAdjacentMotionKeyframe(-1);
      return;
    }

    if (event.key === ']') {
      event.preventDefault();
      stepToAdjacentMotionKeyframe(1);
      return;
    }

    const isDeleteKey = event.key === 'Delete' || event.key === 'Backspace' || event.key === 'Del' || event.code === 'Delete';
    if (isDeleteKey) {
      event.preventDefault();
      const timelineSelected = selectedTimelineSegmentIds().length;
      const shapeSelected = selectedShapeIds().length;
      if (timelineSelected && !shapeSelected) {
        deleteSelectedTimelineClips();
      } else {
        deleteSelected();
      }
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
      event.preventDefault();
      toggleFocusMode();
      return;
    }

    if (event.key.toLowerCase() === 'h') {
      event.preventDefault();
      if (window.matchMedia('(max-width: 1024px)').matches) {
        document.body.classList.toggle('menu-open');
      } else {
        toggleFocusMode();
      }
      return;
    }

    // Close caption float panel first on Escape
    if (event.key === 'Escape' && els.captionFloatPanel && els.captionFloatPanel.classList.contains('open')) {
      closeCaptionFloatPanel();
      return;
    }

    // Present mode shortcuts
    if (presentMode && event.code === 'Escape') {
      exitPresentMode();
      return;
    }
    if (presentMode && event.code === 'Space') {
      event.preventDefault();
      if (state.timeline.playing) {
        setTimelinePlaying(false);
      } else {
        startPresentCountdown(() => setTimelinePlaying(true));
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
      event.preventDefault();
      state.snapGrid = !state.snapGrid;
      els.btnSnap.textContent = state.snapGrid ? 'Snap: On' : 'Snap: Off';
      saveToLocalStorage();
      render();
      setStatus(state.snapGrid ? 'Snap enabled (grid + shape guides).' : 'Snap disabled.');
      return;
    }

    if (event.key.toLowerCase() === 'l') {
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
  window.addEventListener('beforeunload', () => {
    releaseTimelineDrag();
    if (timelineRuntime.micRecorder && timelineRuntime.micRecorder.state !== 'inactive') {
      timelineRuntime.micRecorder.stop();
    }
    stopTimelineMicCaptureTracks();
    teardownTimelineAudio();
    flushAutosaveOnExit();
  });

  // ── Script Editor ──
  if (els.scriptEditorText) {
    els.scriptEditorText.addEventListener('input', scriptEditorStat);
  }
  if (els.scriptSplitMode) {
    els.scriptSplitMode.addEventListener('change', () => {
      updateScriptModeUI();
      scriptEditorStat();
    });
  }
  if (els.scriptWordsPerCap) {
    els.scriptWordsPerCap.addEventListener('input', scriptEditorStat);
  }
  if (els.scriptWPM) {
    els.scriptWPM.addEventListener('input', () => {
      if (els.scriptWpmValue) els.scriptWpmValue.textContent = els.scriptWPM.value;
      scriptEditorStat();
    });
  }
  if (els.btnScriptGenerate) {
    els.btnScriptGenerate.addEventListener('click', () => generateCaptionsFromScript(false));
  }
  if (els.btnScriptAppend) {
    els.btnScriptAppend.addEventListener('click', () => generateCaptionsFromScript(true));
  }
  if (els.btnScriptClear) {
    els.btnScriptClear.addEventListener('click', () => {
      if (els.scriptEditorText) {
        els.scriptEditorText.value = '';
        scriptEditorStat();
      }
    });
  }
  if (els.btnScriptFromTranscript) {
    els.btnScriptFromTranscript.addEventListener('click', pullScriptFromTranscript);
  }

  // ── Format / Shorts ──
  if (els.btnFormat16x9) {
    els.btnFormat16x9.addEventListener('click', () => setCanvasFormat('16:9'));
  }
  if (els.btnFormat9x16) {
    els.btnFormat9x16.addEventListener('click', () => setCanvasFormat('9:16'));
  }
  if (els.btnFormat1x1) {
    els.btnFormat1x1.addEventListener('click', () => setCanvasFormat('1:1'));
  }

  // ── Caption Placement ──
  if (els.btnCaptionPosTop) {
    els.btnCaptionPosTop.addEventListener('click', () => setCaptionPlacementVertical('top'));
  }
  if (els.btnCaptionPosCenter) {
    els.btnCaptionPosCenter.addEventListener('click', () => setCaptionPlacementVertical('center'));
  }
  if (els.btnCaptionPosBottom) {
    els.btnCaptionPosBottom.addEventListener('click', () => setCaptionPlacementVertical('bottom'));
  }
  if (els.captionOffsetPct) {
    els.captionOffsetPct.addEventListener('input', () => {
      captionPlacement.offsetPct = Number(els.captionOffsetPct.value);
      if (els.captionOffsetPctValue) els.captionOffsetPctValue.textContent = els.captionOffsetPct.value;
      updateCaptionOverlayGeometry();
      saveToLocalStorage();
    });
  }
  if (els.captionZoneVisible) {
    els.captionZoneVisible.addEventListener('change', () => {
      captionPlacement.zoneVisible = els.captionZoneVisible.checked;
      updateCaptionOverlayGeometry();
      saveToLocalStorage();
    });
  }
  setupCaptionZoneDrag();
  setupCaptionOverlayDrag();

  // ── Webcam PiP ──
  if (els.btnWebcamToggle) {
    els.btnWebcamToggle.addEventListener('click', () => {
      if (webcamState.enabled) {
        stopWebcam();
        render();
      } else {
        startWebcam();
      }
    });
  }
  if (els.webcamPosition) {
    els.webcamPosition.addEventListener('change', () => {
      webcamState.position = els.webcamPosition.value;
      saveToLocalStorage();
    });
  }
  if (els.webcamShape) {
    els.webcamShape.addEventListener('change', () => {
      webcamState.shape = els.webcamShape.value;
      saveToLocalStorage();
    });
  }
  if (els.webcamSize) {
    els.webcamSize.addEventListener('input', () => {
      webcamState.size = Number(els.webcamSize.value);
      if (els.webcamSizeValue) els.webcamSizeValue.textContent = els.webcamSize.value;
      saveToLocalStorage();
    });
  }
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

function groupSelection() {
  const items = selectedUnlockedShapes();
  if (items.length < 2) {
    setStatus('Select at least two unlocked shapes to group.');
    return;
  }
  let targetGroupId = null;
  const existing = new Set(items.map((shape) => shape.groupId).filter(Boolean));
  if (existing.size === 1) {
    targetGroupId = items[0].groupId;
  } else {
    targetGroupId = `grp-${uid()}`;
  }
  for (const shape of items) {
    shape.groupId = targetGroupId;
  }
  setSelection(items.map((shape) => shape.id));
  pushHistory();
  render();
  setStatus(`Grouped ${items.length} shape(s).`);
}

function ungroupSelection() {
  const items = selectedUnlockedShapes().filter((shape) => !!shape.groupId);
  if (!items.length) {
    setStatus('Select grouped, unlocked shapes to ungroup.');
    return;
  }
  for (const shape of items) {
    delete shape.groupId;
  }
  setSelection(items.map((shape) => shape.id));
  pushHistory();
  render();
  setStatus(`Ungrouped ${items.length} shape(s).`);
}

function setSelectedVisibility(hidden) {
  const items = selectedShapes();
  if (!items.length) {
    setStatus('Select at least one shape first.');
    return;
  }
  for (const shape of items) {
    shape.hidden = !!hidden;
  }
  pushHistory();
  render();
  setStatus(hidden ? `Hid ${items.length} shape(s).` : `Showed ${items.length} shape(s).`);
}

function setSelectedLock(locked) {
  const items = selectedShapes();
  if (!items.length) {
    setStatus('Select at least one shape first.');
    return;
  }
  for (const shape of items) {
    shape.locked = !!locked;
  }
  pushHistory();
  render();
  setStatus(locked ? `Locked ${items.length} shape(s).` : `Unlocked ${items.length} shape(s).`);
}

// Selected-shape motion supports multi-keyframe offsets/opacity/trim data.
function sameShapeMotion(a, b) {
  if (!a || !b) {
    return false;
  }
  const ka = Array.isArray(a.keyframes) ? a.keyframes : [];
  const kb = Array.isArray(b.keyframes) ? b.keyframes : [];
  if (ka.length !== kb.length) {
    return false;
  }
  for (let i = 0; i < ka.length; i += 1) {
    const left = ka[i];
    const right = kb[i];
    if (!left || !right) {
      return false;
    }
    if (Math.abs(left.at - right.at) > 1e-6 ||
      Math.abs(left.dx - right.dx) > 1e-6 ||
      Math.abs(left.dy - right.dy) > 1e-6 ||
      Math.abs(left.opacity - right.opacity) > 1e-6 ||
      Math.abs((left.trimStart ?? 0) - (right.trimStart ?? 0)) > 1e-6 ||
      Math.abs((left.trimEnd ?? 1) - (right.trimEnd ?? 1)) > 1e-6 ||
      normalizeMotionEase(left.ease, a.ease) !== normalizeMotionEase(right.ease, b.ease)) {
      return false;
    }
  }
  return a.enabled === b.enabled &&
    Math.abs(a.start - b.start) < 1e-6 &&
    Math.abs(a.duration - b.duration) < 1e-6 &&
    Math.abs(a.fromX - b.fromX) < 1e-6 &&
    Math.abs(a.fromY - b.fromY) < 1e-6 &&
    Math.abs(a.toX - b.toX) < 1e-6 &&
    Math.abs(a.toY - b.toY) < 1e-6 &&
    Math.abs(a.fromOpacity - b.fromOpacity) < 1e-6 &&
    Math.abs(a.toOpacity - b.toOpacity) < 1e-6 &&
    Math.abs((a.trimStart ?? 0) - (b.trimStart ?? 0)) < 1e-6 &&
    Math.abs((a.trimEnd ?? 1) - (b.trimEnd ?? 1)) < 1e-6 &&
    a.ease === b.ease;
}

function readMotionControls() {
  const trim = normalizeTrimWindow(
    (Number(els.motionTrimStart && els.motionTrimStart.value) || 0) / 100,
    (Number(els.motionTrimEnd && els.motionTrimEnd.value) || 100) / 100
  );
  const start = els.motionStart ? Number(els.motionStart.value) : state.timeline.currentTime;
  const duration = els.motionDuration ? Number(els.motionDuration.value) : 1.2;
  const fromX = els.motionFromX ? Number(els.motionFromX.value) : 0;
  const fromY = els.motionFromY ? Number(els.motionFromY.value) : 0;
  const toX = els.motionToX ? Number(els.motionToX.value) : 140;
  const toY = els.motionToY ? Number(els.motionToY.value) : 0;
  const fromOpacity = els.motionFromOpacity ? Number(els.motionFromOpacity.value) : 1;
  const toOpacity = els.motionToOpacity ? Number(els.motionToOpacity.value) : 1;
  const ease = els.motionEase ? els.motionEase.value : 'easeInOutCubic';
  return normalizeShapeMotion({
    enabled: true,
    start,
    duration,
    fromX,
    fromY,
    toX,
    toY,
    fromOpacity,
    toOpacity,
    trimStart: trim.trimStart,
    trimEnd: trim.trimEnd,
    ease,
    keyframes: [
      { at: start, dx: fromX, dy: fromY, opacity: fromOpacity, trimStart: trim.trimStart, trimEnd: trim.trimEnd, ease },
      { at: start + Math.max(0.05, Number(duration) || 1.2), dx: toX, dy: toY, opacity: toOpacity, trimStart: trim.trimStart, trimEnd: trim.trimEnd, ease }
    ]
  }, {
    start: state.timeline.currentTime,
    duration: 1.2,
    fromX: 0,
    fromY: 0,
    toX: 140,
    toY: 0,
    fromOpacity: 1,
    toOpacity: 1,
    trimStart: 0,
    trimEnd: 1,
    ease: 'easeInOutCubic'
  });
}

function syncSelectedShapeMotionControls() {
  const selected = selectedUnlockedShapes();
  const source = selected.find((shape) => !!(shape.motion && shape.motion.enabled)) || selected[0] || null;
  const motion = source
    ? normalizeShapeMotion(source.motion, { start: state.timeline.currentTime })
    : normalizeShapeMotion({
      start: state.timeline.currentTime,
      duration: 1.2,
      fromX: 0,
      fromY: 0,
      toX: 140,
      toY: 0,
      fromOpacity: 1,
      toOpacity: 1,
      trimStart: 0,
      trimEnd: 1,
      ease: 'easeInOutCubic'
    });
  const keyframes = motion.keyframes || [];
  const focusIndex = keyframes.length
    ? Math.max(0, findMotionKeyframeIndexNearTime(keyframes, state.timeline.currentTime, Number.POSITIVE_INFINITY))
    : -1;
  const focus = focusIndex >= 0
    ? keyframes[focusIndex]
    : {
      dx: motion.fromX,
      dy: motion.fromY,
      opacity: motion.fromOpacity,
      trimStart: motion.trimStart ?? 0,
      trimEnd: motion.trimEnd ?? 1
    };
  const first = keyframes.length ? keyframes[0] : motion;
  const last = keyframes.length ? keyframes[keyframes.length - 1] : motion;
  if (els.motionStart) {
    els.motionStart.value = String(roundTimelineSeconds(first.at ?? motion.start));
  }
  if (els.motionDuration) {
    const duration = keyframes.length
      ? Math.max(0.05, (last.at ?? motion.start) - (first.at ?? motion.start))
      : motion.duration;
    els.motionDuration.value = String(roundTimelineSeconds(duration));
  }
  if (els.motionFromX) {
    els.motionFromX.value = String(Math.round(focus.dx ?? motion.fromX));
  }
  if (els.motionFromY) {
    els.motionFromY.value = String(Math.round(focus.dy ?? motion.fromY));
  }
  if (els.motionToX) {
    els.motionToX.value = String(Math.round(last.dx ?? motion.toX));
  }
  if (els.motionToY) {
    els.motionToY.value = String(Math.round(last.dy ?? motion.toY));
  }
  if (els.motionFromOpacity) {
    els.motionFromOpacity.value = String(Math.round((focus.opacity ?? motion.fromOpacity) * 100) / 100);
  }
  if (els.motionToOpacity) {
    els.motionToOpacity.value = String(Math.round((last.opacity ?? motion.toOpacity) * 100) / 100);
  }
  if (els.motionTrimStart) {
    els.motionTrimStart.value = String(Math.round(clamp((focus.trimStart ?? motion.trimStart ?? 0) * 100, 0, 100)));
  }
  if (els.motionTrimEnd) {
    els.motionTrimEnd.value = String(Math.round(clamp((focus.trimEnd ?? motion.trimEnd ?? 1) * 100, 0, 100)));
  }
  if (els.motionEase) {
    els.motionEase.value = motion.ease;
  }

  const hasUnlocked = selected.length > 0;
  const hasMotion = selected.some((shape) => !!(shape.motion && shape.motion.enabled));
  const hasKeyframeNearPlayhead = selected.some((shape) => {
    if (!shape || !shape.motion || !shape.motion.enabled) {
      return false;
    }
    const motion = normalizeShapeMotion(shape.motion, { start: state.timeline.currentTime, duration: 0.2 });
    return findMotionKeyframeIndexNearTime(motion.keyframes, state.timeline.currentTime) >= 0;
  });
  if (els.btnMotionApply) {
    els.btnMotionApply.disabled = !hasUnlocked;
  }
  if (els.btnMotionInsertKeyframe) {
    els.btnMotionInsertKeyframe.disabled = !hasUnlocked;
  }
  if (els.btnMotionSetKeyframe) {
    els.btnMotionSetKeyframe.disabled = !hasUnlocked;
  }
  if (els.btnMotionDeleteKeyframe) {
    els.btnMotionDeleteKeyframe.disabled = !hasKeyframeNearPlayhead;
  }
  if (els.btnMotionClear) {
    els.btnMotionClear.disabled = !hasMotion;
  }
  if (els.btnTimelineInsertKeyframe) {
    els.btnTimelineInsertKeyframe.disabled = !hasUnlocked;
  }
}

function applyMotionToSelectedShapes() {
  const targets = selectedUnlockedShapes();
  if (!targets.length) {
    setStatus('Select at least one unlocked shape first.');
    syncSelectedShapeMotionControls();
    return;
  }
  const nextMotion = readMotionControls();
  let changed = 0;
  for (const shape of targets) {
    const prev = shape.motion ? normalizeShapeMotion(shape.motion) : null;
    if (prev && sameShapeMotion(prev, nextMotion)) {
      continue;
    }
    shape.motion = JSON.parse(JSON.stringify(nextMotion));
    changed += 1;
  }
  if (!changed) {
    setStatus('Selected shapes already use that motion.');
    syncSelectedShapeMotionControls();
    return;
  }
  invalidateShapeCaches();
  markLayerPanelDirty();
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTimelineEditor({ keepScroll: true });
  pushHistory();
  render();
  syncSelectedShapeMotionControls();
  setStatus(`Applied motion to ${changed} shape(s).`);
}

function applyMotionPreset(presetKey) {
  const targets = selectedUnlockedShapes();
  if (!targets.length) {
    setStatus('Select at least one unlocked shape first.');
    return;
  }
  const t = state.timeline.currentTime;
  const dur = 0.45;
  const ease = 'easeOutCubic';
  const fly = 90;
  let kf0, kf1;
  switch (presetKey) {
    case 'fade_in':
      kf0 = { at: t, dx: 0, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_left':
      kf0 = { at: t, dx: -fly, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_right':
      kf0 = { at: t, dx: fly, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_up':
      kf0 = { at: t, dx: 0, dy: fly, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_down':
      kf0 = { at: t, dx: 0, dy: -fly, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'zoom_in':
      kf0 = { at: t, dx: 0, dy: 20, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fade_out':
      kf0 = { at: t, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: 0, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_out_left':
      kf0 = { at: t, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: -fly, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      break;
    case 'fly_out_right':
      kf0 = { at: t, dx: 0, dy: 0, opacity: 1, trimStart: 0, trimEnd: 1, ease };
      kf1 = { at: t + dur, dx: fly, dy: 0, opacity: 0, trimStart: 0, trimEnd: 1, ease };
      break;
    default:
      setStatus(`Unknown preset: ${presetKey}`);
      return;
  }
  for (const shape of targets) {
    shape.motion = normalizeShapeMotion({ enabled: true, keyframes: [kf0, kf1] });
  }
  invalidateShapeCaches();
  markLayerPanelDirty();
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTimelineEditor({ keepScroll: true });
  pushHistory();
  render();
  syncSelectedShapeMotionControls();
  setStatus(`Applied "${presetKey.replace(/_/g, ' ')}" preset to ${targets.length} shape(s).`);
}

function clearMotionFromSelectedShapes() {
  const targets = selectedUnlockedShapes();
  if (!targets.length) {
    setStatus('Select at least one unlocked shape first.');
    syncSelectedShapeMotionControls();
    return;
  }
  let cleared = 0;
  for (const shape of targets) {
    if (!shape.motion) {
      continue;
    }
    delete shape.motion;
    cleared += 1;
  }
  if (!cleared) {
    setStatus('Selected shapes already have no motion.');
    syncSelectedShapeMotionControls();
    return;
  }
  invalidateShapeCaches();
  markLayerPanelDirty();
  refreshTimelineDurationFromSources({ preserveExisting: false });
  renderTimelineEditor({ keepScroll: true });
  pushHistory();
  render();
  syncSelectedShapeMotionControls();
  setStatus(`Cleared motion on ${cleared} shape(s).`);
}

function setSelectedLineStyle(style) {
  const targets = selectedUnlockedShapes().filter((shape) => shape.type === 'line' || shape.type === 'arrow');
  if (!targets.length) {
    setStatus('Select at least one unlocked line/arrow first.');
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
  const targets = selectedUnlockedShapes().filter((shape) => shape.type === 'line' || shape.type === 'arrow');
  if (!targets.length) {
    setStatus('Select at least one unlocked line/arrow first.');
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
  const rect = selectedUnlockedShapes().find((shape) => shape.type === 'rect');
  editRectRadiusForShape(rect || null);
}

function beginCanvasTextEdit(shape) {
  if (!shape || shape.type !== 'text' || shape.locked) {
    return;
  }
  const editor = els.canvasTextEditor;
  if (!editor) {
    return;
  }
  const stageRect = getStageRect();
  const screen = toScreen(shape.x, shape.y);
  const fontSize = Math.max(12, shape.size || 28) * state.camera.scale;
  editor.dataset.shapeId = shape.id;
  editor.value = String(shape.text || '');
  editor.style.left = `${screen.x}px`;
  editor.style.top = `${screen.y - fontSize * 0.6}px`;
  editor.style.fontSize = `${fontSize}px`;
  editor.style.minWidth = `${Math.max(80, fontSize * 4)}px`;
  editor.style.display = 'block';
  editor.focus();
  editor.select();
}

function commitCanvasTextEdit() {
  const editor = els.canvasTextEditor;
  if (!editor || editor.style.display === 'none') {
    return;
  }
  const shapeId = editor.dataset.shapeId;
  const shape = shapeId ? state.shapes.find((s) => s.id === shapeId) : null;
  if (shape && shape.type === 'text') {
    const newText = editor.value.trim();
    if (newText !== shape.text) {
      shape.text = newText || 'Text';
      invalidateShapeCaches(shape.id);
      pushHistory();
      render();
    }
  }
  editor.style.display = 'none';
  editor.dataset.shapeId = '';
}

function cancelCanvasTextEdit() {
  const editor = els.canvasTextEditor;
  if (!editor) {
    return;
  }
  editor.style.display = 'none';
  editor.dataset.shapeId = '';
}

function alignSelected(mode) {
  const validModes = new Set(['left', 'center', 'right', 'top', 'middle', 'bottom']);
  if (!validModes.has(mode)) {
    setStatus('Unknown align mode.');
    return;
  }

  const items = selectedUnlockedShapes();
  if (items.length < 2) {
    setStatus('Select at least two unlocked shapes to align.');
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

  const items = selectedUnlockedShapes();
  if (items.length < 3) {
    setStatus('Select at least three unlocked shapes to distribute.');
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
  const originals = selectedUnlockedShapes();
  if (!originals.length) {
    setStatus('Nothing unlocked selected to duplicate.');
    return;
  }
  const selectedCloneIds = [];
  const groupCloneMap = new Map();
  const offset = 24 / state.camera.scale;
  for (const original of originals) {
    const clone = JSON.parse(JSON.stringify(original));
    clone.id = uid();
    if (clone.groupId) {
      if (!groupCloneMap.has(clone.groupId)) {
        groupCloneMap.set(clone.groupId, `grp-${uid()}`);
      }
      clone.groupId = groupCloneMap.get(clone.groupId);
    }
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
  const ids = selectedUnlockedShapes().map((shape) => shape.id);
  if (!ids.length) {
    setStatus('Select at least one unlocked shape first.');
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
  const ids = selectedUnlockedShapes().map((shape) => shape.id);
  if (!ids.length) {
    setStatus('Select at least one unlocked shape first.');
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
    ctx,
    isShapeSelectable,
    onSelectionChanged: () => {
      markLayerPanelDirty();
      syncSelectedShapeMotionControls();
      renderTimelineEditor({ keepScroll: true });
    }
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
    drawSnapGuides,
    drawShape,
    selectedShapeIds,
    drawMarqueeOverlay,
    drawEditHandlesOverlay,
    renderLayers: renderLayerPanel,
    shouldRenderLayers: shouldRenderLayerPanel
  });
  const baseRenderNow = renderController.render;
  renderNow = () => {
    baseRenderNow();
    if (webcamState.enabled) {
      drawWebcamPiP();
    }
  };
  render = () => {
    renderNow();
    ensureRenderLoop();
  };

  const historyController = createHistoryController({
    state,
    DEFAULT_BG,
    sanitizeArtboards,
    clearSelection,
    renderArtboardTabs,
    render,
    syncControls,
    refreshTimelineViews,
    saveToLocalStorage,
    setStatus,
    onHistoryStateChange: () => {
      invalidateShapeCaches();
      markLayerPanelDirty();
    }
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
  initButtonIconLabels();
  updateTimelineMicRecordButton();
  const restored = loadFromLocalStorage();
  initPanelToggles();
  if (!restored) {
    pushHistory();
    setStatus('New canvas ready.');
  }
  syncControls();
  initTimelineUI();
  setupBindings();
  updateScriptModeUI();
  scriptEditorStat();
  resizeCanvas();
  ensureRenderLoop();
}

init();
