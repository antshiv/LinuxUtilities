export function createHistoryController(deps) {
  const {
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
    onHistoryStateChange
  } = deps;

  function timelineSnapshot() {
    return {
      currentTime: state.timeline.currentTime,
      duration: state.timeline.duration,
      zoomPxPerSec: state.timeline.zoomPxPerSec,
      snapSec: state.timeline.snapSec,
      captionStyle: state.timeline.captionStyle,
      transcriptSegments: state.timeline.transcriptSegments,
      transcriptName: state.timeline.transcriptName,
      audioName: state.timeline.audioName
    };
  }

  function applyTimelineSnapshot(snapshot) {
    if (!snapshot || typeof snapshot !== 'object') {
      return;
    }
    state.timeline.currentTime = Math.max(0, Number(snapshot.currentTime) || 0);
    state.timeline.duration = Math.max(1, Number(snapshot.duration) || state.timeline.duration);
    state.timeline.zoomPxPerSec = Number(snapshot.zoomPxPerSec) || state.timeline.zoomPxPerSec;
    state.timeline.snapSec = Math.max(0, Number(snapshot.snapSec) || 0);
    state.timeline.captionStyle = snapshot.captionStyle && typeof snapshot.captionStyle === 'object'
      ? snapshot.captionStyle
      : state.timeline.captionStyle;
    state.timeline.transcriptSegments = Array.isArray(snapshot.transcriptSegments)
      ? snapshot.transcriptSegments
      : [];
    state.timeline.transcriptName = snapshot.transcriptName || '';
    state.timeline.audioName = snapshot.audioName || '';
    state.timeline.playing = false;
    state.timeline.lastTickMs = 0;
  }

  function pushHistory() {
    const snapshot = JSON.stringify({
      artboards: state.artboards,
      activeArtboardId: state.activeArtboardId,
      grid: state.grid,
      timeline: timelineSnapshot()
    });
    state.history = state.history.slice(0, state.historyIndex + 1);
    state.history.push(snapshot);
    if (state.history.length > 120) {
      state.history.shift();
    }
    state.historyIndex = state.history.length - 1;
    if (typeof onHistoryStateChange === 'function') {
      onHistoryStateChange();
    }
    renderArtboardTabs();
    saveToLocalStorage();
  }

  function restoreHistory(index) {
    const snap = state.history[index];
    if (!snap) {
      return;
    }
    try {
      const parsed = JSON.parse(snap);
      if (Array.isArray(parsed.artboards)) {
        state.artboards = sanitizeArtboards(parsed.artboards);
        state.activeArtboardId = parsed.activeArtboardId || state.artboards[0].id;
      } else {
        state.artboards = sanitizeArtboards(null, {
          name: 'Artboard 1',
          bgColor: parsed.bgColor || DEFAULT_BG,
          camera: parsed.camera || { x: 0, y: 0, scale: 1 },
          shapes: Array.isArray(parsed.shapes) ? parsed.shapes : []
        });
        state.activeArtboardId = state.artboards[0].id;
      }
      state.grid = parsed.grid !== false;
      applyTimelineSnapshot(parsed.timeline);
      clearSelection();
      if (typeof onHistoryStateChange === 'function') {
        onHistoryStateChange();
      }
      renderArtboardTabs();
      render();
      syncControls();
      if (typeof refreshTimelineViews === 'function') {
        refreshTimelineViews();
      }
      saveToLocalStorage();
    } catch (_err) {
      setStatus('Unable to restore history snapshot.');
    }
  }

  function undo() {
    if (state.historyIndex <= 0) {
      return;
    }
    state.historyIndex -= 1;
    restoreHistory(state.historyIndex);
    setStatus('Undo');
  }

  function redo() {
    if (state.historyIndex >= state.history.length - 1) {
      return;
    }
    state.historyIndex += 1;
    restoreHistory(state.historyIndex);
    setStatus('Redo');
  }

  return {
    pushHistory,
    undo,
    redo,
    restoreHistory
  };
}
