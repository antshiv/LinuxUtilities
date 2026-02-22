export function createHandleController(deps) {
  const {
    state,
    selectedShapes,
    setSelection,
    snapPoint,
    constrainEndpoint,
    clamp,
    ctx
  } = deps;

  function activeEditableShape() {
    if (state.tool !== 'select') {
      return null;
    }
    const items = selectedShapes();
    if (items.length !== 1) {
      return null;
    }
    const shape = items[0];
    if (!shape) {
      return null;
    }
    if (shape.type === 'line' || shape.type === 'arrow' || shape.type === 'rect') {
      return shape;
    }
    return null;
  }

  function normalizedRectBounds(shape) {
    if (!shape || typeof shape.x1 !== 'number' || typeof shape.y1 !== 'number' || typeof shape.x2 !== 'number' || typeof shape.y2 !== 'number') {
      return null;
    }
    return {
      x1: Math.min(shape.x1, shape.x2),
      y1: Math.min(shape.y1, shape.y2),
      x2: Math.max(shape.x1, shape.x2),
      y2: Math.max(shape.y1, shape.y2)
    };
  }

  function clampRectRadiusToBounds(shape) {
    if (!shape || shape.type !== 'rect') {
      return;
    }
    const box = normalizedRectBounds(shape);
    if (!box) {
      return;
    }
    const maxRadius = Math.max(0, Math.min((box.x2 - box.x1) / 2, (box.y2 - box.y1) / 2));
    shape.radius = clamp(Number(shape.radius) || 0, 0, maxRadius);
  }

  function editHandlesForShape(shape) {
    if (!shape) {
      return [];
    }
    if (shape.type === 'line' || shape.type === 'arrow') {
      return [
        { id: 'start', kind: 'vertex', x: shape.x1, y: shape.y1 },
        { id: 'end', kind: 'vertex', x: shape.x2, y: shape.y2 }
      ];
    }
    if (shape.type === 'rect') {
      const box = normalizedRectBounds(shape);
      if (!box) {
        return [];
      }
      const maxRadius = Math.max(0, Math.min((box.x2 - box.x1) / 2, (box.y2 - box.y1) / 2));
      const radius = clamp(Number(shape.radius) || 0, 0, maxRadius);
      return [
        { id: 'nw', kind: 'vertex', x: box.x1, y: box.y1 },
        { id: 'ne', kind: 'vertex', x: box.x2, y: box.y1 },
        { id: 'se', kind: 'vertex', x: box.x2, y: box.y2 },
        { id: 'sw', kind: 'vertex', x: box.x1, y: box.y2 },
        { id: 'radius', kind: 'radius', x: box.x1 + radius, y: box.y1 - 16 / state.camera.scale }
      ];
    }
    return [];
  }

  function findEditHandleAt(x, y) {
    const shape = activeEditableShape();
    if (!shape) {
      return null;
    }
    const handles = editHandlesForShape(shape);
    const hitRadius = 10 / state.camera.scale;
    for (let i = handles.length - 1; i >= 0; i -= 1) {
      const handle = handles[i];
      if (Math.hypot(handle.x - x, handle.y - y) <= hitRadius) {
        return { shape, handle };
      }
    }
    return null;
  }

  function drawEditHandlesOverlay() {
    const shape = activeEditableShape();
    if (!shape) {
      return;
    }
    const handles = editHandlesForShape(shape);
    if (!handles.length) {
      return;
    }

    const box = shape.type === 'rect' ? normalizedRectBounds(shape) : null;
    const radiusHandle = handles.find((handle) => handle.id === 'radius');
    ctx.save();
    ctx.setLineDash([]);
    ctx.lineWidth = 1.2 / state.camera.scale;

    if (box && radiusHandle) {
      ctx.strokeStyle = 'rgba(157, 214, 255, 0.75)';
      ctx.beginPath();
      ctx.moveTo(box.x1, radiusHandle.y);
      ctx.lineTo(radiusHandle.x, radiusHandle.y);
      ctx.lineTo(radiusHandle.x, box.y1);
      ctx.stroke();
    }

    for (const handle of handles) {
      const isRadius = handle.kind === 'radius';
      const size = (isRadius ? 6.6 : 5.2) / state.camera.scale;
      ctx.fillStyle = isRadius ? 'rgba(90, 255, 191, 0.95)' : 'rgba(174, 211, 255, 0.95)';
      ctx.strokeStyle = 'rgba(13, 22, 42, 0.95)';
      ctx.beginPath();
      ctx.arc(handle.x, handle.y, size, 0, Math.PI * 2);
      ctx.fill();
      ctx.stroke();
    }
    ctx.restore();
  }

  function beginHandleDrag(hit) {
    if (!hit || !hit.shape || !hit.handle) {
      return;
    }
    const drag = {
      shapeId: hit.shape.id,
      shapeType: hit.shape.type,
      handleId: hit.handle.id,
      didMove: false
    };
    if (hit.shape.type === 'rect') {
      drag.baseBox = normalizedRectBounds(hit.shape);
    }
    state.pointer.handleDrag = drag;
  }

  function applyHandleDrag(world, shiftKey) {
    const drag = state.pointer.handleDrag;
    if (!drag) {
      return false;
    }
    const shape = state.shapes.find((item) => item.id === drag.shapeId);
    if (!shape) {
      state.pointer.handleDrag = null;
      return false;
    }

    if (drag.shapeType === 'line' || drag.shapeType === 'arrow') {
      const snapped = snapPoint(world);
      if (drag.handleId === 'start') {
        const constrained = constrainEndpoint({ type: shape.type, x1: shape.x2, y1: shape.y2 }, snapped, !!shiftKey);
        shape.x1 = constrained.x;
        shape.y1 = constrained.y;
        drag.didMove = true;
        return true;
      }
      if (drag.handleId === 'end') {
        const constrained = constrainEndpoint({ type: shape.type, x1: shape.x1, y1: shape.y1 }, snapped, !!shiftKey);
        shape.x2 = constrained.x;
        shape.y2 = constrained.y;
        drag.didMove = true;
        return true;
      }
      return false;
    }

    if (drag.shapeType === 'rect') {
      if (drag.handleId === 'radius') {
        const box = normalizedRectBounds(shape);
        if (!box) {
          return false;
        }
        const maxRadius = Math.max(0, Math.min((box.x2 - box.x1) / 2, (box.y2 - box.y1) / 2));
        shape.radius = clamp(world.x - box.x1, 0, maxRadius);
        drag.didMove = true;
        return true;
      }

      const base = drag.baseBox;
      if (!base) {
        return false;
      }
      const snapped = snapPoint(world);
      const cornerConfig = {
        nw: { ax: base.x2, ay: base.y2, sx: -1, sy: -1 },
        ne: { ax: base.x1, ay: base.y2, sx: 1, sy: -1 },
        sw: { ax: base.x2, ay: base.y1, sx: -1, sy: 1 },
        se: { ax: base.x1, ay: base.y1, sx: 1, sy: 1 }
      }[drag.handleId];
      if (!cornerConfig) {
        return false;
      }

      let cx = snapped.x;
      let cy = snapped.y;
      if (shiftKey) {
        const dx = cx - cornerConfig.ax;
        const dy = cy - cornerConfig.ay;
        const size = Math.max(Math.abs(dx), Math.abs(dy));
        const sx = Math.sign(dx) || cornerConfig.sx;
        const sy = Math.sign(dy) || cornerConfig.sy;
        cx = cornerConfig.ax + size * sx;
        cy = cornerConfig.ay + size * sy;
      }

      shape.x1 = Math.min(cornerConfig.ax, cx);
      shape.y1 = Math.min(cornerConfig.ay, cy);
      shape.x2 = Math.max(cornerConfig.ax, cx);
      shape.y2 = Math.max(cornerConfig.ay, cy);
      clampRectRadiusToBounds(shape);
      drag.didMove = true;
      return true;
    }

    return false;
  }

  return {
    activeEditableShape,
    normalizedRectBounds,
    clampRectRadiusToBounds,
    editHandlesForShape,
    findEditHandleAt,
    drawEditHandlesOverlay,
    beginHandleDrag,
    applyHandleDrag,
    setSelection
  };
}
