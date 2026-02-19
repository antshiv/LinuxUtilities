#define _POSIX_C_SOURCE 200809L

#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    double radius;
    double dim_alpha;
    double feather_ratio;
    double ring_alpha;
    int fps;
} SpotlightConfig;

typedef struct {
    Display *dpy;
    int screen;
    Window root;
    Window win;
    Visual *visual;
    int depth;
    Colormap colormap;
    cairo_surface_t *surface;
    cairo_t *cr;
    int width;
    int height;
    int escape_keycode;
    unsigned int lock_mask;
    unsigned int numlock_mask;
    bool escape_grabbed;
} SpotlightContext;

static volatile sig_atomic_t g_running = 1;

static void on_signal(int sig) {
    (void)sig;
    g_running = 0;
}

static double clampd(double value, double lo, double hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

static int clampi(int value, int lo, int hi) {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

static bool parse_double_arg(const char *text, double *out) {
    char *end = NULL;
    double value = 0.0;

    if (!text || !*text || !out) {
        return false;
    }
    value = strtod(text, &end);
    if (!end || *end != '\0') {
        return false;
    }
    *out = value;
    return true;
}

static bool parse_int_arg(const char *text, int *out) {
    char *end = NULL;
    long value = 0;

    if (!text || !*text || !out) {
        return false;
    }
    value = strtol(text, &end, 10);
    if (!end || *end != '\0') {
        return false;
    }
    if (value < INT_MIN || value > INT_MAX) {
        return false;
    }
    *out = (int)value;
    return true;
}

static void print_usage(FILE *stream, const char *argv0) {
    fprintf(stream,
            "Usage: %s [--radius px] [--dim alpha] [--fps n] [--feather ratio] [--ring alpha]\n"
            "Defaults: --radius 180 --dim 0.68 --fps 50 --feather 0.30 --ring 0.26\n"
            "Emergency exit: press Esc, or run `pkill -f cursor_spotlight`.\n",
            argv0 ? argv0 : "cursor_spotlight");
}

static bool parse_args(int argc, char **argv, SpotlightConfig *cfg) {
    int i = 0;

    if (!cfg) {
        return false;
    }

    cfg->radius = 180.0;
    cfg->dim_alpha = 0.68;
    cfg->fps = 50;
    cfg->feather_ratio = 0.30;
    cfg->ring_alpha = 0.26;

    for (i = 1; i < argc; ++i) {
        double dval = 0.0;
        int ival = 0;
        const char *arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(stdout, argv[0]);
            return false;
        }
        if (strcmp(arg, "--radius") == 0) {
            if (i + 1 >= argc || !parse_double_arg(argv[++i], &dval)) {
                fprintf(stderr, "Invalid --radius value.\n");
                return false;
            }
            cfg->radius = dval;
            continue;
        }
        if (strcmp(arg, "--dim") == 0) {
            if (i + 1 >= argc || !parse_double_arg(argv[++i], &dval)) {
                fprintf(stderr, "Invalid --dim value.\n");
                return false;
            }
            cfg->dim_alpha = dval;
            continue;
        }
        if (strcmp(arg, "--fps") == 0) {
            if (i + 1 >= argc || !parse_int_arg(argv[++i], &ival)) {
                fprintf(stderr, "Invalid --fps value.\n");
                return false;
            }
            cfg->fps = ival;
            continue;
        }
        if (strcmp(arg, "--feather") == 0) {
            if (i + 1 >= argc || !parse_double_arg(argv[++i], &dval)) {
                fprintf(stderr, "Invalid --feather value.\n");
                return false;
            }
            cfg->feather_ratio = dval;
            continue;
        }
        if (strcmp(arg, "--ring") == 0) {
            if (i + 1 >= argc || !parse_double_arg(argv[++i], &dval)) {
                fprintf(stderr, "Invalid --ring value.\n");
                return false;
            }
            cfg->ring_alpha = dval;
            continue;
        }

        fprintf(stderr, "Unknown argument: %s\n", arg);
        print_usage(stderr, argv[0]);
        return false;
    }

    cfg->radius = clampd(cfg->radius, 20.0, 900.0);
    cfg->dim_alpha = clampd(cfg->dim_alpha, 0.05, 0.95);
    cfg->fps = clampi(cfg->fps, 5, 240);
    cfg->feather_ratio = clampd(cfg->feather_ratio, 0.05, 0.95);
    cfg->ring_alpha = clampd(cfg->ring_alpha, 0.0, 1.0);
    return true;
}

static bool compositor_present(Display *dpy, int screen) {
    char atom_name[32];
    Atom atom = None;

    if (!dpy) {
        return false;
    }
    snprintf(atom_name, sizeof(atom_name), "_NET_WM_CM_S%d", screen);
    atom = XInternAtom(dpy, atom_name, True);
    if (atom == None) {
        return false;
    }
    return XGetSelectionOwner(dpy, atom) != None;
}

static unsigned int modifier_mask_for_keysym(Display *dpy, KeySym sym) {
    XModifierKeymap *map = NULL;
    KeyCode target = 0;
    unsigned int mask = 0;
    int mod = 0;
    int key = 0;

    if (!dpy || sym == NoSymbol) {
        return 0;
    }

    target = XKeysymToKeycode(dpy, sym);
    if (!target) {
        return 0;
    }

    map = XGetModifierMapping(dpy);
    if (!map || map->max_keypermod <= 0) {
        if (map) {
            XFreeModifiermap(map);
        }
        return 0;
    }

    for (mod = 0; mod < 8; ++mod) {
        for (key = 0; key < map->max_keypermod; ++key) {
            KeyCode code = map->modifiermap[mod * map->max_keypermod + key];
            if (code == target) {
                mask |= (1u << mod);
                break;
            }
        }
    }

    XFreeModifiermap(map);
    return mask;
}

static void install_escape_grab(SpotlightContext *ctx) {
    unsigned int mods[4];
    int mod_count = 0;
    int i = 0;

    if (!ctx || !ctx->dpy || !ctx->root) {
        return;
    }

    ctx->escape_keycode = (int)XKeysymToKeycode(ctx->dpy, XK_Escape);
    if (ctx->escape_keycode <= 0) {
        return;
    }

    ctx->lock_mask = modifier_mask_for_keysym(ctx->dpy, XK_Caps_Lock);
    ctx->numlock_mask = modifier_mask_for_keysym(ctx->dpy, XK_Num_Lock);

    mods[mod_count++] = 0;
    if (ctx->lock_mask != 0) {
        mods[mod_count++] = ctx->lock_mask;
    }
    if (ctx->numlock_mask != 0) {
        mods[mod_count++] = ctx->numlock_mask;
    }
    if (ctx->lock_mask != 0 && ctx->numlock_mask != 0) {
        mods[mod_count++] = ctx->lock_mask | ctx->numlock_mask;
    }

    for (i = 0; i < mod_count; ++i) {
        XGrabKey(ctx->dpy,
                 ctx->escape_keycode,
                 mods[i],
                 ctx->root,
                 False,
                 GrabModeAsync,
                 GrabModeAsync);
    }
    XSync(ctx->dpy, False);
    ctx->escape_grabbed = true;
}

static void uninstall_escape_grab(SpotlightContext *ctx) {
    unsigned int mods[4];
    int mod_count = 0;
    int i = 0;

    if (!ctx || !ctx->dpy || !ctx->root || !ctx->escape_grabbed || ctx->escape_keycode <= 0) {
        return;
    }

    mods[mod_count++] = 0;
    if (ctx->lock_mask != 0) {
        mods[mod_count++] = ctx->lock_mask;
    }
    if (ctx->numlock_mask != 0) {
        mods[mod_count++] = ctx->numlock_mask;
    }
    if (ctx->lock_mask != 0 && ctx->numlock_mask != 0) {
        mods[mod_count++] = ctx->lock_mask | ctx->numlock_mask;
    }

    for (i = 0; i < mod_count; ++i) {
        XUngrabKey(ctx->dpy, ctx->escape_keycode, mods[i], ctx->root);
    }
    XSync(ctx->dpy, False);
    ctx->escape_grabbed = false;
}

static bool find_argb_visual(Display *dpy, int screen, Visual **visual_out, int *depth_out) {
    XVisualInfo tpl;
    XVisualInfo *infos = NULL;
    int count = 0;
    int i = 0;
    bool found = false;

    if (!dpy || !visual_out || !depth_out) {
        return false;
    }

    memset(&tpl, 0, sizeof(tpl));
    tpl.screen = screen;
    tpl.depth = 32;
    tpl.class = TrueColor;
    infos = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask | VisualClassMask, &tpl, &count);
    if (!infos || count <= 0) {
        return false;
    }

    for (i = 0; i < count; ++i) {
        XRenderPictFormat *fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
        if (fmt && fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
            *visual_out = infos[i].visual;
            *depth_out = infos[i].depth;
            found = true;
            break;
        }
    }
    XFree(infos);
    return found;
}

static void set_window_properties(Display *dpy, Window win) {
    Atom wm_state = None;
    Atom wm_state_above = None;
    Atom wm_type = None;
    Atom wm_type_dock = None;
    Atom bypass = None;
    unsigned long bypass_off = 0;

    if (!dpy || !win) {
        return;
    }

    wm_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    wm_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    if (wm_type != None && wm_type_dock != None) {
        XChangeProperty(dpy,
                        win,
                        wm_type,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char *)&wm_type_dock,
                        1);
    }

    wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    wm_state_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    if (wm_state != None && wm_state_above != None) {
        XChangeProperty(dpy,
                        win,
                        wm_state,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (unsigned char *)&wm_state_above,
                        1);
    }

    bypass = XInternAtom(dpy, "_NET_WM_BYPASS_COMPOSITOR", False);
    if (bypass != None) {
        XChangeProperty(dpy,
                        win,
                        bypass,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        (unsigned char *)&bypass_off,
                        1);
    }
}

static bool create_overlay_window(SpotlightContext *ctx) {
    XSetWindowAttributes attrs;
    XserverRegion input_region = 0;
    unsigned long mask = 0;

    if (!ctx || !ctx->dpy) {
        return false;
    }

    ctx->root = RootWindow(ctx->dpy, ctx->screen);
    ctx->width = DisplayWidth(ctx->dpy, ctx->screen);
    ctx->height = DisplayHeight(ctx->dpy, ctx->screen);

    memset(&attrs, 0, sizeof(attrs));
    ctx->colormap = XCreateColormap(ctx->dpy, ctx->root, ctx->visual, AllocNone);
    attrs.colormap = ctx->colormap;
    attrs.override_redirect = True;
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;
    attrs.event_mask = ExposureMask | StructureNotifyMask;

    mask = CWColormap | CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask;
    ctx->win = XCreateWindow(ctx->dpy,
                             ctx->root,
                             0,
                             0,
                             (unsigned int)ctx->width,
                             (unsigned int)ctx->height,
                             0,
                             ctx->depth,
                             InputOutput,
                             ctx->visual,
                             mask,
                             &attrs);
    if (!ctx->win) {
        return false;
    }

    set_window_properties(ctx->dpy, ctx->win);

    input_region = XFixesCreateRegion(ctx->dpy, NULL, 0);
    XFixesSetWindowShapeRegion(ctx->dpy, ctx->win, ShapeInput, 0, 0, input_region);
    XFixesDestroyRegion(ctx->dpy, input_region);

    XMapRaised(ctx->dpy, ctx->win);
    XFlush(ctx->dpy);
    return true;
}

static bool create_cairo_surface(SpotlightContext *ctx) {
    if (!ctx || !ctx->dpy || !ctx->win || !ctx->visual) {
        return false;
    }

    ctx->surface = cairo_xlib_surface_create(ctx->dpy, ctx->win, ctx->visual, ctx->width, ctx->height);
    if (!ctx->surface || cairo_surface_status(ctx->surface) != CAIRO_STATUS_SUCCESS) {
        return false;
    }

    ctx->cr = cairo_create(ctx->surface);
    if (!ctx->cr || cairo_status(ctx->cr) != CAIRO_STATUS_SUCCESS) {
        return false;
    }
    cairo_set_antialias(ctx->cr, CAIRO_ANTIALIAS_BEST);
    return true;
}

static void draw_frame(SpotlightContext *ctx, const SpotlightConfig *cfg, int mx, int my) {
    cairo_pattern_t *feather = NULL;
    double radius = 0.0;
    double inner = 0.0;

    if (!ctx || !ctx->cr || !cfg) {
        return;
    }

    radius = cfg->radius;
    inner = radius * (1.0 - cfg->feather_ratio);
    if (inner < 4.0) {
        inner = 4.0;
    }
    if (inner >= radius - 1.0) {
        inner = radius - 1.0;
    }

    cairo_save(ctx->cr);
    cairo_set_operator(ctx->cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(ctx->cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(ctx->cr);
    cairo_restore(ctx->cr);

    cairo_set_operator(ctx->cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(ctx->cr, 0.0, 0.0, 0.0, cfg->dim_alpha);
    cairo_paint(ctx->cr);

    cairo_set_operator(ctx->cr, CAIRO_OPERATOR_DEST_OUT);
    feather = cairo_pattern_create_radial((double)mx, (double)my, inner, (double)mx, (double)my, radius);
    cairo_pattern_add_color_stop_rgba(feather, 0.0, 0.0, 0.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(feather, 1.0, 0.0, 0.0, 0.0, 0.0);
    cairo_set_source(ctx->cr, feather);
    cairo_arc(ctx->cr, (double)mx, (double)my, radius, 0.0, 2.0 * M_PI);
    cairo_fill(ctx->cr);
    cairo_pattern_destroy(feather);

    if (cfg->ring_alpha > 0.0) {
        cairo_set_operator(ctx->cr, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(ctx->cr, 1.0, 1.0, 1.0, cfg->ring_alpha);
        cairo_set_line_width(ctx->cr, 2.0);
        cairo_arc(ctx->cr, (double)mx, (double)my, radius - 1.0, 0.0, 2.0 * M_PI);
        cairo_stroke(ctx->cr);
    }

    cairo_surface_flush(ctx->surface);
    XFlush(ctx->dpy);
}

static void handle_pending_events(SpotlightContext *ctx) {
    if (!ctx || !ctx->dpy) {
        return;
    }

    while (XPending(ctx->dpy) > 0) {
        XEvent ev;
        XNextEvent(ctx->dpy, &ev);
        if (ev.type == KeyPress) {
            KeySym sym = XLookupKeysym(&ev.xkey, 0);
            if (sym == XK_Escape) {
                g_running = 0;
                continue;
            }
        }
        if (ev.type == ConfigureNotify && ev.xconfigure.window == ctx->win) {
            if (ctx->width != ev.xconfigure.width || ctx->height != ev.xconfigure.height) {
                ctx->width = ev.xconfigure.width;
                ctx->height = ev.xconfigure.height;
                if (ctx->surface) {
                    cairo_xlib_surface_set_size(ctx->surface, ctx->width, ctx->height);
                }
            }
        }
    }
}

static void cleanup(SpotlightContext *ctx) {
    if (!ctx) {
        return;
    }
    if (ctx->dpy && ctx->root && ctx->escape_grabbed) {
        uninstall_escape_grab(ctx);
    }
    if (ctx->cr) {
        cairo_destroy(ctx->cr);
        ctx->cr = NULL;
    }
    if (ctx->surface) {
        cairo_surface_destroy(ctx->surface);
        ctx->surface = NULL;
    }
    if (ctx->dpy && ctx->win) {
        XDestroyWindow(ctx->dpy, ctx->win);
        ctx->win = 0;
    }
    if (ctx->dpy && ctx->colormap) {
        XFreeColormap(ctx->dpy, ctx->colormap);
        ctx->colormap = 0;
    }
    if (ctx->dpy) {
        XCloseDisplay(ctx->dpy);
        ctx->dpy = NULL;
    }
}

int main(int argc, char **argv) {
    SpotlightConfig cfg;
    SpotlightContext ctx;
    struct timespec sleep_ts;
    int pointer_x = 0;
    int pointer_y = 0;
    int i = 0;

    memset(&ctx, 0, sizeof(ctx));
    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        }
    }
    if (!parse_args(argc, argv, &cfg)) {
        return EXIT_FAILURE;
    }

    signal(SIGINT, on_signal);
    signal(SIGTERM, on_signal);

    ctx.dpy = XOpenDisplay(NULL);
    if (!ctx.dpy) {
        fprintf(stderr, "cursor_spotlight: could not connect to X display.\n");
        return EXIT_FAILURE;
    }

    ctx.screen = DefaultScreen(ctx.dpy);
    if (!find_argb_visual(ctx.dpy, ctx.screen, &ctx.visual, &ctx.depth)) {
        fprintf(stderr, "cursor_spotlight: ARGB visual unavailable (is compositing enabled?).\n");
        cleanup(&ctx);
        return EXIT_FAILURE;
    }

    if (!compositor_present(ctx.dpy, ctx.screen)) {
        fprintf(stderr, "cursor_spotlight: no compositor detected; refusing to start overlay.\n");
        fprintf(stderr, "cursor_spotlight: start a compositor (for example: picom) and retry.\n");
        cleanup(&ctx);
        return EXIT_FAILURE;
    }

    if (!create_overlay_window(&ctx)) {
        fprintf(stderr, "cursor_spotlight: failed to create overlay window.\n");
        cleanup(&ctx);
        return EXIT_FAILURE;
    }
    install_escape_grab(&ctx);
    if (!create_cairo_surface(&ctx)) {
        fprintf(stderr, "cursor_spotlight: failed to create Cairo surface.\n");
        cleanup(&ctx);
        return EXIT_FAILURE;
    }

    pointer_x = ctx.width / 2;
    pointer_y = ctx.height / 2;

    sleep_ts.tv_sec = 0;
    sleep_ts.tv_nsec = (long)(1000000000.0 / (double)cfg.fps);
    if (sleep_ts.tv_nsec < 1000000L) {
        sleep_ts.tv_nsec = 1000000L;
    }

    while (g_running) {
        Window root_return = 0;
        Window child_return = 0;
        int root_x = pointer_x;
        int root_y = pointer_y;
        int win_x = 0;
        int win_y = 0;
        unsigned int mask_return = 0;

        handle_pending_events(&ctx);

        if (XQueryPointer(ctx.dpy,
                          ctx.root,
                          &root_return,
                          &child_return,
                          &root_x,
                          &root_y,
                          &win_x,
                          &win_y,
                          &mask_return)) {
            pointer_x = root_x;
            pointer_y = root_y;
        }

        draw_frame(&ctx, &cfg, pointer_x, pointer_y);
        nanosleep(&sleep_ts, NULL);
    }

    cleanup(&ctx);
    return EXIT_SUCCESS;
}
