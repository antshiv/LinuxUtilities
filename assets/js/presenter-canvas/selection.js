export function createSelectionController(deps) {
  const { state, getBounds, setStatus, ctx } = deps;

  function setSelection(ids) {
    const safeIds = [];
    for (const id of ids || []) {
      if (!id || safeIds.includes(id)) continue;
      if (state.shapes.some((s) => s.id === id)) {
        safeIds.push(id);
      }
    }
    state.selectedIds = safeIds;
    state.selectedId = safeIds.length ? safeIds[safeIds.length - 1] : null;
  }

  function clearSelection() {
    state.selectedId = null;
    state.selectedIds = [];
  }

  function selectedShapeIds() {
    if (state.selectedIds && state.selectedIds.length) {
      return state.selectedIds.slice();
    }
    return state.selectedId ? [state.selectedId] : [];
  }

  function selectedShapes() {
    const ids = selectedShapeIds();
    return ids.map((id) => state.shapes.find((s) => s.id === id)).filter(Boolean);
  }

  function boxFromPoints(a, b) {
    if (!a || !b) {
      return null;
    }
    const x = Math.min(a.x, b.x);
    const y = Math.min(a.y, b.y);
    return { x, y, w: Math.abs(b.x - a.x), h: Math.abs(b.y - a.y) };
  }

  function boxesOverlap(a, b) {
    if (!a || !b) {
      return false;
    }
    return a.x <= b.x + b.w &&
           a.x + a.w >= b.x &&
           a.y <= b.y + b.h &&
           a.y + a.h >= b.y;
  }

  function selectedIdsInMarquee(box) {
    if (!box) {
      return [];
    }
    const ids = [];
    for (const shape of state.shapes) {
      const bounds = getBounds(shape);
      if (!bounds) {
        continue;
      }
      const pad = Math.max(6 / state.camera.scale, (shape.width || 2) * 0.5);
      const expanded = {
        x: bounds.x - pad,
        y: bounds.y - pad,
        w: bounds.w + pad * 2,
        h: bounds.h + pad * 2
      };
      if (boxesOverlap(expanded, box)) {
        ids.push(shape.id);
      }
    }
    return ids;
  }

  function finalizeMarqueeSelection() {
    const marquee = state.pointer.marquee;
    if (!marquee) {
      return;
    }
    const box = boxFromPoints(marquee.start, marquee.current);
    const clickThreshold = 4 / state.camera.scale;
    const isClick = !box || (box.w < clickThreshold && box.h < clickThreshold);
    const before = selectedShapeIds();
    let next = before.slice();
    let touched = [];

    if (isClick) {
      if (marquee.mode === 'replace') {
        next = [];
        setStatus('Selection cleared.');
      }
    } else {
      touched = selectedIdsInMarquee(box);
      if (marquee.mode === 'add') {
        for (const id of touched) {
          if (!next.includes(id)) {
            next.push(id);
          }
        }
      } else if (marquee.mode === 'subtract') {
        next = next.filter((id) => !touched.includes(id));
      } else {
        next = touched;
      }

      if (marquee.mode === 'add') {
        const added = Math.max(0, next.length - before.length);
        setStatus(`Marquee added ${added} shape(s).`);
      } else if (marquee.mode === 'subtract') {
        const removed = Math.max(0, before.length - next.length);
        setStatus(`Marquee removed ${removed} shape(s).`);
      } else {
        setStatus(`Marquee selected ${touched.length} shape(s).`);
      }
    }

    setSelection(next);
    state.pointer.marquee = null;
  }

  function drawMarqueeOverlay() {
    const marquee = state.pointer.marquee;
    if (!marquee) {
      return;
    }
    const box = boxFromPoints(marquee.start, marquee.current);
    if (!box) {
      return;
    }

    ctx.save();
    ctx.setLineDash([10 / state.camera.scale, 6 / state.camera.scale]);
    ctx.lineWidth = 1.4 / state.camera.scale;

    if (marquee.mode === 'subtract') {
      ctx.strokeStyle = 'rgba(255, 122, 122, 0.95)';
      ctx.fillStyle = 'rgba(255, 96, 96, 0.12)';
    } else if (marquee.mode === 'add') {
      ctx.strokeStyle = 'rgba(132, 214, 255, 0.95)';
      ctx.fillStyle = 'rgba(106, 194, 255, 0.12)';
    } else {
      ctx.strokeStyle = 'rgba(176, 204, 255, 0.95)';
      ctx.fillStyle = 'rgba(125, 166, 255, 0.1)';
    }

    ctx.fillRect(box.x, box.y, box.w, box.h);
    ctx.strokeRect(box.x, box.y, box.w, box.h);
    ctx.restore();
  }

  return {
    setSelection,
    clearSelection,
    selectedShapeIds,
    selectedShapes,
    boxFromPoints,
    boxesOverlap,
    selectedIdsInMarquee,
    finalizeMarqueeSelection,
    drawMarqueeOverlay
  };
}
