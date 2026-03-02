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
    if (shape.hidden || shape.locked) {
      return null;
    }
    if (shape.type === 'line' || shape.type === 'arrow' || shape.type === 'rect' || shape.type === 'ellipse') {
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

  function rotatePoint(px, py, cx, cy, angle) {
    const cos = Math.cos(angle);
    const sin = Math.sin(angle);
    const dx = px - cx;
    const dy = py - cy;
    return { x: cx + dx * cos - dy * sin, y: cy + dx * sin + dy * cos };
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
      const rotation = Number(shape.rotation) || 0;
      const cx = (box.x1 + box.x2) / 2;
      const cy = (box.y1 + box.y2) / 2;
      const rotHandlePt = rotatePoint(cx, box.y1 - 28 / state.camera.scale, cx, cy, rotation);
      const rotHandle = { id: 'rotation', kind: 'rotation', x: rotHandlePt.x, y: rotHandlePt.y };

      if (Math.abs(rotation) < 0.01) {
        const maxRadius = Math.max(0, Math.min((box.x2 - box.x1) / 2, (box.y2 - box.y1) / 2));
        const radius = clamp(Number(shape.radius) || 0, 0, maxRadius);
        return [
          { id: 'nw', kind: 'vertex', x: box.x1, y: box.y1 },
          { id: 'ne', kind: 'vertex', x: box.x2, y: box.y1 },
          { id: 'se', kind: 'vertex', x: box.x2, y: box.y2 },
          { id: 'sw', kind: 'vertex', x: box.x1, y: box.y2 },
          { id: 'radius', kind: 'radius', x: box.x1 + radius, y: box.y1 - 16 / state.camera.scale },
          rotHandle
        ];
      }
      const nw = rotatePoint(box.x1, box.y1, cx, cy, rotation);
      const ne = rotatePoint(box.x2, box.y1, cx, cy, rotation);
      const se = rotatePoint(box.x2, box.y2, cx, cy, rotation);
      const sw = rotatePoint(box.x1, box.y2, cx, cy, rotation);
      return [
        { id: 'nw', kind: 'vertex', x: nw.x, y: nw.y },
        { id: 'ne', kind: 'vertex', x: ne.x, y: ne.y },
        { id: 'se', kind: 'vertex', x: se.x, y: se.y },
        { id: 'sw', kind: 'vertex', x: sw.x, y: sw.y },
        rotHandle
      ];
    }
    if (shape.type === 'ellipse') {
      const box = normalizedRectBounds(shape);
      if (!box) {
        return [];
      }
      const rotation = Number(shape.rotation) || 0;
      const cx = (box.x1 + box.x2) / 2;
      const cy = (box.y1 + box.y2) / 2;
      const hh = (box.y2 - box.y1) / 2;
      const rotHandlePt = rotatePoint(cx, cy - hh - 28 / state.camera.scale, cx, cy, rotation);
      return [
        { id: 'rotation', kind: 'rotation', x: rotHandlePt.x, y: rotHandlePt.y }
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

    const box = (shape.type === 'rect' || shape.type === 'ellipse') ? normalizedRectBounds(shape) : null;
    const radiusHandle = handles.find((handle) => handle.id === 'radius');
    const rotHandle = handles.find((handle) => handle.id === 'rotation');
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

    if (box && rotHandle) {
      const cx = (box.x1 + box.x2) / 2;
      const cy = (box.y1 + box.y2) / 2;
      const rotation = Number(shape.rotation) || 0;
      const topPt = rotatePoint(cx, box.y1, cx, cy, rotation);
      ctx.strokeStyle = 'rgba(255, 200, 80, 0.65)';
      ctx.setLineDash([3 / state.camera.scale, 3 / state.camera.scale]);
      ctx.beginPath();
      ctx.moveTo(topPt.x, topPt.y);
      ctx.lineTo(rotHandle.x, rotHandle.y);
      ctx.stroke();
      ctx.setLineDash([]);
    }

    for (const handle of handles) {
      const isRadius = handle.kind === 'radius';
      const isRotation = handle.kind === 'rotation';
      const size = (isRadius || isRotation ? 6.6 : 5.2) / state.camera.scale;
      if (isRotation) {
        ctx.fillStyle = 'rgba(255, 200, 80, 0.95)';
      } else if (isRadius) {
        ctx.fillStyle = 'rgba(90, 255, 191, 0.95)';
      } else {
        ctx.fillStyle = 'rgba(174, 211, 255, 0.95)';
      }
      ctx.strokeStyle = 'rgba(13, 22, 42, 0.95)';
      ctx.beginPath();
      if (isRotation) {
        ctx.arc(handle.x, handle.y, size, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        // inner arrow indicator
        ctx.strokeStyle = 'rgba(13, 22, 42, 0.7)';
        ctx.lineWidth = 1 / state.camera.scale;
        ctx.beginPath();
        ctx.arc(handle.x, handle.y, size * 0.5, -Math.PI * 0.75, Math.PI * 0.5);
        ctx.stroke();
      } else {
        ctx.arc(handle.x, handle.y, size, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
      }
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
    if (shape.hidden || shape.locked) {
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

    if (drag.shapeType === 'rect' || drag.shapeType === 'ellipse') {
      if (drag.handleId === 'rotation') {
        const box = normalizedRectBounds(shape);
        if (!box) {
          return false;
        }
        const cx = (box.x1 + box.x2) / 2;
        const cy = (box.y1 + box.y2) / 2;
        // angle from center to pointer; handle starts at top => subtract -PI/2
        let angle = Math.atan2(world.y - cy, world.x - cx) + Math.PI / 2;
        if (shiftKey) {
          // snap to 15-degree increments
          const step = Math.PI / 12;
          angle = Math.round(angle / step) * step;
        }
        shape.rotation = angle;
        drag.didMove = true;
        return true;
      }
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
