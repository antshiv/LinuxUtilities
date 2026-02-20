/**
 * App entry point — imports all modules and initializes on DOM ready.
 *
 * This is loaded as <script type="module"> so imports work natively
 * in all modern browsers (no bundler needed).
 */

import { initNav }       from './nav.js';
import { initSvgViewer } from './svg-viewer.js';

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', boot);
} else {
    boot();
}

function boot() {
    initNav();
    initSvgViewer();
}
