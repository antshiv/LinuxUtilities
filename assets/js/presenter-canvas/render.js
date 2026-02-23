export function createRenderController(deps) {
  const {
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
    renderLayers,
    shouldRenderLayers
  } = deps;

  function render() {
    const width = els.stage.width / DPR;
    const height = els.stage.height / DPR;

    ctx.save();
    ctx.setTransform(DPR, 0, 0, DPR, 0, 0);
    ctx.clearRect(0, 0, width, height);
    ctx.fillStyle = state.bgColor;
    ctx.fillRect(0, 0, width, height);

    ctx.translate(-state.camera.x * state.camera.scale, -state.camera.y * state.camera.scale);
    ctx.scale(state.camera.scale, state.camera.scale);

    drawGrid();

    const selected = new Set(selectedShapeIds());
    for (const shape of state.shapes) {
      drawShape(shape, selected.has(shape.id));
    }
    if (state.pointer.temp) {
      drawShape(state.pointer.temp, false);
    }
    if (typeof drawSnapGuides === 'function') {
      drawSnapGuides();
    }
    drawMarqueeOverlay();
    drawEditHandlesOverlay();

    ctx.restore();

    const activeTool = TOOLS.find((t) => t.id === state.tool);
    const board = currentArtboard();
    els.toolPill.textContent = `Tool: ${activeTool ? activeTool.label : state.tool}`;
    els.zoomPill.textContent = `Zoom: ${Math.round(state.camera.scale * 100)}%`;
    els.animPill.textContent = `Animation: ${state.animated ? 'On' : 'Off'}`;
    els.shapesPill.textContent = `${board.name}: ${state.shapes.length}`;
    els.undoPill.textContent = `Undo: ${state.historyIndex}/${state.history.length - 1}`;
    els.snapBadge.classList.toggle('on', state.snapGrid);
    if (typeof renderLayers === 'function' && (typeof shouldRenderLayers !== 'function' || shouldRenderLayers())) {
      renderLayers();
    }
  }

  return {
    render
  };
}
