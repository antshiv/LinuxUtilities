import test from 'node:test';
import assert from 'node:assert/strict';

import {
  uid,
  makeArtboard,
  sanitizeArtboards,
  createInitialState
} from '../assets/js/presenter-canvas/state.js';
import { createSelectionController } from '../assets/js/presenter-canvas/selection.js';
import { createHistoryController } from '../assets/js/presenter-canvas/history.js';
import { createPathfinderController } from '../assets/js/presenter-canvas/pathfinder.js';

test('uid returns stable unique ids', () => {
  const ids = new Set();
  for (let i = 0; i < 200; i += 1) {
    ids.add(uid());
  }
  assert.equal(ids.size, 200);
});

test('makeArtboard clones content and clamps camera scale', () => {
  const src = {
    name: 'Demo',
    bgColor: '#111111',
    camera: { x: 4, y: 9, scale: 99 },
    shapes: [{ id: 's1', type: 'rect', x1: 0, y1: 0, x2: 10, y2: 8 }]
  };
  const board = makeArtboard('Fallback', src);
  assert.equal(board.name, 'Demo');
  assert.equal(board.camera.scale, 8);
  assert.notEqual(board.shapes, src.shapes);
  board.shapes[0].x1 = 123;
  assert.equal(src.shapes[0].x1, 0);
});

test('selection controller filters unknown ids and emits change callbacks', () => {
  const state = {
    shapes: [{ id: 'a' }, { id: 'b' }],
    selectedId: null,
    selectedIds: [],
    camera: { scale: 1 },
    pointer: { marquee: null }
  };
  const calls = [];
  const selection = createSelectionController({
    state,
    getBounds: () => ({ x: 0, y: 0, w: 10, h: 10 }),
    setStatus: () => {},
    ctx: {},
    isShapeSelectable: () => true,
    onSelectionChanged: (ids) => calls.push(ids)
  });

  selection.setSelection(['a', 'missing', 'a', 'b']);
  assert.deepEqual(state.selectedIds, ['a', 'b']);
  assert.equal(state.selectedId, 'b');
  assert.deepEqual(calls.at(-1), ['a', 'b']);

  selection.clearSelection();
  assert.deepEqual(state.selectedIds, []);
  assert.equal(state.selectedId, null);
  assert.deepEqual(calls.at(-1), []);
});

test('history snapshots include timeline edits and restore on undo/redo', () => {
  const state = createInitialState();
  state.activeArtboardId = state.artboards[0].id;
  state.timeline.currentTime = 0.4;
  state.timeline.duration = 5;
  state.timeline.transcriptSegments = [{ id: 's0', start: 0, end: 1, text: 'alpha' }];

  let refreshed = 0;
  const history = createHistoryController({
    state,
    DEFAULT_BG: '#06080f',
    sanitizeArtboards,
    clearSelection: () => {},
    renderArtboardTabs: () => {},
    render: () => {},
    syncControls: () => {},
    refreshTimelineViews: () => { refreshed += 1; },
    saveToLocalStorage: () => {},
    setStatus: () => {},
    onHistoryStateChange: () => {}
  });

  history.pushHistory();

  state.timeline.currentTime = 1.25;
  state.timeline.duration = 12;
  state.timeline.transcriptSegments = [{ id: 's1', start: 1, end: 2, text: 'beta' }];
  history.pushHistory();

  state.timeline.currentTime = 2.4;
  state.timeline.duration = 16;
  state.timeline.transcriptSegments = [{ id: 's2', start: 2, end: 3, text: 'gamma' }];
  history.pushHistory();

  state.timeline.playing = true;
  history.undo();
  assert.equal(state.timeline.duration, 12);
  assert.equal(state.timeline.currentTime, 1.25);
  assert.deepEqual(state.timeline.transcriptSegments.map((seg) => seg.id), ['s1']);
  assert.equal(state.timeline.playing, false);

  history.redo();
  assert.equal(state.timeline.duration, 16);
  assert.deepEqual(state.timeline.transcriptSegments.map((seg) => seg.id), ['s2']);
  assert.ok(refreshed >= 2);
});

test('pathfinder union replaces two rectangles with one merged shape when overlap is horizontal', () => {
  const state = {
    shapes: [
      { id: 'a', type: 'rect', x1: 0, y1: 0, x2: 10, y2: 10, color: '#fff', width: 2, style: 'solid', animated: false, opacity: 1 },
      { id: 'b', type: 'rect', x1: 8, y1: 0, x2: 20, y2: 10, color: '#fff', width: 2, style: 'solid', animated: false, opacity: 1 }
    ],
    selectedId: 'b',
    strokeColor: '#fff',
    lineWidth: 2,
    lineStyle: 'solid'
  };
  let selected = [];
  let pushed = 0;
  let rendered = 0;
  let status = '';
  const pathfinder = createPathfinderController({
    state,
    uid: (() => {
      let n = 0;
      return () => `id-${++n}`;
    })(),
    selectedShapeIds: () => ['a', 'b'],
    setSelection: (ids) => { selected = ids; },
    pushHistory: () => { pushed += 1; },
    render: () => { rendered += 1; },
    setStatus: (msg) => { status = msg; }
  });

  pathfinder.applyPathfinder('union');

  assert.equal(state.shapes.length, 1);
  assert.equal(state.shapes[0].type, 'rect');
  assert.equal(state.shapes[0].x1, 0);
  assert.equal(state.shapes[0].x2, 20);
  assert.equal(selected.length, 1);
  assert.equal(pushed, 1);
  assert.equal(rendered, 1);
  assert.match(status, /Pathfinder union complete/i);
});
