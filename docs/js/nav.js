/**
 * Mobile navigation — hamburger toggle + dropdown accordion.
 *
 * Usage:
 *   import { initNav } from './nav.js';
 *   initNav();   // call after DOM ready
 */

export function initNav() {
    const navToggle = document.querySelector('.nav-toggle');
    const siteNav   = document.querySelector('.site-nav');
    const dropdowns = document.querySelectorAll('.nav-dropdown');

    if (navToggle && siteNav) {
        navToggle.addEventListener('click', () => {
            navToggle.classList.toggle('active');
            siteNav.classList.toggle('open');
            navToggle.setAttribute('aria-expanded', siteNav.classList.contains('open'));
        });

        // Close menu on non-dropdown link click (mobile)
        siteNav.querySelectorAll('a:not(.nav-dropdown-menu a)').forEach(link => {
            link.addEventListener('click', () => {
                if (window.innerWidth <= 768) {
                    navToggle.classList.remove('active');
                    siteNav.classList.remove('open');
                }
            });
        });
    }

    // Dropdown accordion on mobile
    dropdowns.forEach(dropdown => {
        const trigger = dropdown.querySelector('.nav-trigger');
        if (trigger) {
            trigger.addEventListener('click', (e) => {
                if (window.innerWidth <= 768) {
                    e.preventDefault();
                    dropdowns.forEach(d => { if (d !== dropdown) d.classList.remove('open'); });
                    dropdown.classList.toggle('open');
                }
            });
        }
    });

    // Close dropdowns when clicking outside (mobile)
    document.addEventListener('click', (e) => {
        if (window.innerWidth <= 768 && !e.target.closest('.nav-dropdown')) {
            dropdowns.forEach(d => d.classList.remove('open'));
        }
    });
}
