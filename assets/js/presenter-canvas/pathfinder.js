export function createPathfinderController(deps) {
  const {
    state,
    uid,
    getBounds,
    selectedShapeIds,
    setSelection,
    pushHistory,
    render,
    setStatus
  } = deps;

  function normalizeRectFromShape(shape) {
    if (!shape || shape.type !== 'rect') return null;
    const x1 = Math.min(shape.x1, shape.x2);
    const y1 = Math.min(shape.y1, shape.y2);
    const x2 = Math.max(shape.x1, shape.x2);
    const y2 = Math.max(shape.y1, shape.y2);
    return { x1, y1, x2, y2 };
  }

  function pathfinderBoxFromShape(shape) {
    if (!shape) {
      return null;
    }
    if (shape.type === 'rect') {
      return normalizeRectFromShape(shape);
    }
    const bounds = getBounds(shape);
    if (!bounds) {
      return null;
    }
    const strokePad = Math.max(0.5, (shape.width || 1) * 0.5);
    return {
      x1: bounds.x - strokePad,
      y1: bounds.y - strokePad,
      x2: bounds.x + bounds.w + strokePad,
      y2: bounds.y + bounds.h + strokePad
    };
  }

  function rectArea(box) {
    return Math.max(0, box.x2 - box.x1) * Math.max(0, box.y2 - box.y1);
  }

  function makeRectFromBox(box, styleSource, options) {
    const opts = options || {};
    return {
      id: uid(),
      type: 'rect',
      x1: box.x1,
      y1: box.y1,
      x2: box.x2,
      y2: box.y2,
      color: styleSource.color || state.strokeColor,
      width: styleSource.width || state.lineWidth,
      style: styleSource.style || state.lineStyle,
      animated: !!styleSource.animated,
      opacity: typeof styleSource.opacity === 'number' ? styleSource.opacity : 1,
      radius: opts.keepRadius ? Number(styleSource.radius) || 0 : 0
    };
  }

  function selectedPathfinderPair() {
    const ids = selectedShapeIds();
    if (ids.length < 2) {
      return { error: 'Select two objects first.' };
    }
    const existing = ids.filter((id) => state.shapes.some((shape) => shape.id === id));
    if (existing.length < 2) {
      return { error: 'Select two objects first.' };
    }

    const preferredPrimaryId = existing.includes(state.selectedId) ? state.selectedId : existing[existing.length - 1];
    let secondaryId = null;
    for (let i = existing.length - 1; i >= 0; i -= 1) {
      if (existing[i] !== preferredPrimaryId) {
        secondaryId = existing[i];
        break;
      }
    }
    if (!secondaryId) {
      return { error: 'Select two different objects first.' };
    }

    const primary = state.shapes.find((shape) => shape.id === preferredPrimaryId);
    const secondary = state.shapes.find((shape) => shape.id === secondaryId);
    if (!primary || !secondary) {
      return { error: 'Select two objects first.' };
    }
    return { primary, secondary, ignoredCount: Math.max(0, existing.length - 2) };
  }

  function applyPathfinder(mode) {
    const pair = selectedPathfinderPair();
    if (!pair || pair.error) {
      setStatus((pair && pair.error) || 'Select two objects first.');
      return;
    }
    if (mode !== 'union' && mode !== 'intersect' && mode !== 'subtract') {
      setStatus('Unknown pathfinder mode.');
      return;
    }
    const topShape = pair.primary;
    const other = pair.secondary;
    const shapeA = topShape;
    const shapeB = other;
    const approximate = topShape.type !== 'rect' || other.type !== 'rect';
    const a = pathfinderBoxFromShape(topShape);
    const b = pathfinderBoxFromShape(other);
    if (!a || !b || rectArea(a) <= 0.0001 || rectArea(b) <= 0.0001) {
      setStatus('Pathfinder needs two area-based objects.');
      return;
    }
    const overlap = {
      x1: Math.max(a.x1, b.x1),
      y1: Math.max(a.y1, b.y1),
      x2: Math.min(a.x2, b.x2),
      y2: Math.min(a.y2, b.y2)
    };
    const hasOverlap = rectArea(overlap) > 0.0001;

    const toAdd = [];
    if (mode === 'union') {
      toAdd.push(makeRectFromBox({
        x1: Math.min(a.x1, b.x1),
        y1: Math.min(a.y1, b.y1),
        x2: Math.max(a.x2, b.x2),
        y2: Math.max(a.y2, b.y2)
      }, topShape));
    } else if (mode === 'intersect') {
      if (!hasOverlap) {
        setStatus('No overlap to intersect.');
        return;
      }
      toAdd.push(makeRectFromBox(overlap, topShape));
    } else if (mode === 'subtract') {
      if (!hasOverlap) {
        setStatus('No overlap to subtract.');
        return;
      }
      const parts = [
        { x1: a.x1, y1: a.y1, x2: a.x2, y2: overlap.y1 },
        { x1: a.x1, y1: overlap.y2, x2: a.x2, y2: a.y2 },
        { x1: a.x1, y1: overlap.y1, x2: overlap.x1, y2: overlap.y2 },
        { x1: overlap.x2, y1: overlap.y1, x2: a.x2, y2: overlap.y2 }
      ].filter((box) => rectArea(box) > 0.0001);
      if (!parts.length) {
        setStatus('Subtract removed the primary object completely.');
        return;
      }
      for (const box of parts) {
        toAdd.push(makeRectFromBox(box, topShape));
      }
    }

    state.shapes = state.shapes.filter((shape) => shape.id !== shapeA.id && shape.id !== shapeB.id);
    state.shapes.push(...toAdd);
    setSelection(toAdd.map((shape) => shape.id));
    pushHistory();
    render();
    const approxMsg = approximate ? ' (bounds approximation)' : '';
    const ignoredMsg = pair.ignoredCount > 0 ? `; used last 2 of ${pair.ignoredCount + 2} selected` : '';
    setStatus(`Pathfinder ${mode} complete: ${toAdd.length} result shape(s)${approxMsg}${ignoredMsg}.`);
  }

  return {
    applyPathfinder
  };
}
