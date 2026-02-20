/**
 * SVG Viewer — Full-screen overlay with zoom, pan, keyboard nav.
 * Ported from C-Kernel-Engine docs, refactored to ES6 module.
 *
 * Usage:
 *   import { initSvgViewer } from './svg-viewer.js';
 *   initSvgViewer();          // call after DOM ready
 *
 * Any element with class="svg-viewer" and a child <img> becomes
 * clickable — opens in the full-screen overlay.
 */

let overlay, img, titleEl, container, zoomDisplay;
const images = [];
let currentIndex = 0;
let scale = 1, panX = 0, panY = 0;
let isDragging = false, dragStartX, dragStartY, panStartX, panStartY;

function updateTransform() {
    img.style.transform = `translate(${panX}px, ${panY}px) scale(${scale})`;
    if (zoomDisplay) zoomDisplay.textContent = `${Math.round(scale * 100)}%`;
}

function open(index) {
    if (!images[index]) return;
    currentIndex = index;
    scale = 1; panX = 0; panY = 0;
    img.src = images[index].src;
    titleEl.textContent = `${images[index].title} (${index + 1}/${images.length})`;
    updateTransform();
    overlay.style.display = 'block';
    document.body.style.overflow = 'hidden';
}

function close() {
    overlay.style.display = 'none';
    document.body.style.overflow = '';
}

function prev() { open((currentIndex - 1 + images.length) % images.length); }
function next() { open((currentIndex + 1) % images.length); }
function zoomIn()  { scale = Math.min(10, scale + 0.25); updateTransform(); }
function zoomOut() { scale = Math.max(0.5, scale - 0.25); updateTransform(); }
function resetZoom() { scale = 1; panX = 0; panY = 0; updateTransform(); }

function fitWidth() {
    if (!img.naturalWidth || !img.complete) { img.onload = fitWidth; return; }
    scale = Math.max(0.1, Math.min(10, (window.innerWidth - 150) / img.naturalWidth));
    panX = 0; panY = 0; updateTransform();
}

function fitHeight() {
    if (!img.naturalHeight || !img.complete) { img.onload = fitHeight; return; }
    scale = Math.max(0.1, Math.min(10, (window.innerHeight - 120) / img.naturalHeight));
    panX = 0; panY = 0; updateTransform();
}

/**
 * Inject the overlay HTML and wire up all events.
 * Call once after DOMContentLoaded.
 */
export function initSvgViewer() {
    // Inject overlay markup if not already present
    if (!document.getElementById('svg-overlay')) {
        const tpl = document.createElement('div');
        tpl.innerHTML = `
            <div id="svg-overlay">
                <div class="toolbar">
                    <span class="title" id="svg-title">Image</span>
                    <div class="controls">
                        <button data-action="zoomOut" title="Zoom Out (-)">−</button>
                        <span class="zoom-display" id="zoom-display">100%</span>
                        <button data-action="zoomIn" title="Zoom In (+)">+</button>
                        <span class="separator">|</span>
                        <button data-action="fitWidth" title="Fit Width (W)">Width</button>
                        <button data-action="fitHeight" title="Fit Height (H)">Height</button>
                        <button data-action="resetZoom" title="Reset (0)">Reset</button>
                        <span class="separator">|</span>
                        <button class="close-btn" data-action="close" title="Close (ESC)">&times;</button>
                    </div>
                </div>
                <button class="nav-btn nav-prev" data-action="prev" title="Previous">&larr;</button>
                <button class="nav-btn nav-next" data-action="next" title="Next">&rarr;</button>
                <div class="image-container" id="svg-container">
                    <img id="svg-image" src="" alt="">
                </div>
                <div class="hint">Scroll to zoom · Drag to pan · W/H to fit · 0 to reset · ESC to close</div>
            </div>`;
        document.body.appendChild(tpl.firstElementChild);
    }

    overlay     = document.getElementById('svg-overlay');
    img         = document.getElementById('svg-image');
    titleEl     = document.getElementById('svg-title');
    container   = document.getElementById('svg-container');
    zoomDisplay = document.getElementById('zoom-display');

    // Action map for buttons
    const actions = { zoomIn, zoomOut, fitWidth, fitHeight, resetZoom, close, prev, next };
    overlay.querySelectorAll('[data-action]').forEach(btn => {
        btn.addEventListener('click', () => {
            const fn = actions[btn.dataset.action];
            if (fn) fn();
        });
    });

    // Collect all .svg-viewer elements
    images.length = 0;
    document.querySelectorAll('.svg-viewer').forEach((viewer, index) => {
        const viewerImg = viewer.querySelector('img');
        if (viewerImg) {
            images.push({
                src: viewerImg.src,
                title: viewer.getAttribute('data-title') || viewerImg.alt || 'Diagram',
            });
            viewer.addEventListener('click', (e) => { e.preventDefault(); open(index); });
        }
    });

    // Wheel zoom
    container.addEventListener('wheel', (e) => {
        e.preventDefault();
        scale = Math.max(0.5, Math.min(10, scale + (e.deltaY > 0 ? -0.2 : 0.2)));
        updateTransform();
    });

    // Pan drag
    container.addEventListener('mousedown', (e) => {
        isDragging = true;
        dragStartX = e.clientX; dragStartY = e.clientY;
        panStartX = panX; panStartY = panY;
    });
    document.addEventListener('mousemove', (e) => {
        if (!isDragging) return;
        panX = panStartX + (e.clientX - dragStartX);
        panY = panStartY + (e.clientY - dragStartY);
        updateTransform();
    });
    document.addEventListener('mouseup', () => { isDragging = false; });

    // Keyboard shortcuts
    document.addEventListener('keydown', (e) => {
        if (overlay.style.display !== 'block') return;
        const key = e.key;
        if (key === 'Escape') close();
        else if (key === 'ArrowLeft') prev();
        else if (key === 'ArrowRight') next();
        else if (key === '+' || key === '=') zoomIn();
        else if (key === '-') zoomOut();
        else if (key === '0') resetZoom();
        else if (key === 'w' || key === 'W') fitWidth();
        else if (key === 'h' || key === 'H') fitHeight();
    });

    // Click backdrop to close
    overlay.addEventListener('click', (e) => {
        if (e.target === overlay || e.target === container) close();
    });
}
