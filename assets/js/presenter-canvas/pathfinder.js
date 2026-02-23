export function createPathfinderController(deps) {
  const {
    state,
    uid,
    selectedShapeIds,
    setSelection,
    pushHistory,
    render,
    setStatus
  } = deps;

  const EPS = 1e-6;

  function normalizeRectFromShape(shape) {
    if (!shape || shape.type !== 'rect') return null;
    const x1 = Math.min(shape.x1, shape.x2);
    const y1 = Math.min(shape.y1, shape.y2);
    const x2 = Math.max(shape.x1, shape.x2);
    const y2 = Math.max(shape.y1, shape.y2);
    return { x1, y1, x2, y2 };
  }

  function pathfinderBoxFromShape(shape) {
    return normalizeRectFromShape(shape);
  }

  function rectArea(box) {
    return Math.max(0, box.x2 - box.x1) * Math.max(0, box.y2 - box.y1);
  }

  function pointInRect(rect, x, y) {
    return x >= rect.x1 - EPS && x <= rect.x2 + EPS && y >= rect.y1 - EPS && y <= rect.y2 + EPS;
  }

  function uniqueSortedCuts(values) {
    const sorted = values.slice().sort((a, b) => a - b);
    const out = [];
    for (const value of sorted) {
      if (!out.length || Math.abs(out[out.length - 1] - value) > EPS) {
        out.push(value);
      }
    }
    return out;
  }

  function mergeHorizontal(cells) {
    const sorted = cells
      .slice()
      .sort((a, b) => (a.y1 - b.y1) || (a.y2 - b.y2) || (a.x1 - b.x1));

    const merged = [];
    for (const cell of sorted) {
      const last = merged[merged.length - 1];
      if (
        last &&
        Math.abs(last.y1 - cell.y1) <= EPS &&
        Math.abs(last.y2 - cell.y2) <= EPS &&
        Math.abs(last.x2 - cell.x1) <= EPS
      ) {
        last.x2 = cell.x2;
      } else {
        merged.push({ x1: cell.x1, y1: cell.y1, x2: cell.x2, y2: cell.y2 });
      }
    }
    return merged;
  }

  function mergeVertical(cells) {
    const sorted = cells
      .slice()
      .sort((a, b) => (a.x1 - b.x1) || (a.x2 - b.x2) || (a.y1 - b.y1));

    const merged = [];
    for (const cell of sorted) {
      const last = merged[merged.length - 1];
      if (
        last &&
        Math.abs(last.x1 - cell.x1) <= EPS &&
        Math.abs(last.x2 - cell.x2) <= EPS &&
        Math.abs(last.y2 - cell.y1) <= EPS
      ) {
        last.y2 = cell.y2;
      } else {
        merged.push({ x1: cell.x1, y1: cell.y1, x2: cell.x2, y2: cell.y2 });
      }
    }
    return merged;
  }

  function rectBooleanPieces(a, b, mode) {
    const xCuts = uniqueSortedCuts([a.x1, a.x2, b.x1, b.x2]);
    const yCuts = uniqueSortedCuts([a.y1, a.y2, b.y1, b.y2]);
    if (xCuts.length < 2 || yCuts.length < 2) {
      return [];
    }

    const cells = [];
    for (let yi = 0; yi < yCuts.length - 1; yi += 1) {
      for (let xi = 0; xi < xCuts.length - 1; xi += 1) {
        const x1 = xCuts[xi];
        const x2 = xCuts[xi + 1];
        const y1 = yCuts[yi];
        const y2 = yCuts[yi + 1];
        if (x2 - x1 <= EPS || y2 - y1 <= EPS) {
          continue;
        }
        const mx = (x1 + x2) / 2;
        const my = (y1 + y2) / 2;
        const inA = pointInRect(a, mx, my);
        const inB = pointInRect(b, mx, my);

        let keep = false;
        if (mode === 'union') {
          keep = inA || inB;
        } else if (mode === 'intersect') {
          keep = inA && inB;
        } else if (mode === 'subtract') {
          keep = inA && !inB;
        }

        if (keep) {
          cells.push({ x1, y1, x2, y2 });
        }
      }
    }

    if (!cells.length) {
      return [];
    }
    const mergedH = mergeHorizontal(cells);
    const mergedHV = mergeVertical(mergedH);
    return mergedHV.filter((box) => rectArea(box) > EPS);
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
    const existing = [];
    let lockedIgnoredCount = 0;
    for (const id of ids) {
      const shape = state.shapes.find((item) => item.id === id);
      if (!shape) {
        continue;
      }
      if (shape.locked) {
        lockedIgnoredCount += 1;
        continue;
      }
      existing.push(id);
    }
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
    return {
      primary,
      secondary,
      ignoredCount: Math.max(0, existing.length - 2),
      lockedIgnoredCount
    };
  }

  function replacePairWithResults(shapeA, shapeB, toAdd) {
    const removeIds = new Set([shapeA.id, shapeB.id]);
    const nextShapes = [];
    let inserted = false;

    for (const shape of state.shapes) {
      if (removeIds.has(shape.id)) {
        if (!inserted) {
          nextShapes.push(...toAdd);
          inserted = true;
        }
        continue;
      }
      nextShapes.push(shape);
    }

    if (!inserted) {
      nextShapes.push(...toAdd);
    }

    state.shapes = nextShapes;
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
    if (topShape.type !== 'rect' || other.type !== 'rect') {
      setStatus('Pathfinder currently supports rectangles only for exact results.');
      return;
    }
    const a = pathfinderBoxFromShape(topShape);
    const b = pathfinderBoxFromShape(other);
    if (!a || !b || rectArea(a) <= EPS || rectArea(b) <= EPS) {
      setStatus('Pathfinder needs two area-based objects.');
      return;
    }

    const pieces = rectBooleanPieces(a, b, mode);
    if (!pieces.length) {
      if (mode === 'intersect') {
        setStatus('No overlap to intersect.');
      } else if (mode === 'subtract') {
        setStatus('Subtract removed the primary object completely.');
      } else {
        setStatus('Pathfinder produced no geometry.');
      }
      return;
    }

    const keepRadius = mode === 'intersect' && pieces.length === 1;
    const toAdd = pieces.map((box) => makeRectFromBox(box, topShape, { keepRadius }));

    replacePairWithResults(topShape, other, toAdd);
    setSelection(toAdd.map((shape) => shape.id));
    pushHistory();
    render();

    const ignoredMsg = pair.ignoredCount > 0 ? `; used last 2 of ${pair.ignoredCount + 2} selected` : '';
    const lockedMsg = pair.lockedIgnoredCount > 0 ? `; ignored ${pair.lockedIgnoredCount} locked` : '';
    setStatus(`Pathfinder ${mode} complete: ${toAdd.length} result shape(s) (exact for rectangles)${ignoredMsg}${lockedMsg}.`);
  }

  return {
    applyPathfinder
  };
}
