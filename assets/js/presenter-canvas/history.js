export function createHistoryController(deps) {
  const {
    state,
    DEFAULT_BG,
    sanitizeArtboards,
    clearSelection,
    renderArtboardTabs,
    render,
    syncControls,
    saveToLocalStorage,
    setStatus
  } = deps;

  function pushHistory() {
    const snapshot = JSON.stringify({
      artboards: state.artboards,
      activeArtboardId: state.activeArtboardId,
      grid: state.grid
    });
    state.history = state.history.slice(0, state.historyIndex + 1);
    state.history.push(snapshot);
    if (state.history.length > 120) {
      state.history.shift();
    }
    state.historyIndex = state.history.length - 1;
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
      clearSelection();
      renderArtboardTabs();
      render();
      syncControls();
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
