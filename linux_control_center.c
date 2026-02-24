#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <cairo-svg.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <pango/pango.h>

typedef enum {
    GTK4_MARK_ARROW = 0,
    GTK4_MARK_LINE,
    GTK4_MARK_RECT,
    GTK4_MARK_CALLOUT,
    GTK4_MARK_STAMP,
    GTK4_MARK_TEXT
} Gtk4MarkKind;

typedef enum {
    GTK4_TOOL_SELECT = 0,
    GTK4_TOOL_ARROW,
    GTK4_TOOL_LINE,
    GTK4_TOOL_RECT,
    GTK4_TOOL_CALLOUT,
    GTK4_TOOL_TEXT,
    GTK4_TOOL_STAMP
} Gtk4Tool;

typedef enum {
    GTK4_ARROW_STYLE_CLASSIC = 0,
    GTK4_ARROW_STYLE_BOLD,
    GTK4_ARROW_STYLE_DASHED,
    GTK4_ARROW_STYLE_DOTTED,
    GTK4_ARROW_STYLE_DOUBLE
} Gtk4ArrowStyle;

typedef enum {
    GTK4_CALLOUT_STYLE_ROUNDED = 0,
    GTK4_CALLOUT_STYLE_LEFT,
    GTK4_CALLOUT_STYLE_RIGHT,
    GTK4_CALLOUT_STYLE_HIGHLIGHT,
    GTK4_CALLOUT_STYLE_SHADOW
} Gtk4CalloutStyle;

typedef enum {
    GTK4_TEXT_STYLE_NORMAL = 0,
    GTK4_TEXT_STYLE_BOLD,
    GTK4_TEXT_STYLE_ITALIC,
    GTK4_TEXT_STYLE_BOLD_ITALIC
} Gtk4TextStyle;

static const gchar *const GTK4_ARROW_STYLE_IDS[] = {
    "classic",
    "bold",
    "dashed",
    "dotted",
    "double"
};

static const gchar *const GTK4_ARROW_STYLE_LABELS[] = {
    "Classic",
    "Bold",
    "Dashed",
    "Dotted",
    "Double"
};

static const gchar *const GTK4_CALLOUT_STYLE_IDS[] = {
    "rounded",
    "left",
    "right",
    "highlight",
    "shadow"
};

static const gchar *const GTK4_CALLOUT_STYLE_LABELS[] = {
    "Rounded",
    "Left Tail",
    "Right Tail",
    "Highlight",
    "Shadow"
};

static const gchar *const GTK4_FONT_STYLE_IDS[] = {
    "normal",
    "bold",
    "italic",
    "bolditalic"
};

static const gchar *const GTK4_FONT_STYLE_LABELS[] = {
    "Normal",
    "Bold",
    "Italic",
    "Bold Italic"
};

static const gchar *const GTK4_STAMP_IDS[] = {
    "1",
    "2",
    "3",
    "!",
    "?",
    "OK"
};

static const gchar *const GTK4_STAMP_LABELS[] = {
    "1",
    "2",
    "3",
    "!",
    "?",
    "OK"
};

typedef struct {
    Gtk4MarkKind kind;
    gdouble x1;
    gdouble y1;
    gdouble x2;
    gdouble y2;
    GdkRGBA color;
    gdouble stroke_width;
    Gtk4ArrowStyle arrow_style;
    gdouble arrow_head_size;
    gdouble arrow_head_angle_deg;
    gboolean arrow_shadow;
    gdouble arrow_shadow_offset;
    gboolean rect_rounded;
    gboolean rect_fill;
    gdouble rect_fill_alpha;
    gboolean rect_shadow;
    Gtk4CalloutStyle callout_style;
    gboolean callout_fill;
    gdouble callout_fill_alpha;
    gboolean callout_shadow;
    gchar *text;
    Gtk4TextStyle text_style;
    gdouble text_size;
    gdouble text_stroke_width;
    gboolean text_fill_enabled;
    gboolean text_stroke_enabled;
    gboolean text_shadow_enabled;
    gdouble text_fill_opacity;
    gdouble stamp_radius;
} Gtk4Mark;

typedef struct {
    gchar *name;
    gchar *path;
    gint64 mtime;
} Gtk4ImageEntry;

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkCssProvider *css_provider;
    GtkWidget *notebook;
    GtkWidget *global_status;
    gint screenshots_page;

    gchar *launch_dir;
    gchar *shots_dir;
    gchar *editor_image_path;

    GtkWidget *status_label;
    GtkWidget *selected_label;
    GtkWidget *paths_view;
    GtkWidget *search_entry;
    GtkWidget *thumb_flow;
    GtkWidget *thumb_size_scale;
    GtkWidget *thumb_preset_dropdown;
    GtkWidget *editor_area;
    GtkWidget *editor_status;
    GtkWidget *editor_path;

    GtkWidget *tool_select_btn;
    GtkWidget *tool_arrow_btn;
    GtkWidget *tool_line_btn;
    GtkWidget *tool_rect_btn;
    GtkWidget *tool_callout_btn;
    GtkWidget *tool_text_btn;
    GtkWidget *tool_stamp_btn;
    GtkWidget *editor_stroke_scale;
    GtkWidget *editor_props_stack;
    GtkWidget *editor_props_title;
    GtkWidget *editor_styles_scroller;
    GtkWidget *editor_browser_box;
    GtkWidget *editor_split;
    GtkWidget *editor_studio_shell;
    GtkWidget *editor_dock;
    GtkWidget *editor_main_box;
    GtkWidget *editor_toggle_styles_btn;
    GtkWidget *editor_toggle_thumbs_btn;
    GtkWidget *editor_dock_toggle_btn;
    GtkWidget *arrow_style_combo;
    GtkWidget *arrow_head_scale;
    GtkWidget *arrow_angle_scale;
    GtkWidget *arrow_shadow_check;
    GtkWidget *arrow_shadow_offset_scale;
    GtkWidget *rect_round_check;
    GtkWidget *rect_fill_check;
    GtkWidget *rect_fill_scale;
    GtkWidget *rect_shadow_check;
    GtkWidget *callout_style_combo;
    GtkWidget *callout_fill_check;
    GtkWidget *callout_fill_scale;
    GtkWidget *callout_shadow_check;
    GtkWidget *text_entry;
    GtkWidget *text_apply_btn;
    GtkWidget *font_style_combo;
    GtkWidget *font_size_scale;
    GtkWidget *text_stroke_scale;
    GtkWidget *text_fill_check;
    GtkWidget *text_stroke_check;
    GtkWidget *text_shadow_check;
    GtkWidget *stamp_combo;
    GtkWidget *auto_step_check;
    GtkWidget *link_step_check;
    GtkWidget *step_label;

    GtkWidget *night_status;
    GtkWidget *night_scale;
    GtkWidget *night_auto_switch;
    GtkWidget *audio_status;
    GtkWidget *audio_scale;

    gboolean tool_syncing;
    Gtk4Tool active_tool;

    GdkPixbuf *editor_pixbuf;
    GPtrArray *marks;

    gdouble draw_scale;
    gdouble draw_offset_x;
    gdouble draw_offset_y;
    gdouble zoom_factor;
    gboolean dragging;
    Gtk4MarkKind drag_kind;
    gdouble drag_start_x;
    gdouble drag_start_y;
    gdouble drag_cur_x;
    gdouble drag_cur_y;
    GdkRGBA editor_color;
    gdouble editor_stroke_width;
    Gtk4ArrowStyle editor_arrow_style;
    gdouble editor_arrow_head_size;
    gdouble editor_arrow_head_angle_deg;
    gboolean editor_arrow_shadow;
    gdouble editor_arrow_shadow_offset;
    gboolean editor_rect_rounded;
    gboolean editor_rect_fill;
    gdouble editor_rect_fill_alpha;
    gboolean editor_rect_shadow;
    Gtk4CalloutStyle editor_callout_style;
    gboolean editor_callout_fill;
    gdouble editor_callout_fill_alpha;
    gboolean editor_callout_shadow;
    Gtk4TextStyle editor_text_style;
    gdouble editor_text_size;
    gdouble editor_text_stroke_width;
    gboolean editor_text_fill_enabled;
    gboolean editor_text_stroke_enabled;
    gboolean editor_text_shadow_enabled;
    gdouble editor_text_fill_opacity;
    gdouble editor_stamp_radius;
    gboolean editor_auto_step;
    gboolean editor_link_steps;
    gint editor_next_step;
    gint thumb_preview_width;
    gint thumb_preview_height;
    gint split_pos_saved;
    gint studio_split_pos_saved;
    gint studio_dock_width_saved;
    gboolean thumb_ui_syncing;
    guint ui_state_save_source_id;
    gboolean editor_styles_visible;
    gboolean editor_thumbs_visible;
    gboolean editor_dock_right;
    gchar *ui_state_file;
} Gtk4State;

static void gtk4_editor_load_image(Gtk4State *state, const gchar *path);
static void gtk4_reload(Gtk4State *state);
static void gtk4_set_active_tool(Gtk4State *state, Gtk4Tool tool);
static gchar *gtk4_get_trimmed_entry_text(GtkWidget *entry);
static void gtk4_apply_current_text_style_to_mark(Gtk4State *state, Gtk4Mark *mark);
static void gtk4_update_step_label(Gtk4State *state);
static const gchar *gtk4_dropdown_selected_id(GtkWidget *dropdown,
                                              const gchar *const *ids,
                                              guint count);
static void gtk4_ui_state_schedule_save(Gtk4State *state);

static gboolean gtk4_ends_with_image_ext(const gchar *name) {
    if (!name) {
        return FALSE;
    }
    return g_str_has_suffix(name, ".png") ||
           g_str_has_suffix(name, ".jpg") ||
           g_str_has_suffix(name, ".jpeg") ||
           g_str_has_suffix(name, ".webp") ||
           g_str_has_suffix(name, ".svg") ||
           g_str_has_suffix(name, ".PNG") ||
           g_str_has_suffix(name, ".JPG") ||
           g_str_has_suffix(name, ".JPEG") ||
           g_str_has_suffix(name, ".WEBP") ||
           g_str_has_suffix(name, ".SVG");
}

static void gtk4_free_mark(gpointer data) {
    Gtk4Mark *mark = data;
    if (!mark) {
        return;
    }
    g_free(mark->text);
    g_free(mark);
}

static void gtk4_free_image_entry(gpointer data) {
    Gtk4ImageEntry *entry = data;
    if (!entry) {
        return;
    }
    g_free(entry->name);
    g_free(entry->path);
    g_free(entry);
}

static gint gtk4_compare_entries_desc(gconstpointer a, gconstpointer b) {
    const Gtk4ImageEntry *ea = *(Gtk4ImageEntry * const *)a;
    const Gtk4ImageEntry *eb = *(Gtk4ImageEntry * const *)b;
    if (ea->mtime == eb->mtime) {
        return g_ascii_strcasecmp(ea->name, eb->name);
    }
    return (ea->mtime < eb->mtime) ? 1 : -1;
}

static gboolean gtk4_name_matches_query(const gchar *name, const gchar *query) {
    gchar *q = NULL;
    gchar *n = NULL;
    gboolean match = TRUE;

    if (!query || !*query) {
        return TRUE;
    }
    if (!name) {
        return FALSE;
    }

    q = g_utf8_casefold(query, -1);
    n = g_utf8_casefold(name, -1);
    if (q && *q) {
        match = (n && strstr(n, q) != NULL);
    }
    g_free(q);
    g_free(n);
    return match;
}

static void gtk4_set_status(GtkWidget *label, const gchar *text, const gchar *fallback) {
    gchar *copy = NULL;
    const gchar *value = fallback ? fallback : "";

    if (!label) {
        return;
    }

    if (text) {
        copy = g_strdup(text);
        g_strstrip(copy);
        if (copy[0] != '\0') {
            value = copy;
        }
    }
    gtk_label_set_text(GTK_LABEL(label), value);
    g_free(copy);
}

static gchar *gtk4_repo_shell(Gtk4State *state, const gchar *snippet) {
    gchar *quoted = NULL;
    gchar *cmd = NULL;
    if (!state || !snippet || !*snippet) {
        return NULL;
    }
    quoted = g_shell_quote(state->launch_dir ? state->launch_dir : ".");
    cmd = g_strdup_printf("bash -lc \"cd %s && %s\"", quoted, snippet);
    g_free(quoted);
    return cmd;
}

static gchar *gtk4_default_ui_state_path(void) {
    gchar *dir = g_build_filename(g_get_user_config_dir(), "linuxutilities", NULL);
    gchar *path = g_build_filename(dir, "control_center_gtk4.ini", NULL);
    g_free(dir);
    return path;
}

static gint gtk4_thumb_width_for_preset(guint preset) {
    switch (preset) {
        case 0:
            return 210;
        case 2:
            return 340;
        case 1:
        default:
            return 270;
    }
}

static guint gtk4_thumb_preset_for_width(gint width) {
    if (width <= 235) {
        return 0;
    }
    if (width >= 315) {
        return 2;
    }
    return 1;
}

static gint gtk4_clamp_editor_split_position(Gtk4State *state, gint pos) {
    gint height = 820;
    const gint min_top = 240;
    const gint min_bottom = 190;
    gint max_top = 0;
    if (state && state->editor_split) {
        gint measured = gtk_widget_get_height(state->editor_split);
        if (measured > 0) {
            height = measured;
        }
    }
    max_top = MAX(min_top, height - min_bottom);
    return CLAMP(pos, min_top, max_top);
}

static gint gtk4_clamp_studio_split_position(Gtk4State *state, gint pos) {
    gint width = 1260;
    const gint min_main = 360;
    const gint min_dock = 250;
    gint max_main = 0;
    if (state && state->editor_studio_shell) {
        gint measured = gtk_widget_get_width(state->editor_studio_shell);
        if (measured > 0) {
            width = measured;
        }
    }
    max_main = MAX(min_main, width - min_dock);
    return CLAMP(pos, min_main, max_main);
}

static gint gtk4_clamp_studio_dock_width(Gtk4State *state, gint dock_width) {
    gint width = 1260;
    const gint min_main = 360;
    const gint min_dock = 220;
    gint max_dock = 0;
    if (state && state->editor_studio_shell) {
        gint measured = gtk_widget_get_width(state->editor_studio_shell);
        if (measured > 0) {
            width = measured;
        }
    }
    max_dock = MAX(min_dock, width - min_main);
    return CLAMP(dock_width, min_dock, max_dock);
}

static void gtk4_ui_state_load(Gtk4State *state) {
    GKeyFile *kf = NULL;
    GError *err = NULL;
    gint thumb_w = 0;
    gint split_pos = 0;
    gint studio_pos = 0;
    gint dock_w = 0;
    gboolean dock_right = TRUE;
    gboolean thumbs_visible = TRUE;
    gboolean styles_visible = TRUE;

    if (!state || !state->ui_state_file || !g_file_test(state->ui_state_file, G_FILE_TEST_EXISTS)) {
        return;
    }
    kf = g_key_file_new();
    if (!g_key_file_load_from_file(kf, state->ui_state_file, G_KEY_FILE_NONE, &err)) {
        g_clear_error(&err);
        g_key_file_unref(kf);
        return;
    }
    thumb_w = g_key_file_get_integer(kf, "screenshots", "thumb_width", NULL);
    split_pos = g_key_file_get_integer(kf, "screenshots", "split_position", NULL);
    studio_pos = g_key_file_get_integer(kf, "screenshots", "studio_split_position", NULL);
    dock_w = g_key_file_get_integer(kf, "screenshots", "studio_dock_width", NULL);
    err = NULL;
    dock_right = g_key_file_get_boolean(kf, "screenshots", "dock_right", &err);
    if (err) {
        g_clear_error(&err);
        dock_right = TRUE;
    }
    err = NULL;
    thumbs_visible = g_key_file_get_boolean(kf, "screenshots", "thumbs_visible", &err);
    if (err) {
        g_clear_error(&err);
        thumbs_visible = TRUE;
    }
    err = NULL;
    styles_visible = g_key_file_get_boolean(kf, "screenshots", "styles_visible", &err);
    if (err) {
        g_clear_error(&err);
        styles_visible = TRUE;
    }
    if (thumb_w > 0) {
        state->thumb_preview_width = CLAMP(thumb_w, 170, 440);
        state->thumb_preview_height = CLAMP((gint)lrint((gdouble)state->thumb_preview_width * 0.62), 110, 300);
    }
    if (split_pos > 0) {
        state->split_pos_saved = gtk4_clamp_editor_split_position(state, split_pos);
    }
    if (studio_pos > 0) {
        state->studio_split_pos_saved = gtk4_clamp_studio_split_position(state, studio_pos);
    }
    if (dock_w > 0) {
        state->studio_dock_width_saved = gtk4_clamp_studio_dock_width(state, dock_w);
    }
    state->editor_dock_right = dock_right;
    state->editor_thumbs_visible = thumbs_visible;
    state->editor_styles_visible = styles_visible;
    g_key_file_unref(kf);
}

static void gtk4_ui_state_save(Gtk4State *state) {
    GKeyFile *kf = NULL;
    gchar *data = NULL;
    gsize data_len = 0;
    gchar *dir = NULL;
    gint split_pos = 0;
    gint studio_pos = 0;

    if (!state || !state->ui_state_file) {
        return;
    }
    if (state->editor_split) {
        split_pos = gtk_paned_get_position(GTK_PANED(state->editor_split));
        if (split_pos > 0) {
            state->split_pos_saved = gtk4_clamp_editor_split_position(state, split_pos);
        }
    }
    if (state->editor_studio_shell) {
        studio_pos = gtk_paned_get_position(GTK_PANED(state->editor_studio_shell));
        if (studio_pos > 0) {
            if (state->editor_dock_right) {
                state->studio_split_pos_saved = gtk4_clamp_studio_split_position(state, studio_pos);
            } else {
                state->studio_dock_width_saved = gtk4_clamp_studio_dock_width(state, studio_pos);
            }
        }
    }
    state->split_pos_saved = gtk4_clamp_editor_split_position(state, state->split_pos_saved > 0 ? state->split_pos_saved : 430);
    state->studio_split_pos_saved = gtk4_clamp_studio_split_position(state, state->studio_split_pos_saved > 0 ? state->studio_split_pos_saved : 840);
    state->studio_dock_width_saved = gtk4_clamp_studio_dock_width(
        state,
        state->studio_dock_width_saved > 0 ? state->studio_dock_width_saved : 280);
    kf = g_key_file_new();
    g_key_file_set_integer(kf,
                           "screenshots",
                           "thumb_width",
                           CLAMP(state->thumb_preview_width > 0 ? state->thumb_preview_width : 270, 170, 440));
    g_key_file_set_integer(kf,
                           "screenshots",
                           "split_position",
                           state->split_pos_saved);
    g_key_file_set_integer(kf,
                           "screenshots",
                           "studio_split_position",
                           state->studio_split_pos_saved);
    g_key_file_set_integer(kf,
                           "screenshots",
                           "studio_dock_width",
                           state->studio_dock_width_saved);
    g_key_file_set_boolean(kf,
                           "screenshots",
                           "dock_right",
                           state->editor_dock_right);
    g_key_file_set_boolean(kf,
                           "screenshots",
                           "thumbs_visible",
                           state->editor_thumbs_visible);
    g_key_file_set_boolean(kf,
                           "screenshots",
                           "styles_visible",
                           state->editor_styles_visible);

    data = g_key_file_to_data(kf, &data_len, NULL);
    dir = g_path_get_dirname(state->ui_state_file);
    g_mkdir_with_parents(dir, 0755);
    if (data && data_len > 0) {
        g_file_set_contents(state->ui_state_file, data, (gssize)data_len, NULL);
    }
    g_free(dir);
    g_free(data);
    g_key_file_unref(kf);
}

static gboolean gtk4_ui_state_save_cb(gpointer data) {
    Gtk4State *state = data;
    if (!state) {
        return G_SOURCE_REMOVE;
    }
    state->ui_state_save_source_id = 0;
    gtk4_ui_state_save(state);
    return G_SOURCE_REMOVE;
}

static void gtk4_ui_state_schedule_save(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->ui_state_save_source_id != 0) {
        g_source_remove(state->ui_state_save_source_id);
        state->ui_state_save_source_id = 0;
    }
    state->ui_state_save_source_id = g_timeout_add(260, gtk4_ui_state_save_cb, state);
}

static void gtk4_apply_theme(gboolean dark) {
    GtkSettings *settings = gtk_settings_get_default();
    if (settings) {
        g_object_set(settings, "gtk-application-prefer-dark-theme", dark, NULL);
    }
}

static void gtk4_apply_css(Gtk4State *state) {
    static const gchar *css =
        ".lcu-root { background: #0a1324; color: #d8e8ff; }\n"
        ".lcu-header { background: #101d31; border-bottom: 1px solid rgba(130,170,220,0.20); padding: 10px; }\n"
        ".lcu-title { font-weight: 700; font-size: 18px; }\n"
        ".lcu-panel { background: rgba(20,34,56,0.74); border: 1px solid rgba(120,170,230,0.20); border-radius: 10px; padding: 10px; }\n"
        ".lcu-surface { background: #16243a; border: 1px solid rgba(120,170,230,0.25); border-radius: 8px; }\n"
        ".dim-label { color: #9bb6d5; }\n"
        ".shortcut-content { background: transparent; }\n"
        ".shortcut-h1 { color: #d6ebff; font-weight: 700; font-size: 17px; margin-top: 2px; }\n"
        ".shortcut-h2 { color: #9fc9ff; font-weight: 700; font-size: 15px; margin-top: 8px; }\n"
        ".shortcut-h3 { color: #7fb8f8; font-weight: 700; font-size: 14px; margin-top: 4px; }\n"
        ".shortcut-subtitle { color: #bdd3ea; font-weight: 700; margin-top: 2px; }\n"
        ".shortcut-note { color: #d8e8ff; }\n"
        ".shortcut-bullet { color: #d8e8ff; margin-left: 6px; }\n"
        ".shortcut-table { border: 1px solid rgba(120,170,230,0.25); border-radius: 8px; padding: 6px; background: rgba(17,30,48,0.68); }\n"
        ".shortcut-row { padding: 3px 6px; border-radius: 6px; }\n"
        ".shortcut-row-header { border-bottom: 1px solid rgba(120,170,230,0.30); padding-bottom: 6px; margin-bottom: 2px; }\n"
        ".shortcut-key-chip { background: #284f7a; color: #f5fbff; border-radius: 6px; padding: 2px 8px; font-family: monospace; font-weight: 700; }\n"
        ".shortcut-key-head { color: #9fc9ff; font-weight: 700; }\n"
        ".shortcut-action { color: #d8e8ff; }\n"
        ".shortcut-code-block { background: #13253c; color: #d1e6ff; border: 1px solid rgba(120,170,230,0.28); border-radius: 8px; padding: 8px; font-family: monospace; }\n"
        ".thumb-card { background: rgba(15,30,52,0.86); border: 1px solid rgba(120,170,230,0.28); border-radius: 10px; padding: 6px; }\n"
        ".thumb-card:hover { border-color: rgba(140,205,255,0.84); background: rgba(24,47,77,0.90); }\n"
        ".thumb-name { color: #c8def5; font-size: 12px; margin-top: 4px; }\n"
        "button.utility-btn { border: 1px solid rgba(120,170,230,0.24); background: rgba(26,40,61,0.78); border-radius: 10px; padding: 8px; }\n"
        "button.utility-btn:hover { border-color: rgba(125,198,255,0.76); background: rgba(38,69,105,0.88); }\n"
        "button.utility-btn:focus-visible { border-color: rgba(120,195,255,0.98); box-shadow: 0 0 0 2px rgba(72,138,214,0.34); }\n"
        "button.utility-btn:active { border-color: rgba(120,195,255,1.0); background: rgba(46,95,144,0.98); }\n"
        "button.utility-btn.utility-btn-active { border-color: rgba(115,214,255,1.0); background: rgba(45,108,166,0.95); }\n"
        ".utility-title { color: #e6f3ff; font-weight: 700; }\n"
        ".utility-subtitle { color: #9fb9d4; }\n"
        "paned > separator { background: rgba(100,150,220,0.35); min-width: 8px; min-height: 8px; }\n"
        "textview { font-size: 13px; line-height: 1.35; }\n";
    if (!state) {
        return;
    }
    if (!state->css_provider) {
        state->css_provider = gtk_css_provider_new();
        gtk_css_provider_load_from_string(state->css_provider, css);
    }
    GdkDisplay *display = gdk_display_get_default();
    if (display) {
        gtk_style_context_add_provider_for_display(display,
                                                   GTK_STYLE_PROVIDER(state->css_provider),
                                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
}

static void gtk4_set_global_status(Gtk4State *state, const gchar *text) {
    if (!state || !state->global_status) {
        return;
    }
    gtk_label_set_text(GTK_LABEL(state->global_status), text ? text : "");
}

static gboolean gtk4_spawn_with_feedback(Gtk4State *state, const gchar *command, const gchar *success_text) {
    GError *error = NULL;
    if (!command || *command == '\0') {
        gtk4_set_global_status(state, "No command configured.");
        return FALSE;
    }
    if (!g_spawn_command_line_async(command, &error)) {
        gchar *msg = g_strdup_printf("Launch failed: %s", error ? error->message : "unknown");
        gtk4_set_global_status(state, msg);
        g_warning("%s (cmd=%s)", msg, command);
        g_free(msg);
        g_clear_error(&error);
        return FALSE;
    }
    gtk4_set_global_status(state, success_text ? success_text : "Command launched.");
    return TRUE;
}

static void gtk4_update_paths_text(Gtk4State *state, GPtrArray *paths) {
    GString *joined = g_string_new(NULL);
    GtkTextBuffer *buffer = NULL;

    if (!state || !state->paths_view) {
        g_string_free(joined, TRUE);
        return;
    }
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->paths_view));
    for (guint i = 0; i < paths->len; i += 1) {
        const gchar *path = g_ptr_array_index(paths, i);
        g_string_append(joined, path ? path : "");
        if (i + 1 < paths->len) {
            g_string_append_c(joined, '\n');
        }
    }
    gtk_text_buffer_set_text(buffer, joined->str, -1);
    g_string_free(joined, TRUE);
}

static GPtrArray *gtk4_selected_paths(Gtk4State *state) {
    GPtrArray *paths = g_ptr_array_new_with_free_func(g_free);
    GList *selected = NULL;

    if (!state || !state->thumb_flow) {
        return paths;
    }

    selected = gtk_flow_box_get_selected_children(GTK_FLOW_BOX(state->thumb_flow));
    if (!selected) {
        return paths;
    }

    for (GList *node = selected; node != NULL; node = node->next) {
        GtkFlowBoxChild *child = node->data;
        GtkWidget *row = NULL;
        const gchar *path = NULL;
        if (!child) {
            continue;
        }
        row = gtk_flow_box_child_get_child(child);
        path = row ? g_object_get_data(G_OBJECT(row), "shot-path") : NULL;
        if (path) {
            g_ptr_array_add(paths, g_strdup(path));
        }
    }
    g_list_free(selected);

    return paths;
}

static void gtk4_update_selection_ui(Gtk4State *state) {
    GPtrArray *paths = gtk4_selected_paths(state);
    gchar *msg = NULL;

    if (!state || !state->selected_label) {
        if (paths) {
            g_ptr_array_free(paths, TRUE);
        }
        return;
    }

    if (paths->len == 0) {
        gtk_label_set_text(GTK_LABEL(state->selected_label),
                           "No images selected. Tip: Ctrl+A selects all, Delete removes selected.");
    } else {
        msg = g_strdup_printf("%u image(s) selected.", paths->len);
        gtk_label_set_text(GTK_LABEL(state->selected_label), msg);
        g_free(msg);
    }
    gtk4_update_paths_text(state, paths);
    g_ptr_array_free(paths, TRUE);
}

static gboolean gtk4_widget_to_image(Gtk4State *state, gdouble wx, gdouble wy, gdouble *ix, gdouble *iy, gboolean clamp_pos) {
    gint img_w = 0;
    gint img_h = 0;
    gdouble x = 0.0;
    gdouble y = 0.0;
    if (!state || !state->editor_pixbuf || state->draw_scale <= 0.0) {
        return FALSE;
    }

    img_w = gdk_pixbuf_get_width(state->editor_pixbuf);
    img_h = gdk_pixbuf_get_height(state->editor_pixbuf);
    x = (wx - state->draw_offset_x) / state->draw_scale;
    y = (wy - state->draw_offset_y) / state->draw_scale;

    if (!clamp_pos) {
        if (x < 0.0 || y < 0.0 || x > (gdouble)(img_w - 1) || y > (gdouble)(img_h - 1)) {
            return FALSE;
        }
    } else {
        x = CLAMP(x, 0.0, (gdouble)(img_w - 1));
        y = CLAMP(y, 0.0, (gdouble)(img_h - 1));
    }

    if (ix) {
        *ix = x;
    }
    if (iy) {
        *iy = y;
    }
    return TRUE;
}

static void gtk4_draw_rounded_rect(cairo_t *cr, gdouble x, gdouble y, gdouble w, gdouble h, gdouble r) {
    gdouble radius = MIN(r, MIN(w * 0.5, h * 0.5));
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius, y + radius, radius, -G_PI_2, 0);
    cairo_arc(cr, x + w - radius, y + h - radius, radius, 0, G_PI_2);
    cairo_arc(cr, x + radius, y + h - radius, radius, G_PI_2, G_PI);
    cairo_arc(cr, x + radius, y + radius, radius, G_PI, 3 * G_PI_2);
    cairo_close_path(cr);
}

static void gtk4_draw_arrow_head(cairo_t *cr,
                                 gdouble tip_x,
                                 gdouble tip_y,
                                 gdouble dir_x,
                                 gdouble dir_y,
                                 gdouble head_len,
                                 gdouble head_angle) {
    cairo_move_to(cr, tip_x, tip_y);
    cairo_line_to(cr,
                  tip_x - head_len * (dir_x * cos(head_angle) - dir_y * sin(head_angle)),
                  tip_y - head_len * (dir_y * cos(head_angle) + dir_x * sin(head_angle)));
    cairo_move_to(cr, tip_x, tip_y);
    cairo_line_to(cr,
                  tip_x - head_len * (dir_x * cos(head_angle) + dir_y * sin(head_angle)),
                  tip_y - head_len * (dir_y * cos(head_angle) - dir_x * sin(head_angle)));
}

static void gtk4_add_callout_path(cairo_t *cr,
                                  gdouble x,
                                  gdouble y,
                                  gdouble w,
                                  gdouble h,
                                  Gtk4CalloutStyle style) {
    gboolean with_tail = (style != GTK4_CALLOUT_STYLE_HIGHLIGHT);
    gdouble radius = (style == GTK4_CALLOUT_STYLE_HIGHLIGHT) ? (h * 0.48) : 12.0;
    gdouble tail_ratio = 0.25;
    gdouble tail_w = MAX(14.0, w * 0.11);
    gdouble tail_h = MAX(10.0, h * 0.16);
    gdouble tail_x = 0.0;

    if (style == GTK4_CALLOUT_STYLE_LEFT) {
        tail_ratio = 0.16;
    } else if (style == GTK4_CALLOUT_STYLE_RIGHT) {
        tail_ratio = 0.80;
    } else {
        tail_ratio = 0.25;
    }

    gtk4_draw_rounded_rect(cr, x, y, w, h, radius);
    if (!with_tail) {
        return;
    }
    tail_x = x + w * tail_ratio;
    cairo_move_to(cr, tail_x - tail_w * 0.5, y + h);
    cairo_line_to(cr, tail_x + tail_w * 0.5, y + h);
    cairo_line_to(cr, tail_x, y + h + tail_h);
    cairo_close_path(cr);
}

static void gtk4_text_style_to_cairo(Gtk4TextStyle style,
                                     cairo_font_slant_t *slant_out,
                                     cairo_font_weight_t *weight_out) {
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_BOLD;
    switch (style) {
        case GTK4_TEXT_STYLE_NORMAL:
            slant = CAIRO_FONT_SLANT_NORMAL;
            weight = CAIRO_FONT_WEIGHT_NORMAL;
            break;
        case GTK4_TEXT_STYLE_ITALIC:
            slant = CAIRO_FONT_SLANT_ITALIC;
            weight = CAIRO_FONT_WEIGHT_NORMAL;
            break;
        case GTK4_TEXT_STYLE_BOLD_ITALIC:
            slant = CAIRO_FONT_SLANT_ITALIC;
            weight = CAIRO_FONT_WEIGHT_BOLD;
            break;
        case GTK4_TEXT_STYLE_BOLD:
        default:
            slant = CAIRO_FONT_SLANT_NORMAL;
            weight = CAIRO_FONT_WEIGHT_BOLD;
            break;
    }
    if (slant_out) {
        *slant_out = slant;
    }
    if (weight_out) {
        *weight_out = weight;
    }
}

static void gtk4_draw_styled_text(cairo_t *cr, const Gtk4Mark *mark, const gchar *text, gdouble x, gdouble y) {
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_BOLD;
    gdouble text_size = 24.0;
    gdouble stroke_width = 2.0;
    gdouble fill_alpha = 1.0;
    gboolean fill_enabled = TRUE;
    gboolean stroke_enabled = FALSE;
    gboolean shadow_enabled = TRUE;
    if (!cr || !mark || !text || !*text) {
        return;
    }
    gtk4_text_style_to_cairo(mark->text_style, &slant, &weight);
    text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;
    stroke_width = mark->text_stroke_width > 0.0 ? mark->text_stroke_width : 2.0;
    fill_alpha = mark->text_fill_opacity > 0.0 ? mark->text_fill_opacity : 1.0;
    fill_enabled = mark->text_fill_enabled || !mark->text_stroke_enabled;
    stroke_enabled = mark->text_stroke_enabled;
    shadow_enabled = mark->text_shadow_enabled;

    cairo_select_font_face(cr, "Sans", slant, weight);
    cairo_set_font_size(cr, text_size);

    if (shadow_enabled) {
        cairo_move_to(cr, x + 2.0, y + 2.0);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
        cairo_show_text(cr, text);
    }
    if (stroke_enabled) {
        cairo_move_to(cr, x, y);
        cairo_text_path(cr, text);
        cairo_set_line_width(cr, stroke_width);
        cairo_set_source_rgba(cr, 0.02, 0.03, 0.05, 0.9);
        cairo_stroke(cr);
    }
    if (fill_enabled) {
        cairo_move_to(cr, x, y);
        cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, fill_alpha);
        cairo_show_text(cr, text);
    }
}

static void gtk4_draw_stamp_mark(cairo_t *cr, const Gtk4Mark *mark) {
    gdouble radius = 16.0;
    const gchar *text = NULL;
    if (!cr || !mark) {
        return;
    }
    radius = mark->stamp_radius > 0.0 ? mark->stamp_radius : 16.0;
    text = (mark->text && *mark->text) ? mark->text : "1";

    cairo_save(cr);
    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 0.96);
    cairo_arc(cr, mark->x1, mark->y1, radius, 0.0, 2.0 * G_PI);
    cairo_fill_preserve(cr);
    cairo_set_line_width(cr, 2.2);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.95);
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, MAX(12.0, radius * 1.25));
    cairo_text_extents_t ex = {0};
    cairo_text_extents(cr, text, &ex);
    cairo_move_to(cr,
                  mark->x1 - (ex.width * 0.5 + ex.x_bearing),
                  mark->y1 - (ex.height * 0.5 + ex.y_bearing));
    cairo_set_source_rgba(cr, 0.98, 0.99, 1.0, 1.0);
    cairo_show_text(cr, text);
    cairo_restore(cr);
}

static void gtk4_draw_mark(cairo_t *cr, const Gtk4Mark *mark) {
    if (!mark) {
        return;
    }
    cairo_save(cr);
    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, mark->color.alpha);
    cairo_set_line_width(cr, mark->stroke_width > 0.0 ? mark->stroke_width : 4.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

    if (mark->kind == GTK4_MARK_STAMP) {
        gtk4_draw_stamp_mark(cr, mark);
        cairo_restore(cr);
        return;
    }
    if (mark->kind == GTK4_MARK_TEXT) {
        const gchar *text = (mark->text && *mark->text) ? mark->text : "Note";
        gtk4_draw_styled_text(cr, mark, text, mark->x1, mark->y1);
        cairo_restore(cr);
        return;
    }
    if (mark->kind == GTK4_MARK_ARROW || mark->kind == GTK4_MARK_LINE) {
        gdouble vx = mark->x2 - mark->x1;
        gdouble vy = mark->y2 - mark->y1;
        gdouble len = sqrt(vx * vx + vy * vy);
        gdouble nx = 0.0;
        gdouble ny = 0.0;
        gdouble head = mark->arrow_head_size > 0.0 ? mark->arrow_head_size : 16.0;
        gdouble spread = ((mark->arrow_head_angle_deg > 0.0 ? mark->arrow_head_angle_deg : 26.0) * G_PI) / 180.0;
        gdouble dash[2] = {0.0, 0.0};
        gdouble lw = mark->stroke_width > 0.0 ? mark->stroke_width : 4.0;
        gboolean dashed = FALSE;
        gboolean dotted = FALSE;

        if (mark->arrow_style == GTK4_ARROW_STYLE_BOLD) {
            lw *= 1.7;
        } else if (mark->arrow_style == GTK4_ARROW_STYLE_DASHED) {
            dashed = TRUE;
            dash[0] = 14.0;
            dash[1] = 8.0;
        } else if (mark->arrow_style == GTK4_ARROW_STYLE_DOTTED) {
            dotted = TRUE;
            dash[0] = 1.5;
            dash[1] = 7.0;
        }

        cairo_set_line_width(cr, lw);
        if (dashed || dotted) {
            cairo_set_dash(cr, dash, 2, 0.0);
            if (dotted) {
                cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
            }
        } else {
            cairo_set_dash(cr, NULL, 0, 0.0);
        }

        if (mark->kind == GTK4_MARK_ARROW && mark->arrow_shadow) {
            gdouble off = mark->arrow_shadow_offset > 0.0 ? mark->arrow_shadow_offset : 3.0;
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.34);
            cairo_move_to(cr, mark->x1 + off, mark->y1 + off);
            cairo_line_to(cr, mark->x2 + off, mark->y2 + off);
            if (len > 1e-6) {
                nx = vx / len;
                ny = vy / len;
                gtk4_draw_arrow_head(cr, mark->x2 + off, mark->y2 + off, nx, ny, head, spread);
                if (mark->arrow_style == GTK4_ARROW_STYLE_DOUBLE) {
                    gtk4_draw_arrow_head(cr, mark->x1 + off, mark->y1 + off, -nx, -ny, head, spread);
                }
            }
            cairo_stroke(cr);
            cairo_restore(cr);
        }

        cairo_move_to(cr, mark->x1, mark->y1);
        cairo_line_to(cr, mark->x2, mark->y2);
        if (mark->kind == GTK4_MARK_ARROW && len > 1e-6) {
            nx = vx / len;
            ny = vy / len;
            gtk4_draw_arrow_head(cr, mark->x2, mark->y2, nx, ny, head, spread);
            if (mark->arrow_style == GTK4_ARROW_STYLE_DOUBLE) {
                gtk4_draw_arrow_head(cr, mark->x1, mark->y1, -nx, -ny, head, spread);
            }
        }
        cairo_stroke(cr);
        cairo_set_dash(cr, NULL, 0, 0.0);
    } else if (mark->kind == GTK4_MARK_RECT) {
        gdouble rx = MIN(mark->x1, mark->x2);
        gdouble ry = MIN(mark->y1, mark->y2);
        gdouble rw = fabs(mark->x2 - mark->x1);
        gdouble rh = fabs(mark->y2 - mark->y1);
        if (rw <= 1.0 || rh <= 1.0) {
            cairo_restore(cr);
            return;
        }
        if (mark->rect_shadow) {
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.30);
            if (mark->rect_rounded) {
                gtk4_draw_rounded_rect(cr, rx + 4.0, ry + 4.0, rw, rh, 12.0);
            } else {
                cairo_rectangle(cr, rx + 4.0, ry + 4.0, rw, rh);
            }
            cairo_fill(cr);
        }
        if (mark->rect_fill) {
            gdouble alpha = CLAMP(mark->rect_fill_alpha, 0.02, 0.95);
            cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, alpha);
            if (mark->rect_rounded) {
                gtk4_draw_rounded_rect(cr, rx, ry, rw, rh, 12.0);
            } else {
                cairo_rectangle(cr, rx, ry, rw, rh);
            }
            cairo_fill_preserve(cr);
            cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, mark->color.alpha);
            cairo_stroke(cr);
            cairo_restore(cr);
            return;
        }
        if (mark->rect_rounded) {
            gtk4_draw_rounded_rect(cr, rx, ry, rw, rh, 12.0);
        } else {
            cairo_rectangle(cr, rx, ry, rw, rh);
        }
        cairo_stroke(cr);
    } else {
        gdouble rx = MIN(mark->x1, mark->x2);
        gdouble ry = MIN(mark->y1, mark->y2);
        gdouble rw = fabs(mark->x2 - mark->x1);
        gdouble rh = fabs(mark->y2 - mark->y1);
        gdouble text_x = 0.0;
        gdouble text_y = 0.0;
        gboolean fill = mark->callout_fill || mark->callout_style == GTK4_CALLOUT_STYLE_HIGHLIGHT;
        gboolean shadow = mark->callout_shadow || mark->callout_style == GTK4_CALLOUT_STYLE_SHADOW;
        gdouble fill_alpha = mark->callout_style == GTK4_CALLOUT_STYLE_HIGHLIGHT
                                 ? MAX(mark->callout_fill_alpha, 0.34)
                                 : CLAMP(mark->callout_fill_alpha, 0.02, 0.95);
        const gchar *text = (mark->text && *mark->text) ? mark->text : "Callout";
        if (rw <= 1.0 || rh <= 1.0) {
            cairo_restore(cr);
            return;
        }
        if (shadow) {
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.32);
            gtk4_add_callout_path(cr, rx + 4.0, ry + 4.0, rw, rh, mark->callout_style);
            cairo_fill(cr);
        }
        if (fill) {
            cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, fill_alpha);
            gtk4_add_callout_path(cr, rx, ry, rw, rh, mark->callout_style);
            cairo_fill_preserve(cr);
            cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, mark->color.alpha);
            cairo_stroke(cr);
            cairo_restore(cr);
            return;
        }
        gtk4_add_callout_path(cr, rx, ry, rw, rh, mark->callout_style);
        cairo_stroke(cr);
        text_x = rx + 12.0;
        text_y = ry + MIN(34.0, rh * 0.65);
        gtk4_draw_styled_text(cr, mark, text, text_x, text_y);
    }
    cairo_restore(cr);
}

static void gtk4_editor_undo_last(Gtk4State *state) {
    if (!state || !state->marks || state->marks->len == 0) {
        gtk4_set_status(state ? state->editor_status : NULL,
                        "Nothing to undo.",
                        "No annotations yet.");
        return;
    }
    g_ptr_array_set_size(state->marks, state->marks->len - 1);
    gtk4_set_status(state->editor_status, "Undo: removed last annotation.", "Undo.");
    if (state->editor_area) {
        gtk_widget_queue_draw(state->editor_area);
    }
}

static void gtk4_editor_clear_marks(Gtk4State *state) {
    if (!state || !state->marks) {
        return;
    }
    if (state->marks->len == 0) {
        gtk4_set_status(state->editor_status, "Canvas already clear.", "No annotations.");
        return;
    }
    g_ptr_array_set_size(state->marks, 0);
    gtk4_set_status(state->editor_status, "Cleared annotations.", "Canvas cleared.");
    if (state->editor_area) {
        gtk_widget_queue_draw(state->editor_area);
    }
}

static void gtk4_editor_edit_selected(Gtk4State *state) {
    GPtrArray *paths = NULL;
    if (!state) {
        return;
    }
    paths = gtk4_selected_paths(state);
    if (paths->len == 1) {
        gtk4_editor_load_image(state, g_ptr_array_index(paths, 0));
    } else if (paths->len == 0) {
        gtk4_set_status(state->editor_status, "Select one screenshot to edit.", "No selection.");
    } else {
        gtk4_set_status(state->editor_status, "Select only one screenshot for editing.", "Multiple selected.");
    }
    g_ptr_array_free(paths, TRUE);
}

static void gtk4_editor_capture_new(Gtk4State *state) {
    gchar *flameshot = g_find_program_in_path("flameshot");
    gchar *gnome_shot = g_find_program_in_path("gnome-screenshot");
    gboolean ok = FALSE;
    if (flameshot) {
        ok = gtk4_spawn_with_feedback(state, "flameshot gui", "Launching Flameshot capture...");
    } else if (gnome_shot) {
        ok = gtk4_spawn_with_feedback(state, "gnome-screenshot -a", "Launching GNOME screenshot...");
    } else {
        gtk4_set_global_status(state, "No capture tool found. Install flameshot or gnome-screenshot.");
    }
    if (ok) {
        gtk4_set_status(state->status_label, "Capture launched. Save capture, then press Refresh.", "Capture launched.");
    }
    g_free(flameshot);
    g_free(gnome_shot);
}

static gchar *gtk4_editor_make_output_path(Gtk4State *state, const gchar *suffix, const gchar *ext) {
    gchar *dir = NULL;
    gchar *base = NULL;
    gchar *stem = NULL;
    gchar *dot = NULL;
    GDateTime *now = NULL;
    gchar *stamp = NULL;
    gchar *name = NULL;
    gchar *path = NULL;

    if (!state || !state->editor_image_path || !*state->editor_image_path) {
        return NULL;
    }
    dir = g_path_get_dirname(state->editor_image_path);
    base = g_path_get_basename(state->editor_image_path);
    dot = strrchr(base, '.');
    stem = dot ? g_strndup(base, (gsize)(dot - base)) : g_strdup(base);
    now = g_date_time_new_now_local();
    stamp = g_date_time_format(now, "%Y%m%d-%H%M%S");
    name = g_strdup_printf("%s-%s-%s.%s",
                           stem ? stem : "capture",
                           suffix ? suffix : "annotated",
                           stamp ? stamp : "now",
                           ext ? ext : "png");
    path = g_build_filename(dir, name, NULL);

    g_free(dir);
    g_free(base);
    g_free(stem);
    g_free(stamp);
    g_free(name);
    if (now) {
        g_date_time_unref(now);
    }
    return path;
}

static void gtk4_editor_save_annotated(Gtk4State *state) {
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    cairo_status_t st = CAIRO_STATUS_SUCCESS;
    gchar *out = NULL;
    gint img_w = 0;
    gint img_h = 0;

    if (!state || !state->editor_pixbuf) {
        gtk4_set_status(state ? state->editor_status : NULL, "Load an image before saving.", "No image.");
        return;
    }
    out = gtk4_editor_make_output_path(state, "annotated", "png");
    if (!out) {
        gtk4_set_status(state->editor_status, "Could not build output path.", "Save failed.");
        return;
    }

    img_w = gdk_pixbuf_get_width(state->editor_pixbuf);
    img_h = gdk_pixbuf_get_height(state->editor_pixbuf);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, img_w, img_h);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, state->editor_pixbuf, 0.0, 0.0);
    cairo_paint(cr);
    if (state->marks) {
        for (guint i = 0; i < state->marks->len; i += 1) {
            gtk4_draw_mark(cr, g_ptr_array_index(state->marks, i));
        }
    }
    cairo_destroy(cr);
    st = cairo_surface_write_to_png(surface, out);
    cairo_surface_destroy(surface);

    if (st == CAIRO_STATUS_SUCCESS) {
        gchar *msg = g_strdup_printf("Saved annotated copy: %s", out);
        gtk4_set_status(state->editor_status, msg, "Saved annotated copy.");
        gtk4_set_global_status(state, msg);
        g_free(msg);
        gtk4_reload(state);
    } else {
        gchar *msg = g_strdup_printf("Save failed: %s", cairo_status_to_string(st));
        gtk4_set_status(state->editor_status, msg, "Save failed.");
        gtk4_set_global_status(state, msg);
        g_free(msg);
    }
    g_free(out);
}

static void gtk4_editor_save_svg(Gtk4State *state) {
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    cairo_status_t st = CAIRO_STATUS_SUCCESS;
    gchar *out = NULL;
    gint img_w = 0;
    gint img_h = 0;

    if (!state || !state->editor_pixbuf) {
        gtk4_set_status(state ? state->editor_status : NULL, "Load an image before saving SVG.", "No image.");
        return;
    }
    out = gtk4_editor_make_output_path(state, "annotated", "svg");
    if (!out) {
        gtk4_set_status(state->editor_status, "Could not build SVG path.", "Save failed.");
        return;
    }

    img_w = gdk_pixbuf_get_width(state->editor_pixbuf);
    img_h = gdk_pixbuf_get_height(state->editor_pixbuf);
    surface = cairo_svg_surface_create(out, (double)img_w, (double)img_h);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, state->editor_pixbuf, 0.0, 0.0);
    cairo_paint(cr);
    if (state->marks) {
        for (guint i = 0; i < state->marks->len; i += 1) {
            gtk4_draw_mark(cr, g_ptr_array_index(state->marks, i));
        }
    }
    st = cairo_status(cr);
    cairo_destroy(cr);
    cairo_surface_finish(surface);
    if (st == CAIRO_STATUS_SUCCESS) {
        st = cairo_surface_status(surface);
    }
    cairo_surface_destroy(surface);

    if (st == CAIRO_STATUS_SUCCESS) {
        gchar *msg = g_strdup_printf("Saved SVG copy: %s", out);
        gtk4_set_status(state->editor_status, msg, "Saved SVG copy.");
        gtk4_set_global_status(state, msg);
        g_free(msg);
        gtk4_reload(state);
    } else {
        gchar *msg = g_strdup_printf("SVG save failed: %s", cairo_status_to_string(st));
        gtk4_set_status(state->editor_status, msg, "SVG save failed.");
        gtk4_set_global_status(state, msg);
        g_free(msg);
    }
    g_free(out);
}

static void gtk4_set_editor_color(Gtk4State *state, const gchar *hex, const gchar *name) {
    if (!state || !hex) {
        return;
    }
    if (!gdk_rgba_parse(&state->editor_color, hex)) {
        return;
    }
    if (name && state->editor_status) {
        gchar *msg = g_strdup_printf("Annotation color: %s", name);
        gtk4_set_status(state->editor_status, msg, "Color changed.");
        g_free(msg);
    }
}

static void gtk4_clear_editor(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->editor_pixbuf) {
        g_object_unref(state->editor_pixbuf);
        state->editor_pixbuf = NULL;
    }
    g_clear_pointer(&state->editor_image_path, g_free);
    if (state->marks) {
        g_ptr_array_set_size(state->marks, 0);
    }
    if (state->editor_path) {
        gtk_label_set_text(GTK_LABEL(state->editor_path), "No image selected");
    }
    if (state->editor_area) {
        gtk_widget_queue_draw(state->editor_area);
    }
}

static void gtk4_editor_load_image(Gtk4State *state, const gchar *path) {
    GError *error = NULL;
    GdkPixbuf *pix = NULL;
    gchar *msg = NULL;

    if (!state || !path || !*path) {
        return;
    }

    pix = gdk_pixbuf_new_from_file(path, &error);
    if (!pix) {
        msg = g_strdup_printf("Failed to open image: %s", error ? error->message : "unknown");
        gtk4_set_status(state->editor_status, msg, "Failed to open image.");
        g_free(msg);
        g_clear_error(&error);
        return;
    }

    if (state->editor_pixbuf) {
        g_object_unref(state->editor_pixbuf);
    }
    state->editor_pixbuf = pix;
    g_free(state->editor_image_path);
    state->editor_image_path = g_strdup(path);
    state->zoom_factor = 1.0;
    if (state->marks) {
        g_ptr_array_set_size(state->marks, 0);
    }
    if (state->editor_path) {
        gchar *base = g_path_get_basename(path);
        gchar *label = g_strdup_printf("Editing: %s", base ? base : path);
        gtk_label_set_text(GTK_LABEL(state->editor_path), label);
        g_free(label);
        g_free(base);
    }
    gtk4_set_status(state->editor_status,
                    "Image loaded. Drag for shape tools, click for Text/Stamp.",
                    "Editor ready.");
    gtk_widget_queue_draw(state->editor_area);
}

static void gtk4_editor_draw(GtkDrawingArea *area, cairo_t *cr, gint width, gint height, gpointer user_data) {
    Gtk4State *state = user_data;
    GtkWidget *widget = GTK_WIDGET(area);

    cairo_set_source_rgb(cr, 0.10, 0.13, 0.18);
    cairo_paint(cr);

    if (!state || !state->editor_pixbuf) {
        cairo_set_source_rgba(cr, 0.85, 0.90, 0.96, 0.95);
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 19.0);
        cairo_move_to(cr, 16.0, (gdouble)height * 0.45);
        cairo_show_text(cr, "Select a screenshot to start");
        cairo_set_font_size(cr, 13.0);
        cairo_move_to(cr, 16.0, (gdouble)height * 0.45 + 26.0);
        cairo_show_text(cr, "Event controllers active: click/drag/move/ctrl+scroll");
        return;
    }

    {
        gint img_w = gdk_pixbuf_get_width(state->editor_pixbuf);
        gint img_h = gdk_pixbuf_get_height(state->editor_pixbuf);
        gdouble fit_scale = MIN((gdouble)width / (gdouble)img_w, (gdouble)height / (gdouble)img_h);
        gdouble zoom = state->zoom_factor > 0.0 ? state->zoom_factor : 1.0;
        gdouble draw_w = 0.0;
        gdouble draw_h = 0.0;

        zoom = CLAMP(zoom, 0.10, 8.0);
        state->zoom_factor = zoom;
        state->draw_scale = fit_scale * zoom;
        draw_w = (gdouble)img_w * state->draw_scale;
        draw_h = (gdouble)img_h * state->draw_scale;
        state->draw_offset_x = ((gdouble)width - draw_w) / 2.0;
        state->draw_offset_y = ((gdouble)height - draw_h) / 2.0;

        cairo_save(cr);
        cairo_translate(cr, state->draw_offset_x, state->draw_offset_y);
        cairo_scale(cr, state->draw_scale, state->draw_scale);
        gdk_cairo_set_source_pixbuf(cr, state->editor_pixbuf, 0, 0);
        cairo_rectangle(cr, 0, 0, img_w, img_h);
        cairo_fill(cr);

        if (state->marks) {
            for (guint i = 0; i < state->marks->len; i += 1) {
                const Gtk4Mark *mark = g_ptr_array_index(state->marks, i);
                gtk4_draw_mark(cr, mark);
            }
        }

        if (state->dragging &&
            (state->active_tool == GTK4_TOOL_ARROW ||
             state->active_tool == GTK4_TOOL_LINE ||
             state->active_tool == GTK4_TOOL_RECT ||
             state->active_tool == GTK4_TOOL_CALLOUT)) {
            Gtk4Mark preview = {0};
            preview.kind = GTK4_MARK_RECT;
            preview.x1 = state->drag_start_x;
            preview.y1 = state->drag_start_y;
            preview.x2 = state->drag_cur_x;
            preview.y2 = state->drag_cur_y;
            preview.color = state->editor_color;
            preview.stroke_width = state->editor_stroke_width > 0.0 ? state->editor_stroke_width : 4.0;
            preview.arrow_style = state->editor_arrow_style;
            preview.arrow_head_size = state->editor_arrow_head_size;
            preview.arrow_head_angle_deg = state->editor_arrow_head_angle_deg;
            preview.arrow_shadow = state->editor_arrow_shadow;
            preview.arrow_shadow_offset = state->editor_arrow_shadow_offset;
            preview.rect_rounded = state->editor_rect_rounded;
            preview.rect_fill = state->editor_rect_fill;
            preview.rect_fill_alpha = state->editor_rect_fill_alpha;
            preview.rect_shadow = state->editor_rect_shadow;
            preview.callout_style = state->editor_callout_style;
            preview.callout_fill = state->editor_callout_fill;
            preview.callout_fill_alpha = state->editor_callout_fill_alpha;
            preview.callout_shadow = state->editor_callout_shadow;
            preview.text_style = state->editor_text_style;
            preview.text_size = state->editor_text_size;
            preview.text_stroke_width = state->editor_text_stroke_width;
            preview.text_fill_enabled = state->editor_text_fill_enabled;
            preview.text_stroke_enabled = state->editor_text_stroke_enabled;
            preview.text_shadow_enabled = state->editor_text_shadow_enabled;
            preview.text_fill_opacity = state->editor_text_fill_opacity;
            preview.color.alpha = 0.92;
            if (state->active_tool == GTK4_TOOL_ARROW) {
                preview.kind = GTK4_MARK_ARROW;
            } else if (state->active_tool == GTK4_TOOL_LINE) {
                preview.kind = GTK4_MARK_LINE;
            } else if (state->active_tool == GTK4_TOOL_CALLOUT) {
                preview.kind = GTK4_MARK_CALLOUT;
                if (state->text_entry) {
                    const gchar *raw = gtk_editable_get_text(GTK_EDITABLE(state->text_entry));
                    preview.text = (raw && *raw) ? (gchar *)raw : "Callout";
                }
            } else {
                preview.kind = GTK4_MARK_RECT;
            }
            gtk4_draw_mark(cr, &preview);
        }
        cairo_restore(cr);
    }

    (void)widget;
}

static void gtk4_on_canvas_motion(GtkEventControllerMotion *controller, gdouble x, gdouble y, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble ix = 0.0;
    gdouble iy = 0.0;
    (void)controller;

    if (!state || !state->dragging) {
        return;
    }
    if (!gtk4_widget_to_image(state, x, y, &ix, &iy, TRUE)) {
        return;
    }
    state->drag_cur_x = ix;
    state->drag_cur_y = iy;
    gtk_widget_queue_draw(state->editor_area);
}

static Gtk4Mark *gtk4_find_last_stamp_mark(const Gtk4State *state) {
    if (!state || !state->marks) {
        return NULL;
    }
    for (gint i = (gint)state->marks->len - 1; i >= 0; i -= 1) {
        Gtk4Mark *mark = g_ptr_array_index(state->marks, i);
        if (mark && mark->kind == GTK4_MARK_STAMP) {
            return mark;
        }
    }
    return NULL;
}

static void gtk4_on_canvas_pressed(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble ix = 0.0;
    gdouble iy = 0.0;
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    Gtk4Mark *mark = NULL;
    (void)n_press;

    if (!state || !state->editor_pixbuf || button != GDK_BUTTON_PRIMARY) {
        return;
    }
    if (!gtk4_widget_to_image(state, x, y, &ix, &iy, FALSE)) {
        return;
    }
    if (state->active_tool == GTK4_TOOL_SELECT) {
        gtk4_set_status(state->editor_status, "Select tool is active. Switch to another tool to annotate.", "Select mode.");
        return;
    }
    if (state->active_tool == GTK4_TOOL_TEXT || state->active_tool == GTK4_TOOL_STAMP) {
        mark = g_new0(Gtk4Mark, 1);
        mark->kind = (state->active_tool == GTK4_TOOL_TEXT) ? GTK4_MARK_TEXT : GTK4_MARK_STAMP;
        mark->x1 = ix;
        mark->y1 = iy;
        mark->x2 = ix;
        mark->y2 = iy;
        mark->stroke_width = state->editor_stroke_width > 0.0 ? state->editor_stroke_width : 4.0;
        mark->color = state->editor_color;
        mark->stamp_radius = state->editor_stamp_radius > 0.0 ? state->editor_stamp_radius : 16.0;
        if (mark->kind == GTK4_MARK_TEXT) {
            mark->text = gtk4_get_trimmed_entry_text(state->text_entry);
            if (!mark->text || mark->text[0] == '\0') {
                g_free(mark->text);
                mark->text = g_strdup("Note");
            }
            gtk4_apply_current_text_style_to_mark(state, mark);
            gtk4_set_status(state->editor_status, "Text annotation added.", "Text added.");
        } else {
            gchar *stamp = NULL;
            Gtk4Mark *prev_stamp = state->editor_link_steps ? gtk4_find_last_stamp_mark(state) : NULL;
            if (state->editor_auto_step) {
                stamp = g_strdup_printf("%d", MAX(1, state->editor_next_step));
                state->editor_next_step += 1;
                gtk4_update_step_label(state);
            } else if (state->stamp_combo) {
                const gchar *selected = gtk4_dropdown_selected_id(state->stamp_combo,
                                                                  GTK4_STAMP_IDS,
                                                                  G_N_ELEMENTS(GTK4_STAMP_IDS));
                stamp = g_strdup(selected);
            }
            if (!stamp || stamp[0] == '\0') {
                g_free(stamp);
                stamp = g_strdup("1");
            }
            mark->text = stamp;
            if (state->editor_link_steps && prev_stamp) {
                Gtk4Mark *link = g_new0(Gtk4Mark, 1);
                link->kind = GTK4_MARK_ARROW;
                link->x1 = prev_stamp->x1;
                link->y1 = prev_stamp->y1;
                link->x2 = mark->x1;
                link->y2 = mark->y1;
                link->color = state->editor_color;
                link->stroke_width = MAX(2.0, state->editor_stroke_width);
                link->arrow_style = GTK4_ARROW_STYLE_CLASSIC;
                link->arrow_head_size = state->editor_arrow_head_size > 0.0 ? state->editor_arrow_head_size : 14.0;
                link->arrow_head_angle_deg = state->editor_arrow_head_angle_deg > 0.0 ? state->editor_arrow_head_angle_deg : 24.0;
                link->arrow_shadow = FALSE;
                link->arrow_shadow_offset = 0.0;
                g_ptr_array_add(state->marks, link);
                gtk4_set_status(state->editor_status, "Step/Stamp annotation added with link.", "Step linked.");
            } else {
                gtk4_set_status(state->editor_status, "Step/Stamp annotation added.", "Stamp added.");
            }
        }
        g_ptr_array_add(state->marks, mark);
        gtk_widget_queue_draw(state->editor_area);
        return;
    }

    state->dragging = TRUE;
    if (state->active_tool == GTK4_TOOL_ARROW) {
        state->drag_kind = GTK4_MARK_ARROW;
    } else if (state->active_tool == GTK4_TOOL_LINE) {
        state->drag_kind = GTK4_MARK_LINE;
    } else if (state->active_tool == GTK4_TOOL_CALLOUT) {
        state->drag_kind = GTK4_MARK_CALLOUT;
    } else {
        state->drag_kind = GTK4_MARK_RECT;
    }
    state->drag_start_x = ix;
    state->drag_start_y = iy;
    state->drag_cur_x = ix;
    state->drag_cur_y = iy;
    gtk_widget_queue_draw(state->editor_area);
}

static void gtk4_on_canvas_released(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble ix = 0.0;
    gdouble iy = 0.0;
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    Gtk4Mark *mark = NULL;
    gdouble dx = 0.0;
    gdouble dy = 0.0;
    (void)n_press;

    if (!state || !state->dragging || button != GDK_BUTTON_PRIMARY) {
        return;
    }

    if (!gtk4_widget_to_image(state, x, y, &ix, &iy, TRUE)) {
        ix = state->drag_cur_x;
        iy = state->drag_cur_y;
    }
    state->drag_cur_x = ix;
    state->drag_cur_y = iy;
    dx = state->drag_cur_x - state->drag_start_x;
    dy = state->drag_cur_y - state->drag_start_y;

    if (((state->drag_kind == GTK4_MARK_ARROW || state->drag_kind == GTK4_MARK_LINE) &&
         sqrt(dx * dx + dy * dy) >= 3.0) ||
        ((state->drag_kind == GTK4_MARK_RECT || state->drag_kind == GTK4_MARK_CALLOUT) &&
         fabs(dx) >= 6.0 && fabs(dy) >= 6.0)) {
        mark = g_new0(Gtk4Mark, 1);
        mark->kind = state->drag_kind;
        mark->x1 = state->drag_start_x;
        mark->y1 = state->drag_start_y;
        mark->x2 = state->drag_cur_x;
        mark->y2 = state->drag_cur_y;
        mark->stroke_width = state->editor_stroke_width > 0.0 ? state->editor_stroke_width : 4.0;
        mark->color = state->editor_color;
        mark->arrow_style = state->editor_arrow_style;
        mark->arrow_head_size = state->editor_arrow_head_size;
        mark->arrow_head_angle_deg = state->editor_arrow_head_angle_deg;
        mark->arrow_shadow = state->editor_arrow_shadow;
        mark->arrow_shadow_offset = state->editor_arrow_shadow_offset;
        mark->rect_rounded = state->editor_rect_rounded;
        mark->rect_fill = state->editor_rect_fill;
        mark->rect_fill_alpha = state->editor_rect_fill_alpha;
        mark->rect_shadow = state->editor_rect_shadow;
        mark->callout_style = state->editor_callout_style;
        mark->callout_fill = state->editor_callout_fill;
        mark->callout_fill_alpha = state->editor_callout_fill_alpha;
        mark->callout_shadow = state->editor_callout_shadow;
        if (state->drag_kind == GTK4_MARK_CALLOUT) {
            mark->text = gtk4_get_trimmed_entry_text(state->text_entry);
            if (!mark->text || mark->text[0] == '\0') {
                g_free(mark->text);
                mark->text = g_strdup("Callout");
            }
            gtk4_apply_current_text_style_to_mark(state, mark);
        }
        g_ptr_array_add(state->marks, mark);
        if (state->drag_kind == GTK4_MARK_ARROW) {
            gtk4_set_status(state->editor_status, "Arrow annotation added.", "Annotation added.");
        } else if (state->drag_kind == GTK4_MARK_LINE) {
            gtk4_set_status(state->editor_status, "Line annotation added.", "Annotation added.");
        } else if (state->drag_kind == GTK4_MARK_CALLOUT) {
            gtk4_set_status(state->editor_status, "Callout annotation added.", "Annotation added.");
        } else {
            gtk4_set_status(state->editor_status, "Rectangle annotation added.", "Annotation added.");
        }
    }

    state->dragging = FALSE;
    gtk_widget_queue_draw(state->editor_area);
}

static gboolean gtk4_on_canvas_scroll(GtkEventControllerScroll *controller, gdouble dx, gdouble dy, gpointer user_data) {
    Gtk4State *state = user_data;
    GdkModifierType mods = gtk_event_controller_get_current_event_state(GTK_EVENT_CONTROLLER(controller));
    gdouble zoom = 1.0;
    gchar *msg = NULL;
    (void)dx;

    if (!state || !state->editor_pixbuf || !(mods & GDK_CONTROL_MASK)) {
        return FALSE;
    }
    zoom = state->zoom_factor > 0.0 ? state->zoom_factor : 1.0;
    if (dy < 0.0) {
        zoom *= 1.10;
    } else if (dy > 0.0) {
        zoom /= 1.10;
    }
    zoom = CLAMP(zoom, 0.10, 8.0);
    state->zoom_factor = zoom;
    msg = g_strdup_printf("Canvas zoom: %.0f%% (Ctrl+Scroll).", zoom * 100.0);
    gtk4_set_status(state->editor_status, msg, "Zoom updated.");
    g_free(msg);
    gtk_widget_queue_draw(state->editor_area);
    return TRUE;
}

static const gchar *gtk4_arrow_style_to_id(Gtk4ArrowStyle style) {
    switch (style) {
        case GTK4_ARROW_STYLE_BOLD:
            return "bold";
        case GTK4_ARROW_STYLE_DASHED:
            return "dashed";
        case GTK4_ARROW_STYLE_DOTTED:
            return "dotted";
        case GTK4_ARROW_STYLE_DOUBLE:
            return "double";
        case GTK4_ARROW_STYLE_CLASSIC:
        default:
            return "classic";
    }
}

static Gtk4ArrowStyle gtk4_arrow_style_from_id(const gchar *id) {
    if (g_strcmp0(id, "bold") == 0) {
        return GTK4_ARROW_STYLE_BOLD;
    }
    if (g_strcmp0(id, "dashed") == 0) {
        return GTK4_ARROW_STYLE_DASHED;
    }
    if (g_strcmp0(id, "dotted") == 0) {
        return GTK4_ARROW_STYLE_DOTTED;
    }
    if (g_strcmp0(id, "double") == 0) {
        return GTK4_ARROW_STYLE_DOUBLE;
    }
    return GTK4_ARROW_STYLE_CLASSIC;
}

static const gchar *gtk4_callout_style_to_id(Gtk4CalloutStyle style) {
    switch (style) {
        case GTK4_CALLOUT_STYLE_LEFT:
            return "left";
        case GTK4_CALLOUT_STYLE_RIGHT:
            return "right";
        case GTK4_CALLOUT_STYLE_HIGHLIGHT:
            return "highlight";
        case GTK4_CALLOUT_STYLE_SHADOW:
            return "shadow";
        case GTK4_CALLOUT_STYLE_ROUNDED:
        default:
            return "rounded";
    }
}

static Gtk4CalloutStyle gtk4_callout_style_from_id(const gchar *id) {
    if (g_strcmp0(id, "left") == 0) {
        return GTK4_CALLOUT_STYLE_LEFT;
    }
    if (g_strcmp0(id, "right") == 0) {
        return GTK4_CALLOUT_STYLE_RIGHT;
    }
    if (g_strcmp0(id, "highlight") == 0) {
        return GTK4_CALLOUT_STYLE_HIGHLIGHT;
    }
    if (g_strcmp0(id, "shadow") == 0) {
        return GTK4_CALLOUT_STYLE_SHADOW;
    }
    return GTK4_CALLOUT_STYLE_ROUNDED;
}

static const gchar *gtk4_text_style_to_id(Gtk4TextStyle style) {
    switch (style) {
        case GTK4_TEXT_STYLE_NORMAL:
            return "normal";
        case GTK4_TEXT_STYLE_ITALIC:
            return "italic";
        case GTK4_TEXT_STYLE_BOLD_ITALIC:
            return "bolditalic";
        case GTK4_TEXT_STYLE_BOLD:
        default:
            return "bold";
    }
}

static Gtk4TextStyle gtk4_text_style_from_id(const gchar *id) {
    if (g_strcmp0(id, "normal") == 0) {
        return GTK4_TEXT_STYLE_NORMAL;
    }
    if (g_strcmp0(id, "italic") == 0) {
        return GTK4_TEXT_STYLE_ITALIC;
    }
    if (g_strcmp0(id, "bolditalic") == 0) {
        return GTK4_TEXT_STYLE_BOLD_ITALIC;
    }
    return GTK4_TEXT_STYLE_BOLD;
}

static void gtk4_update_step_label(Gtk4State *state) {
    gchar *text = NULL;
    if (!state || !state->step_label) {
        return;
    }
    text = g_strdup_printf("Next step: %d", MAX(1, state->editor_next_step));
    gtk_label_set_text(GTK_LABEL(state->step_label), text);
    g_free(text);
}

static gchar *gtk4_get_trimmed_entry_text(GtkWidget *entry) {
    gchar *copy = NULL;
    if (!entry) {
        return g_strdup("");
    }
    copy = g_strdup(gtk_editable_get_text(GTK_EDITABLE(entry)));
    if (!copy) {
        return g_strdup("");
    }
    g_strstrip(copy);
    return copy;
}

static guint gtk4_dropdown_index_for_id(const gchar *const *ids, guint count, const gchar *id) {
    if (!ids || count == 0) {
        return 0;
    }
    for (guint i = 0; i < count; i += 1) {
        if (g_strcmp0(ids[i], id) == 0) {
            return i;
        }
    }
    return 0;
}

static const gchar *gtk4_dropdown_selected_id(GtkWidget *dropdown,
                                              const gchar *const *ids,
                                              guint count) {
    guint idx = 0;
    if (!dropdown || !ids || count == 0) {
        return NULL;
    }
    idx = gtk_drop_down_get_selected(GTK_DROP_DOWN(dropdown));
    if (idx >= count) {
        idx = 0;
    }
    return ids[idx];
}

static void gtk4_dropdown_set_selected_id(GtkWidget *dropdown,
                                          const gchar *const *ids,
                                          guint count,
                                          const gchar *id) {
    guint idx = gtk4_dropdown_index_for_id(ids, count, id);
    if (!dropdown) {
        return;
    }
    gtk_drop_down_set_selected(GTK_DROP_DOWN(dropdown), idx);
}

static void gtk4_sync_editor_property_widgets(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->editor_stroke_scale) {
        gtk_range_set_value(GTK_RANGE(state->editor_stroke_scale), state->editor_stroke_width);
    }
    if (state->arrow_style_combo) {
        gtk4_dropdown_set_selected_id(state->arrow_style_combo,
                                      GTK4_ARROW_STYLE_IDS,
                                      G_N_ELEMENTS(GTK4_ARROW_STYLE_IDS),
                                      gtk4_arrow_style_to_id(state->editor_arrow_style));
    }
    if (state->arrow_head_scale) {
        gtk_range_set_value(GTK_RANGE(state->arrow_head_scale), state->editor_arrow_head_size);
    }
    if (state->arrow_angle_scale) {
        gtk_range_set_value(GTK_RANGE(state->arrow_angle_scale), state->editor_arrow_head_angle_deg);
    }
    if (state->arrow_shadow_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->arrow_shadow_check), state->editor_arrow_shadow);
    }
    if (state->arrow_shadow_offset_scale) {
        gtk_range_set_value(GTK_RANGE(state->arrow_shadow_offset_scale), state->editor_arrow_shadow_offset);
    }
    if (state->rect_round_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->rect_round_check), state->editor_rect_rounded);
    }
    if (state->rect_fill_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->rect_fill_check), state->editor_rect_fill);
    }
    if (state->rect_fill_scale) {
        gtk_range_set_value(GTK_RANGE(state->rect_fill_scale), state->editor_rect_fill_alpha);
    }
    if (state->rect_shadow_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->rect_shadow_check), state->editor_rect_shadow);
    }
    if (state->callout_style_combo) {
        gtk4_dropdown_set_selected_id(state->callout_style_combo,
                                      GTK4_CALLOUT_STYLE_IDS,
                                      G_N_ELEMENTS(GTK4_CALLOUT_STYLE_IDS),
                                      gtk4_callout_style_to_id(state->editor_callout_style));
    }
    if (state->callout_fill_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->callout_fill_check), state->editor_callout_fill);
    }
    if (state->callout_fill_scale) {
        gtk_range_set_value(GTK_RANGE(state->callout_fill_scale), state->editor_callout_fill_alpha);
    }
    if (state->callout_shadow_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->callout_shadow_check), state->editor_callout_shadow);
    }
    if (state->font_style_combo) {
        gtk4_dropdown_set_selected_id(state->font_style_combo,
                                      GTK4_FONT_STYLE_IDS,
                                      G_N_ELEMENTS(GTK4_FONT_STYLE_IDS),
                                      gtk4_text_style_to_id(state->editor_text_style));
    }
    if (state->font_size_scale) {
        gtk_range_set_value(GTK_RANGE(state->font_size_scale), state->editor_text_size);
    }
    if (state->text_stroke_scale) {
        gtk_range_set_value(GTK_RANGE(state->text_stroke_scale), state->editor_text_stroke_width);
    }
    if (state->text_fill_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->text_fill_check), state->editor_text_fill_enabled);
    }
    if (state->text_stroke_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->text_stroke_check), state->editor_text_stroke_enabled);
    }
    if (state->text_shadow_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->text_shadow_check), state->editor_text_shadow_enabled);
    }
    if (state->auto_step_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->auto_step_check), state->editor_auto_step);
    }
    if (state->link_step_check) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->link_step_check), state->editor_link_steps);
    }
    gtk4_update_step_label(state);
}

static void gtk4_on_arrow_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *active = NULL;
    (void)widget;
    if (!state) {
        return;
    }
    if (state->editor_stroke_scale) {
        state->editor_stroke_width = gtk_range_get_value(GTK_RANGE(state->editor_stroke_scale));
    }
    if (state->arrow_style_combo) {
        active = gtk4_dropdown_selected_id(state->arrow_style_combo,
                                           GTK4_ARROW_STYLE_IDS,
                                           G_N_ELEMENTS(GTK4_ARROW_STYLE_IDS));
        state->editor_arrow_style = gtk4_arrow_style_from_id(active);
    }
    if (state->arrow_head_scale) {
        state->editor_arrow_head_size = gtk_range_get_value(GTK_RANGE(state->arrow_head_scale));
    }
    if (state->arrow_angle_scale) {
        state->editor_arrow_head_angle_deg = gtk_range_get_value(GTK_RANGE(state->arrow_angle_scale));
    }
    if (state->arrow_shadow_check) {
        state->editor_arrow_shadow = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->arrow_shadow_check));
    }
    if (state->arrow_shadow_offset_scale) {
        state->editor_arrow_shadow_offset = gtk_range_get_value(GTK_RANGE(state->arrow_shadow_offset_scale));
    }
}

static void gtk4_on_rect_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)widget;
    if (!state) {
        return;
    }
    if (state->editor_stroke_scale) {
        state->editor_stroke_width = gtk_range_get_value(GTK_RANGE(state->editor_stroke_scale));
    }
    if (state->rect_round_check) {
        state->editor_rect_rounded = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->rect_round_check));
    }
    if (state->rect_fill_check) {
        state->editor_rect_fill = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->rect_fill_check));
    }
    if (state->rect_fill_scale) {
        state->editor_rect_fill_alpha = gtk_range_get_value(GTK_RANGE(state->rect_fill_scale));
    }
    if (state->rect_shadow_check) {
        state->editor_rect_shadow = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->rect_shadow_check));
    }
}

static void gtk4_on_callout_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *active = NULL;
    (void)widget;
    if (!state) {
        return;
    }
    if (state->editor_stroke_scale) {
        state->editor_stroke_width = gtk_range_get_value(GTK_RANGE(state->editor_stroke_scale));
    }
    if (state->callout_style_combo) {
        active = gtk4_dropdown_selected_id(state->callout_style_combo,
                                           GTK4_CALLOUT_STYLE_IDS,
                                           G_N_ELEMENTS(GTK4_CALLOUT_STYLE_IDS));
        state->editor_callout_style = gtk4_callout_style_from_id(active);
    }
    if (state->callout_fill_check) {
        state->editor_callout_fill = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->callout_fill_check));
    }
    if (state->callout_fill_scale) {
        state->editor_callout_fill_alpha = gtk_range_get_value(GTK_RANGE(state->callout_fill_scale));
    }
    if (state->callout_shadow_check) {
        state->editor_callout_shadow = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->callout_shadow_check));
    }
}

static void gtk4_on_line_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)widget;
    if (!state || !state->editor_stroke_scale) {
        return;
    }
    state->editor_stroke_width = gtk_range_get_value(GTK_RANGE(state->editor_stroke_scale));
}

static void gtk4_on_text_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *active = NULL;
    (void)widget;
    if (!state) {
        return;
    }
    if (state->font_style_combo) {
        active = gtk4_dropdown_selected_id(state->font_style_combo,
                                           GTK4_FONT_STYLE_IDS,
                                           G_N_ELEMENTS(GTK4_FONT_STYLE_IDS));
        state->editor_text_style = gtk4_text_style_from_id(active);
    }
    if (state->font_size_scale) {
        state->editor_text_size = gtk_range_get_value(GTK_RANGE(state->font_size_scale));
    }
    if (state->text_stroke_scale) {
        state->editor_text_stroke_width = gtk_range_get_value(GTK_RANGE(state->text_stroke_scale));
    }
    if (state->text_fill_check) {
        state->editor_text_fill_enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->text_fill_check));
    }
    if (state->text_stroke_check) {
        state->editor_text_stroke_enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->text_stroke_check));
    }
    if (state->text_shadow_check) {
        state->editor_text_shadow_enabled = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->text_shadow_check));
    }
    if (!state->editor_text_fill_enabled && !state->editor_text_stroke_enabled) {
        state->editor_text_fill_enabled = TRUE;
        if (state->text_fill_check) {
            gtk_check_button_set_active(GTK_CHECK_BUTTON(state->text_fill_check), TRUE);
        }
    }
}

static void gtk4_on_stamp_props_changed(GtkWidget *widget, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)widget;
    if (!state) {
        return;
    }
    if (state->auto_step_check) {
        state->editor_auto_step = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->auto_step_check));
    }
    if (state->link_step_check) {
        state->editor_link_steps = gtk_check_button_get_active(GTK_CHECK_BUTTON(state->link_step_check));
    }
    gtk4_update_step_label(state);
}

static void gtk4_on_arrow_dropdown_selected(GObject *object, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    gtk4_on_arrow_props_changed(GTK_WIDGET(object), user_data);
}

static void gtk4_on_callout_dropdown_selected(GObject *object, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    gtk4_on_callout_props_changed(GTK_WIDGET(object), user_data);
}

static void gtk4_on_font_dropdown_selected(GObject *object, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    gtk4_on_text_props_changed(GTK_WIDGET(object), user_data);
}

static void gtk4_on_stamp_dropdown_selected(GObject *object, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    gtk4_on_stamp_props_changed(GTK_WIDGET(object), user_data);
}

static void gtk4_apply_current_text_style_to_mark(Gtk4State *state, Gtk4Mark *mark) {
    if (!state || !mark) {
        return;
    }
    mark->text_style = state->editor_text_style;
    mark->text_size = state->editor_text_size > 0.0 ? state->editor_text_size : 24.0;
    mark->text_stroke_width = state->editor_text_stroke_width > 0.0 ? state->editor_text_stroke_width : 2.0;
    mark->text_fill_enabled = state->editor_text_fill_enabled;
    mark->text_stroke_enabled = state->editor_text_stroke_enabled;
    mark->text_shadow_enabled = state->editor_text_shadow_enabled;
    mark->text_fill_opacity = state->editor_text_fill_opacity > 0.0 ? state->editor_text_fill_opacity : 1.0;
    if (!mark->text_fill_enabled && !mark->text_stroke_enabled) {
        mark->text_fill_enabled = TRUE;
    }
}

static void gtk4_on_apply_text_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gchar *text = NULL;
    gboolean applied = FALSE;
    (void)button;
    if (!state || !state->marks) {
        return;
    }
    text = gtk4_get_trimmed_entry_text(state->text_entry);
    if (!text || text[0] == '\0') {
        g_free(text);
        text = g_strdup("Note");
    }
    for (gint i = (gint)state->marks->len - 1; i >= 0; i -= 1) {
        Gtk4Mark *mark = g_ptr_array_index(state->marks, (guint)i);
        if (!mark) {
            continue;
        }
        if (mark->kind == GTK4_MARK_TEXT || mark->kind == GTK4_MARK_STAMP || mark->kind == GTK4_MARK_CALLOUT) {
            g_free(mark->text);
            mark->text = g_strdup(text);
            if (mark->kind != GTK4_MARK_STAMP) {
                gtk4_apply_current_text_style_to_mark(state, mark);
            }
            applied = TRUE;
            break;
        }
    }
    if (applied) {
        gtk4_set_status(state->editor_status, "Updated latest text/callout/stamp annotation.", "Text updated.");
        if (state->editor_area) {
            gtk_widget_queue_draw(state->editor_area);
        }
    } else {
        gtk4_set_status(state->editor_status, "Add a text/callout/stamp annotation first.", "No text target.");
    }
    g_free(text);
}

static void gtk4_apply_thumb_layout(Gtk4State *state) {
    gint pos = 0;
    if (!state || !state->editor_split) {
        return;
    }
    if (state->editor_thumbs_visible) {
        pos = gtk4_clamp_editor_split_position(state, state->split_pos_saved > 0 ? state->split_pos_saved : 430);
        state->split_pos_saved = pos;
        gtk_paned_set_position(GTK_PANED(state->editor_split), pos);
    } else {
        pos = gtk_paned_get_position(GTK_PANED(state->editor_split));
        if (pos > 0) {
            state->split_pos_saved = gtk4_clamp_editor_split_position(state, pos);
        }
        gtk_paned_set_position(GTK_PANED(state->editor_split), gtk4_clamp_editor_split_position(state, 99999));
    }
}

static void gtk4_apply_dock_layout(Gtk4State *state) {
    gint target_pos = 0;
    if (!state || !state->editor_studio_shell || !state->editor_dock || !state->editor_main_box) {
        return;
    }
    if (state->editor_dock_right) {
        gtk_paned_set_start_child(GTK_PANED(state->editor_studio_shell), state->editor_main_box);
        gtk_paned_set_end_child(GTK_PANED(state->editor_studio_shell), state->editor_dock);
        target_pos = gtk4_clamp_studio_split_position(
            state,
            state->studio_split_pos_saved > 0 ? state->studio_split_pos_saved : 840);
        state->studio_split_pos_saved = target_pos;
        gtk_paned_set_position(GTK_PANED(state->editor_studio_shell), target_pos);
        if (state->editor_dock_toggle_btn) {
            gtk_button_set_label(GTK_BUTTON(state->editor_dock_toggle_btn), "Dock Left");
        }
    } else {
        gtk_paned_set_start_child(GTK_PANED(state->editor_studio_shell), state->editor_dock);
        gtk_paned_set_end_child(GTK_PANED(state->editor_studio_shell), state->editor_main_box);
        target_pos = gtk4_clamp_studio_dock_width(
            state,
            state->studio_dock_width_saved > 0 ? state->studio_dock_width_saved : 280);
        state->studio_dock_width_saved = target_pos;
        gtk_paned_set_position(GTK_PANED(state->editor_studio_shell), target_pos);
        if (state->editor_dock_toggle_btn) {
            gtk_button_set_label(GTK_BUTTON(state->editor_dock_toggle_btn), "Dock Right");
        }
    }
}

static void gtk4_apply_editor_panel_visibility(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->editor_styles_scroller) {
        gtk_widget_set_visible(state->editor_styles_scroller, state->editor_styles_visible);
    }
    if (state->editor_toggle_styles_btn) {
        gtk_button_set_label(GTK_BUTTON(state->editor_toggle_styles_btn),
                             state->editor_styles_visible ? "Hide Styles" : "Show Styles");
    }
    if (state->editor_browser_box) {
        gtk_widget_set_visible(state->editor_browser_box, state->editor_thumbs_visible);
    }
    if (state->editor_toggle_thumbs_btn) {
        gtk_button_set_label(GTK_BUTTON(state->editor_toggle_thumbs_btn),
                             state->editor_thumbs_visible ? "Hide Thumbs" : "Show Thumbs");
    }
    gtk4_apply_thumb_layout(state);
}

static void gtk4_on_toggle_styles_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    if (!state || !state->editor_styles_scroller || !state->editor_toggle_styles_btn) {
        return;
    }
    state->editor_styles_visible = !state->editor_styles_visible;
    gtk4_apply_editor_panel_visibility(state);
    gtk4_ui_state_schedule_save(state);
    gtk4_set_status(state->editor_status,
                    state->editor_styles_visible ? "Quick styles shown." : "Quick styles hidden.",
                    "Style visibility changed.");
}

static void gtk4_on_toggle_thumbs_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    if (!state || !state->editor_browser_box || !state->editor_toggle_thumbs_btn) {
        return;
    }
    state->editor_thumbs_visible = !state->editor_thumbs_visible;
    gtk4_apply_editor_panel_visibility(state);
    gtk4_ui_state_schedule_save(state);
    gtk4_set_status(state->editor_status,
                    state->editor_thumbs_visible ? "Thumbnail browser shown." : "Thumbnail browser hidden.",
                    "Thumbnail visibility changed.");
}

static void gtk4_set_thumb_preview_width(Gtk4State *state, gint width) {
    gdouble current = 0.0;
    guint preset_idx = 0;
    if (!state) {
        return;
    }
    width = CLAMP(width, 170, 440);
    state->thumb_preview_width = width;
    state->thumb_preview_height = CLAMP((gint)lrint((gdouble)width * 0.62), 110, 300);
    preset_idx = gtk4_thumb_preset_for_width(width);
    state->thumb_ui_syncing = TRUE;
    if (state->thumb_size_scale) {
        current = gtk_range_get_value(GTK_RANGE(state->thumb_size_scale));
        if (fabs(current - (gdouble)width) > 0.1) {
            gtk_range_set_value(GTK_RANGE(state->thumb_size_scale), (gdouble)width);
        }
    }
    if (state->thumb_preset_dropdown) {
        if (gtk_drop_down_get_selected(GTK_DROP_DOWN(state->thumb_preset_dropdown)) != preset_idx) {
            gtk_drop_down_set_selected(GTK_DROP_DOWN(state->thumb_preset_dropdown), preset_idx);
        }
    }
    state->thumb_ui_syncing = FALSE;
}

static void gtk4_adjust_thumb_region(Gtk4State *state, gint delta) {
    gint pos = 0;
    if (!state || !state->editor_split || !state->editor_thumbs_visible) {
        return;
    }
    pos = gtk_paned_get_position(GTK_PANED(state->editor_split));
    state->split_pos_saved = gtk4_clamp_editor_split_position(state, pos + delta);
    gtk_paned_set_position(GTK_PANED(state->editor_split), state->split_pos_saved);
    gtk4_ui_state_schedule_save(state);
}

static void gtk4_on_thumb_region_bigger(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    if (!state || !state->editor_thumbs_visible) {
        gtk4_set_status(state ? state->editor_status : NULL,
                        "Show thumbnails first, then resize thumbnail region.",
                        "Thumbnail browser is hidden.");
        return;
    }
    gtk4_adjust_thumb_region(state, -60);
    gtk4_set_status(state ? state->editor_status : NULL,
                    "Thumbnail region expanded.",
                    "Thumbnail region expanded.");
}

static void gtk4_on_thumb_region_smaller(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    if (!state || !state->editor_thumbs_visible) {
        gtk4_set_status(state ? state->editor_status : NULL,
                        "Show thumbnails first, then resize thumbnail region.",
                        "Thumbnail browser is hidden.");
        return;
    }
    gtk4_adjust_thumb_region(state, 60);
    gtk4_set_status(state ? state->editor_status : NULL,
                    "Thumbnail region reduced.",
                    "Thumbnail region reduced.");
}

static void gtk4_on_thumb_size_changed(GtkRange *range, gpointer user_data) {
    Gtk4State *state = user_data;
    gint width = 0;
    if (!state || !range) {
        return;
    }
    if (state->thumb_ui_syncing) {
        return;
    }
    width = (gint)lrint(gtk_range_get_value(range));
    gtk4_set_thumb_preview_width(state, width);
    gtk4_reload(state);
    gtk4_ui_state_schedule_save(state);
}

static void gtk4_on_thumb_preset_selected(GObject *object, GParamSpec *pspec, gpointer user_data) {
    Gtk4State *state = user_data;
    guint sel = 1;
    (void)pspec;
    if (!state || !object || state->thumb_ui_syncing) {
        return;
    }
    sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(object));
    gtk4_set_thumb_preview_width(state, gtk4_thumb_width_for_preset(sel));
    gtk4_reload(state);
    gtk4_ui_state_schedule_save(state);
}

static void gtk4_on_split_position_notify(GObject *object, GParamSpec *pspec, gpointer user_data) {
    Gtk4State *state = user_data;
    gint pos = 0;
    (void)pspec;
    if (!state || !object) {
        return;
    }
    pos = gtk_paned_get_position(GTK_PANED(object));
    if (pos <= 0) {
        return;
    }
    if (object == G_OBJECT(state->editor_split)) {
        if (state->editor_thumbs_visible) {
            state->split_pos_saved = gtk4_clamp_editor_split_position(state, pos);
        }
    } else if (object == G_OBJECT(state->editor_studio_shell)) {
        if (state->editor_dock_right) {
            state->studio_split_pos_saved = gtk4_clamp_studio_split_position(state, pos);
        } else {
            state->studio_dock_width_saved = gtk4_clamp_studio_dock_width(state, pos);
        }
    }
    gtk4_ui_state_schedule_save(state);
}

static void gtk4_on_zoom_reset_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    if (!state) {
        return;
    }
    state->zoom_factor = 1.0;
    if (state->editor_area) {
        gtk_widget_queue_draw(state->editor_area);
    }
    gtk4_set_status(state->editor_status, "Canvas zoom reset to 100%.", "Zoom reset.");
}

static void gtk4_on_dock_toggle_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gint current_pos = 0;
    (void)button;
    if (!state || !state->editor_studio_shell || !state->editor_dock || !state->editor_main_box || !state->editor_dock_toggle_btn) {
        return;
    }
    current_pos = gtk_paned_get_position(GTK_PANED(state->editor_studio_shell));
    if (state->editor_dock_right) {
        if (current_pos > 0) {
            state->studio_split_pos_saved = gtk4_clamp_studio_split_position(state, current_pos);
        }
        state->editor_dock_right = FALSE;
    } else {
        if (current_pos > 0) {
            state->studio_dock_width_saved = gtk4_clamp_studio_dock_width(state, current_pos);
        }
        state->editor_dock_right = TRUE;
    }
    gtk4_apply_dock_layout(state);
    gtk4_ui_state_schedule_save(state);
}

static void gtk4_apply_quick_style(Gtk4State *state, const gchar *style_id) {
    const gchar *status = "Quick style applied.";
    if (!state || !style_id) {
        return;
    }
    if (g_strcmp0(style_id, "arrow_classic") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#ef5b5b", "Arrow Red");
        state->editor_stroke_width = 4.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_CLASSIC;
        state->editor_arrow_shadow = FALSE;
        status = "Quick style: Arrow Classic";
    } else if (g_strcmp0(style_id, "arrow_bold") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#ff8f33", "Arrow Orange");
        state->editor_stroke_width = 7.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_BOLD;
        state->editor_arrow_shadow = FALSE;
        status = "Quick style: Arrow Bold";
    } else if (g_strcmp0(style_id, "arrow_pointer") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#8b5cf6", "Arrow Pointer");
        state->editor_stroke_width = 5.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_CLASSIC;
        state->editor_arrow_head_size = 24.0;
        state->editor_arrow_head_angle_deg = 17.0;
        state->editor_arrow_shadow = FALSE;
        status = "Quick style: Arrow Pointer";
    } else if (g_strcmp0(style_id, "arrow_dashed") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#ef5b5b", "Arrow Red");
        state->editor_stroke_width = 4.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_DASHED;
        status = "Quick style: Arrow Dashed";
    } else if (g_strcmp0(style_id, "arrow_dotted") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#ef5b5b", "Arrow Red");
        state->editor_stroke_width = 4.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_DOTTED;
        status = "Quick style: Arrow Dotted";
    } else if (g_strcmp0(style_id, "arrow_double") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_editor_color(state, "#ff8f33", "Arrow Orange");
        state->editor_stroke_width = 4.0;
        state->editor_arrow_style = GTK4_ARROW_STYLE_DOUBLE;
        status = "Quick style: Arrow Double";
    } else if (g_strcmp0(style_id, "callout_red") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_editor_color(state, "#ef5b5b", "Callout Red");
        state->editor_callout_style = GTK4_CALLOUT_STYLE_LEFT;
        state->editor_callout_fill = FALSE;
        state->editor_callout_shadow = FALSE;
        status = "Quick style: Callout Red";
    } else if (g_strcmp0(style_id, "callout_green") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_editor_color(state, "#4bd38b", "Callout Green");
        state->editor_callout_style = GTK4_CALLOUT_STYLE_LEFT;
        state->editor_callout_fill = FALSE;
        state->editor_callout_shadow = FALSE;
        status = "Quick style: Callout Green";
    } else if (g_strcmp0(style_id, "callout_right") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_editor_color(state, "#ff8f33", "Callout Right");
        state->editor_callout_style = GTK4_CALLOUT_STYLE_RIGHT;
        state->editor_callout_fill = FALSE;
        state->editor_callout_shadow = FALSE;
        status = "Quick style: Callout Right";
    } else if (g_strcmp0(style_id, "callout_highlight") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_editor_color(state, "#ffd84d", "Highlight");
        state->editor_callout_style = GTK4_CALLOUT_STYLE_HIGHLIGHT;
        state->editor_callout_fill = TRUE;
        state->editor_callout_fill_alpha = 0.44;
        state->editor_callout_shadow = FALSE;
        status = "Quick style: Highlight";
    } else if (g_strcmp0(style_id, "callout_shadow") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_editor_color(state, "#4d8bff", "Callout Shadow");
        state->editor_callout_style = GTK4_CALLOUT_STYLE_SHADOW;
        state->editor_callout_fill = TRUE;
        state->editor_callout_fill_alpha = 0.22;
        state->editor_callout_shadow = TRUE;
        status = "Quick style: Callout Shadow";
    } else if (g_strcmp0(style_id, "rect_rounded") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_RECT);
        gtk4_set_editor_color(state, "#ef5b5b", "Rounded Rect");
        state->editor_rect_rounded = TRUE;
        state->editor_rect_fill = FALSE;
        state->editor_rect_shadow = FALSE;
        status = "Quick style: Rounded Rect";
    } else if (g_strcmp0(style_id, "rect_shadow") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_RECT);
        gtk4_set_editor_color(state, "#4d8bff", "Rect Shadow");
        state->editor_rect_rounded = TRUE;
        state->editor_rect_fill = TRUE;
        state->editor_rect_fill_alpha = 0.20;
        state->editor_rect_shadow = TRUE;
        status = "Quick style: Rect Shadow";
    } else if (g_strcmp0(style_id, "step_badge") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_STAMP);
        gtk4_set_editor_color(state, "#ef5b5b", "Step Red");
        state->editor_auto_step = TRUE;
        state->editor_link_steps = FALSE;
        state->editor_stamp_radius = 17.0;
        status = "Quick style: Step Badge";
    } else if (g_strcmp0(style_id, "step_badge_blue") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_STAMP);
        gtk4_set_editor_color(state, "#3b82ff", "Step Blue");
        state->editor_auto_step = TRUE;
        state->editor_link_steps = FALSE;
        state->editor_stamp_radius = 17.0;
        status = "Quick style: Step Badge Blue";
    } else if (g_strcmp0(style_id, "step_badge_linked") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_STAMP);
        gtk4_set_editor_color(state, "#ff8f33", "Step Linked");
        state->editor_auto_step = TRUE;
        state->editor_link_steps = TRUE;
        state->editor_stamp_radius = 17.0;
        status = "Quick style: Step Linked";
    } else if (g_strcmp0(style_id, "text_bold") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_TEXT);
        gtk4_set_editor_color(state, "#4dd3ff", "Text Cyan");
        state->editor_text_style = GTK4_TEXT_STYLE_BOLD;
        state->editor_text_size = 34.0;
        state->editor_text_stroke_width = 2.0;
        state->editor_text_fill_enabled = TRUE;
        state->editor_text_stroke_enabled = FALSE;
        state->editor_text_shadow_enabled = TRUE;
        status = "Quick style: Text Bold";
    } else if (g_strcmp0(style_id, "text_outline") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_TEXT);
        gtk4_set_editor_color(state, "#f8f9ff", "Text White");
        state->editor_text_style = GTK4_TEXT_STYLE_BOLD;
        state->editor_text_size = 32.0;
        state->editor_text_stroke_width = 3.0;
        state->editor_text_fill_enabled = TRUE;
        state->editor_text_stroke_enabled = TRUE;
        state->editor_text_shadow_enabled = FALSE;
        status = "Quick style: Text Outline";
    }
    gtk4_sync_editor_property_widgets(state);
    gtk4_set_status(state->editor_status, status, "Quick style applied.");
}

static void gtk4_on_quick_style_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *style_id = g_object_get_data(G_OBJECT(button), "style-id");
    gtk4_apply_quick_style(state, style_id);
}

static void gtk4_update_properties_panel(Gtk4State *state) {
    const gchar *page = "select";
    const gchar *title = "Selection properties";
    if (!state || !state->editor_props_stack || !state->editor_props_title) {
        return;
    }
    switch (state->active_tool) {
        case GTK4_TOOL_ARROW:
            page = "arrow";
            title = "Arrow properties";
            break;
        case GTK4_TOOL_LINE:
            page = "line";
            title = "Line properties";
            break;
        case GTK4_TOOL_RECT:
            page = "rect";
            title = "Rectangle properties";
            break;
        case GTK4_TOOL_CALLOUT:
            page = "callout";
            title = "Callout properties";
            break;
        case GTK4_TOOL_TEXT:
            page = "text";
            title = "Text properties";
            break;
        case GTK4_TOOL_STAMP:
            page = "stamp";
            title = "Step/Stamp properties";
            break;
        default:
            page = "select";
            title = "Selection properties";
            break;
    }
    gtk_stack_set_visible_child_name(GTK_STACK(state->editor_props_stack), page);
    gtk_label_set_text(GTK_LABEL(state->editor_props_title), title);
}

static void gtk4_set_active_tool(Gtk4State *state, Gtk4Tool tool) {
    if (!state) {
        return;
    }
    state->active_tool = tool;
    if (state->tool_syncing) {
        return;
    }
    state->tool_syncing = TRUE;
    if (state->tool_select_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_select_btn), tool == GTK4_TOOL_SELECT);
    }
    if (state->tool_arrow_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_arrow_btn), tool == GTK4_TOOL_ARROW);
    }
    if (state->tool_line_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_line_btn), tool == GTK4_TOOL_LINE);
    }
    if (state->tool_rect_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_rect_btn), tool == GTK4_TOOL_RECT);
    }
    if (state->tool_callout_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_callout_btn), tool == GTK4_TOOL_CALLOUT);
    }
    if (state->tool_text_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_text_btn), tool == GTK4_TOOL_TEXT);
    }
    if (state->tool_stamp_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_stamp_btn), tool == GTK4_TOOL_STAMP);
    }
    state->tool_syncing = FALSE;
    gtk4_update_properties_panel(state);
}

static void gtk4_on_tool_toggled(GtkCheckButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *tool_id = g_object_get_data(G_OBJECT(button), "tool-id");
    if (!state || !gtk_check_button_get_active(button) || state->tool_syncing) {
        return;
    }
    if (g_strcmp0(tool_id, "arrow") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_ARROW);
        gtk4_set_status(state->editor_status, "Arrow tool selected.", "Arrow tool.");
    } else if (g_strcmp0(tool_id, "line") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_LINE);
        gtk4_set_status(state->editor_status, "Line tool selected.", "Line tool.");
    } else if (g_strcmp0(tool_id, "rect") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_RECT);
        gtk4_set_status(state->editor_status, "Rectangle tool selected.", "Rect tool.");
    } else if (g_strcmp0(tool_id, "callout") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_CALLOUT);
        gtk4_set_status(state->editor_status, "Callout tool selected.", "Callout tool.");
    } else if (g_strcmp0(tool_id, "text") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_TEXT);
        gtk4_set_status(state->editor_status, "Text tool selected. Click canvas to place text.", "Text tool.");
    } else if (g_strcmp0(tool_id, "stamp") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_STAMP);
        gtk4_set_status(state->editor_status, "Step/Stamp tool selected. Click canvas to place stamp.", "Stamp tool.");
    } else {
        gtk4_set_active_tool(state, GTK4_TOOL_SELECT);
        gtk4_set_status(state->editor_status, "Select tool selected.", "Select tool.");
    }
}

static void gtk4_delete_selected(Gtk4State *state) {
    GPtrArray *paths = NULL;
    guint removed = 0;
    guint failed = 0;
    gchar *msg = NULL;
    if (!state) {
        return;
    }
    paths = gtk4_selected_paths(state);
    if (paths->len == 0) {
        gtk4_set_status(state->status_label, "Select one or more images to delete.", "No images selected.");
        g_ptr_array_free(paths, TRUE);
        return;
    }
    for (guint i = 0; i < paths->len; i += 1) {
        const gchar *path = g_ptr_array_index(paths, i);
        if (path && g_remove(path) == 0) {
            removed += 1;
        } else {
            failed += 1;
        }
    }
    g_ptr_array_free(paths, TRUE);
    msg = g_strdup_printf("Deleted %u image(s)%s.", removed, failed > 0 ? " (some failed)" : "");
    gtk4_set_status(state->status_label, msg, "Delete finished.");
    g_free(msg);
}

static void gtk4_reload(Gtk4State *state) {
    GDir *dir = NULL;
    const gchar *name = NULL;
    const gchar *query = NULL;
    GPtrArray *entries = g_ptr_array_new_with_free_func(gtk4_free_image_entry);
    GtkWidget *child = NULL;
    gchar *status = NULL;

    if (!state || !state->thumb_flow || !state->shots_dir) {
        if (entries) {
            g_ptr_array_free(entries, TRUE);
        }
        return;
    }

    query = gtk_editable_get_text(GTK_EDITABLE(state->search_entry));
    while ((child = gtk_widget_get_first_child(state->thumb_flow)) != NULL) {
        gtk_flow_box_remove(GTK_FLOW_BOX(state->thumb_flow), child);
    }

    g_mkdir_with_parents(state->shots_dir, 0755);
    dir = g_dir_open(state->shots_dir, 0, NULL);
    if (!dir) {
        status = g_strdup_printf("Could not open folder: %s", state->shots_dir);
        gtk4_set_status(state->status_label, status, "Could not open screenshot folder.");
        g_free(status);
        g_ptr_array_free(entries, TRUE);
        gtk4_update_selection_ui(state);
        return;
    }

    while ((name = g_dir_read_name(dir)) != NULL) {
        Gtk4ImageEntry *entry = NULL;
        gchar *path = NULL;
        GStatBuf st = {0};
        if (!gtk4_ends_with_image_ext(name) || !gtk4_name_matches_query(name, query)) {
            continue;
        }
        path = g_build_filename(state->shots_dir, name, NULL);
        if (g_stat(path, &st) != 0) {
            g_free(path);
            continue;
        }
        entry = g_new0(Gtk4ImageEntry, 1);
        entry->name = g_strdup(name);
        entry->path = path;
        entry->mtime = (gint64)st.st_mtime;
        g_ptr_array_add(entries, entry);
    }
    g_dir_close(dir);
    g_ptr_array_sort(entries, gtk4_compare_entries_desc);

    for (guint i = 0; i < entries->len; i += 1) {
        Gtk4ImageEntry *entry = g_ptr_array_index(entries, i);
        gint preview_w = (state->thumb_preview_width > 0) ? state->thumb_preview_width : 240;
        gint preview_h = (state->thumb_preview_height > 0) ? state->thumb_preview_height : 150;
        GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(entry->path, preview_w, preview_h, TRUE, NULL);
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        GtkWidget *label = gtk_label_new(entry->name);
        GtkWidget *preview = NULL;

        gtk_widget_set_tooltip_text(row, entry->path);
        g_object_set_data_full(G_OBJECT(row), "shot-path", g_strdup(entry->path), g_free);
        gtk_widget_add_css_class(row, "thumb-card");
        gtk_widget_set_margin_start(row, 4);
        gtk_widget_set_margin_end(row, 4);
        gtk_widget_set_margin_top(row, 4);
        gtk_widget_set_margin_bottom(row, 4);

        if (pix) {
            GdkTexture *texture = gdk_texture_new_for_pixbuf(pix);
            preview = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
            gtk_picture_set_can_shrink(GTK_PICTURE(preview), TRUE);
            gtk_picture_set_content_fit(GTK_PICTURE(preview), GTK_CONTENT_FIT_CONTAIN);
            gtk_widget_set_size_request(preview, preview_w, preview_h);
            g_object_unref(texture);
            g_object_unref(pix);
        } else {
            preview = gtk_image_new_from_icon_name("image-x-generic-symbolic");
            gtk_widget_set_size_request(preview, preview_w, preview_h);
        }

        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        gtk_widget_add_css_class(label, "thumb-name");
        gtk_widget_set_size_request(label, preview_w, -1);
        gtk_box_append(GTK_BOX(row), preview);
        gtk_box_append(GTK_BOX(row), label);
        gtk_flow_box_append(GTK_FLOW_BOX(state->thumb_flow), row);
    }

    if (entries->len == 0) {
        status = g_strdup_printf("No images found in %s", state->shots_dir);
    } else {
        status = g_strdup_printf("%u image(s) loaded from %s", entries->len, state->shots_dir);
    }
    gtk4_set_status(state->status_label, status, "Reloaded.");
    g_free(status);
    g_ptr_array_free(entries, TRUE);
    gtk4_update_selection_ui(state);
}

static void gtk4_on_theme_selected(GObject *dropdown, GParamSpec *pspec, gpointer user_data) {
    Gtk4State *state = user_data;
    guint sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(dropdown));
    (void)pspec;
    gtk4_apply_theme(sel == 0);
    gtk4_set_global_status(state, sel == 0 ? "Theme switched to Dark." : "Theme switched to Light.");
}

static void gtk4_night_refresh(Gtk4State *state) {
    gchar *redshift = NULL;
    if (!state || !state->night_status) {
        return;
    }
    redshift = g_find_program_in_path("redshift");
    if (redshift) {
        gtk_label_set_text(GTK_LABEL(state->night_status),
                           "Night Light ready. Auto uses geoclue2; Manual uses slider + Apply.");
    } else {
        gtk_label_set_text(GTK_LABEL(state->night_status),
                           "redshift not found. Install with: sudo apt install redshift");
    }
    g_free(redshift);
}

static void gtk4_night_apply_manual(Gtk4State *state) {
    gint kelvin = 4200;
    gchar *cmd = NULL;
    if (!state || !state->night_scale) {
        return;
    }
    kelvin = (gint)gtk_range_get_value(GTK_RANGE(state->night_scale));
    cmd = g_strdup_printf("redshift -O %d", kelvin);
    gtk4_spawn_with_feedback(state, cmd, "Applied manual Night Light temperature.");
    g_free(cmd);
    gtk4_night_refresh(state);
}

static void gtk4_on_night_apply(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_night_apply_manual(user_data);
}

static void gtk4_on_night_warmer(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble v = 0.0;
    (void)button;
    if (!state || !state->night_scale) {
        return;
    }
    v = gtk_range_get_value(GTK_RANGE(state->night_scale));
    gtk_range_set_value(GTK_RANGE(state->night_scale), CLAMP(v - 200.0, 1500.0, 6500.0));
    gtk4_night_apply_manual(state);
}

static void gtk4_on_night_cooler(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble v = 0.0;
    (void)button;
    if (!state || !state->night_scale) {
        return;
    }
    v = gtk_range_get_value(GTK_RANGE(state->night_scale));
    gtk_range_set_value(GTK_RANGE(state->night_scale), CLAMP(v + 200.0, 1500.0, 6500.0));
    gtk4_night_apply_manual(state);
}

static void gtk4_on_night_disable(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    gtk4_spawn_with_feedback(state, "redshift -x", "Night Light disabled.");
    if (state && state->night_status) {
        gtk_label_set_text(GTK_LABEL(state->night_status), "Night Light disabled.");
    }
}

static void gtk4_on_night_refresh(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_night_refresh(user_data);
}

static gboolean gtk4_on_night_auto_toggled(GtkSwitch *sw, gboolean state, gpointer user_data) {
    Gtk4State *app = user_data;
    (void)sw;
    if (state) {
        gtk4_spawn_with_feedback(app,
                                 "bash -lc \"pkill -x redshift >/dev/null 2>&1 || true; redshift -l geoclue2 -t 5700:3600 -m randr >/dev/null 2>&1 &\"",
                                 "Auto Night Light enabled.");
        if (app && app->night_status) {
            gtk_label_set_text(GTK_LABEL(app->night_status), "Auto Night Light enabled (geoclue2 + randr).");
        }
    } else {
        gtk4_spawn_with_feedback(app,
                                 "bash -lc \"redshift -x >/dev/null 2>&1; pkill -x redshift >/dev/null 2>&1 || true\"",
                                 "Auto Night Light disabled.");
        if (app && app->night_status) {
            gtk_label_set_text(GTK_LABEL(app->night_status), "Auto Night Light disabled.");
        }
    }
    return FALSE;
}

static void gtk4_audio_refresh(Gtk4State *state) {
    gchar *pactl = NULL;
    gchar *msg = NULL;
    if (!state || !state->audio_status) {
        return;
    }
    pactl = g_find_program_in_path("pactl");
    msg = g_strdup_printf("Volume target: %.0f%%\nBackend: %s",
                          state->audio_scale ? gtk_range_get_value(GTK_RANGE(state->audio_scale)) : 0.0,
                          pactl ? "pactl available" : "pactl not found");
    gtk_label_set_text(GTK_LABEL(state->audio_status), msg);
    g_free(msg);
    g_free(pactl);
}

static void gtk4_on_audio_apply(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gint pct = 20;
    gchar *cmd = NULL;
    (void)button;
    if (!state || !state->audio_scale) {
        return;
    }
    pct = (gint)gtk_range_get_value(GTK_RANGE(state->audio_scale));
    cmd = g_strdup_printf("pactl set-sink-volume @DEFAULT_SINK@ %d%%", pct);
    gtk4_spawn_with_feedback(state, cmd, "Applied volume.");
    g_free(cmd);
    gtk4_audio_refresh(state);
}

static void gtk4_on_audio_adjust(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *delta = g_object_get_data(G_OBJECT(button), "delta");
    gdouble v = 20.0;
    (void)button;
    if (!state || !state->audio_scale || !delta) {
        return;
    }
    v = gtk_range_get_value(GTK_RANGE(state->audio_scale));
    if (g_strcmp0(delta, "+5") == 0) {
        v += 5.0;
    } else {
        v -= 5.0;
    }
    gtk_range_set_value(GTK_RANGE(state->audio_scale), CLAMP(v, 0.0, 150.0));
    gtk4_on_audio_apply(NULL, state);
}

static void gtk4_on_audio_toggle_mute(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    gtk4_spawn_with_feedback(state, "pactl set-sink-mute @DEFAULT_SINK@ toggle", "Toggled mute.");
    gtk4_audio_refresh(state);
}

static void gtk4_on_audio_open_mixer(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    gtk4_spawn_with_feedback(state, "pavucontrol", "Opening Pavucontrol...");
}

static void gtk4_on_audio_refresh(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_audio_refresh(user_data);
}

static gboolean gtk4_clear_utility_click_indicator(gpointer data) {
    GtkWidget *button = GTK_WIDGET(data);
    if (button && GTK_IS_WIDGET(button)) {
        gtk_widget_remove_css_class(button, "utility-btn-active");
    }
    g_object_unref(button);
    return G_SOURCE_REMOVE;
}

static void gtk4_mark_utility_clicked(GtkWidget *button) {
    if (!button || !GTK_IS_WIDGET(button)) {
        return;
    }
    gtk_widget_add_css_class(button, "utility-btn-active");
    g_timeout_add(220, gtk4_clear_utility_click_indicator, g_object_ref(button));
}

static void gtk4_on_utility_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *cmd = g_object_get_data(G_OBJECT(button), "cmd");
    const gchar *status = g_object_get_data(G_OBJECT(button), "status");
    gtk4_mark_utility_clicked(GTK_WIDGET(button));
    if (!cmd) {
        gtk4_set_global_status(state, "This utility has no command configured.");
        return;
    }
    gtk4_spawn_with_feedback(state, cmd, status ? status : "Utility launched.");
    if (state && state->status_label && status) {
        gtk4_set_status(state->status_label, status, "Command launched.");
    }
}

static GtkWidget *gtk4_make_utility_button(Gtk4State *state,
                                           const gchar *title,
                                           const gchar *subtitle,
                                           const gchar *cmd,
                                           const gchar *status) {
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *title_label = gtk_label_new(title);
    GtkWidget *subtitle_label = gtk_label_new(subtitle);
    gtk_widget_add_css_class(button, "utility-btn");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(subtitle_label), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(subtitle_label), TRUE);
    gtk_widget_add_css_class(title_label, "utility-title");
    gtk_widget_add_css_class(subtitle_label, "utility-subtitle");
    gtk_widget_set_focusable(button, TRUE);
    gtk_box_append(GTK_BOX(box), title_label);
    gtk_box_append(GTK_BOX(box), subtitle_label);
    gtk_button_set_child(GTK_BUTTON(button), box);
    if (cmd) {
        g_object_set_data_full(G_OBJECT(button), "cmd", g_strdup(cmd), g_free);
    }
    if (status) {
        g_object_set_data_full(G_OBJECT(button), "status", g_strdup(status), g_free);
    }
    g_signal_connect(button, "clicked", G_CALLBACK(gtk4_on_utility_clicked), state);
    return button;
}

static GtkWidget *gtk4_make_tool_button(const gchar *tool_id, const gchar *label, const gchar *icon_path) {
    GtkWidget *button = gtk_check_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *text = gtk_label_new(label);
    GtkWidget *icon = NULL;

    if (icon_path && g_file_test(icon_path, G_FILE_TEST_EXISTS)) {
        icon = gtk_picture_new_for_filename(icon_path);
        gtk_widget_set_size_request(icon, 16, 16);
    } else {
        icon = gtk_image_new_from_icon_name("applications-graphics-symbolic");
        gtk_widget_set_size_request(icon, 16, 16);
    }
    gtk_label_set_xalign(GTK_LABEL(text), 0.0f);
    gtk_box_append(GTK_BOX(box), icon);
    gtk_box_append(GTK_BOX(box), text);
    gtk_check_button_set_child(GTK_CHECK_BUTTON(button), box);
    g_object_set_data_full(G_OBJECT(button), "tool-id", g_strdup(tool_id), g_free);
    return button;
}

static GtkWidget *gtk4_make_quick_style_button(const gchar *style_id, const gchar *label, const gchar *icon_path) {
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *text = gtk_label_new(label);
    GtkWidget *icon = NULL;

    if (icon_path && g_file_test(icon_path, G_FILE_TEST_EXISTS)) {
        icon = gtk_picture_new_for_filename(icon_path);
        gtk_widget_set_size_request(icon, 36, 22);
    } else {
        icon = gtk_image_new_from_icon_name("applications-graphics-symbolic");
        gtk_widget_set_size_request(icon, 20, 20);
    }
    gtk_label_set_xalign(GTK_LABEL(text), 0.5f);
    gtk_widget_add_css_class(button, "lcu-panel");
    gtk_widget_set_size_request(button, 106, -1);
    gtk_box_append(GTK_BOX(box), icon);
    gtk_box_append(GTK_BOX(box), text);
    gtk_button_set_child(GTK_BUTTON(button), box);
    g_object_set_data_full(G_OBJECT(button), "style-id", g_strdup(style_id), g_free);
    return button;
}

static gchar *gtk4_trim_copy(const gchar *text) {
    gchar *copy = g_strdup(text ? text : "");
    g_strstrip(copy);
    return copy;
}

static gchar *gtk4_strip_backticks(const gchar *text) {
    GString *out = NULL;
    const gchar *p = NULL;
    if (!text) {
        return g_strdup("");
    }
    out = g_string_sized_new(strlen(text));
    for (p = text; *p; p += 1) {
        if (*p != '`') {
            g_string_append_c(out, *p);
        }
    }
    return g_string_free(out, FALSE);
}

static gboolean gtk4_table_cell_is_separator(const gchar *cell) {
    const gchar *p = cell;
    if (!cell || !*cell) {
        return FALSE;
    }
    while (p && *p) {
        if (*p != '-' && *p != ':' && *p != ' ') {
            return FALSE;
        }
        p += 1;
    }
    return TRUE;
}

static gboolean gtk4_shortcuts_line_is_table_separator(const gchar *line) {
    gchar **parts = NULL;
    gboolean saw_cell = FALSE;
    gboolean all_separators = TRUE;
    if (!line || line[0] != '|') {
        return FALSE;
    }
    parts = g_strsplit(line, "|", -1);
    for (gint i = 0; parts && parts[i]; i += 1) {
        gchar *cell = gtk4_trim_copy(parts[i]);
        if (cell[0] == '\0') {
            g_free(cell);
            continue;
        }
        saw_cell = TRUE;
        if (!gtk4_table_cell_is_separator(cell)) {
            all_separators = FALSE;
            g_free(cell);
            break;
        }
        g_free(cell);
    }
    g_strfreev(parts);
    return saw_cell && all_separators;
}

static gboolean gtk4_shortcuts_is_table_header_cell(const gchar *cell) {
    gchar *fold = NULL;
    gboolean is_header = FALSE;
    if (!cell || !*cell) {
        return FALSE;
    }
    fold = g_utf8_casefold(cell, -1);
    is_header = g_strcmp0(fold, "shortcut") == 0 ||
                g_strcmp0(fold, "key") == 0 ||
                g_strcmp0(fold, "command") == 0 ||
                g_strcmp0(fold, "input") == 0 ||
                g_strcmp0(fold, "widget input") == 0 ||
                g_strcmp0(fold, "hold while dragging") == 0 ||
                g_strcmp0(fold, "action") == 0 ||
                g_strcmp0(fold, "target") == 0;
    g_free(fold);
    return is_header;
}

static guint gtk4_shortcuts_parse_table_cells(const gchar *line, gchar **cells, guint max_cells) {
    gchar **parts = NULL;
    guint count = 0;
    if (!line || !cells || max_cells == 0) {
        return 0;
    }
    parts = g_strsplit(line, "|", -1);
    for (gint i = 0; parts && parts[i] && count < max_cells; i += 1) {
        gchar *trimmed = gtk4_trim_copy(parts[i]);
        if (trimmed[0] != '\0') {
            cells[count++] = gtk4_strip_backticks(trimmed);
        }
        g_free(trimmed);
    }
    g_strfreev(parts);
    return count;
}

static GtkWidget *gtk4_shortcuts_make_label(const gchar *text, const gchar *css_class, gboolean wrap) {
    GtkWidget *label = gtk_label_new(text ? text : "");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(label), wrap);
    gtk_label_set_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
    gtk_widget_set_hexpand(label, TRUE);
    if (css_class && *css_class) {
        gtk_widget_add_css_class(label, css_class);
    }
    return label;
}

static GtkWidget *gtk4_shortcuts_make_table_row(const gchar *left_text,
                                                const gchar *right_text,
                                                gboolean header) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *left = gtk4_shortcuts_make_label(left_text ? left_text : "", NULL, FALSE);
    GtkWidget *right = gtk4_shortcuts_make_label(right_text ? right_text : "", NULL, TRUE);
    gtk_widget_set_size_request(left, 240, -1);
    gtk_widget_set_halign(left, GTK_ALIGN_START);
    gtk_widget_set_hexpand(right, TRUE);
    gtk_label_set_selectable(GTK_LABEL(right), TRUE);
    gtk_widget_add_css_class(row, "shortcut-row");
    if (header) {
        gtk_widget_add_css_class(row, "shortcut-row-header");
        gtk_widget_add_css_class(left, "shortcut-key-head");
        gtk_widget_add_css_class(right, "shortcut-key-head");
    } else {
        gtk_widget_add_css_class(left, "shortcut-key-chip");
        gtk_widget_add_css_class(right, "shortcut-action");
    }
    gtk_box_append(GTK_BOX(row), left);
    gtk_box_append(GTK_BOX(row), right);
    return row;
}

static GtkWidget *gtk4_shortcuts_make_code_block(const gchar *text) {
    GtkWidget *label = gtk4_shortcuts_make_label(text ? text : "", "shortcut-code-block", FALSE);
    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    gtk_label_set_wrap(GTK_LABEL(label), FALSE);
    return label;
}

static void gtk4_shortcuts_append_markdown_widget(GtkWidget *content, const gchar *markdown) {
    gchar **lines = NULL;
    GtkWidget *table = NULL;
    gboolean in_code = FALSE;
    gboolean last_blank = FALSE;
    guint table_rows = 0;
    GString *code = NULL;

    if (!content) {
        return;
    }
    if (!markdown || !*markdown) {
        gtk_box_append(GTK_BOX(content),
                       gtk4_shortcuts_make_label("Shortcut cheat sheet not found.", "shortcut-note", TRUE));
        return;
    }

    lines = g_strsplit(markdown, "\n", -1);
    for (gint i = 0; lines && lines[i]; i += 1) {
        const gchar *line = lines[i];
        gchar *trimmed = gtk4_trim_copy(line);

        if (g_str_has_prefix(trimmed, "```")) {
            if (!in_code) {
                in_code = TRUE;
                if (code) {
                    g_string_free(code, TRUE);
                }
                code = g_string_new(NULL);
            } else {
                in_code = FALSE;
                if (code && code->len > 0) {
                    gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_code_block(code->str));
                }
                if (code) {
                    g_string_free(code, TRUE);
                    code = NULL;
                }
            }
            table = NULL;
            table_rows = 0;
            last_blank = FALSE;
            g_free(trimmed);
            continue;
        }

        if (in_code) {
            if (code) {
                g_string_append(code, line);
                g_string_append_c(code, '\n');
            }
            g_free(trimmed);
            continue;
        }

        if (trimmed[0] == '|' && trimmed[1] != '\0') {
            gchar *cells[4] = {NULL, NULL, NULL, NULL};
            guint count = 0;
            gboolean header = FALSE;
            if (gtk4_shortcuts_line_is_table_separator(trimmed)) {
                g_free(trimmed);
                continue;
            }
            if (!table) {
                table = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
                gtk_widget_add_css_class(table, "shortcut-table");
                gtk_box_append(GTK_BOX(content), table);
                table_rows = 0;
            }
            count = gtk4_shortcuts_parse_table_cells(trimmed, cells, G_N_ELEMENTS(cells));
            if (count >= 2) {
                header = (table_rows == 0) &&
                         (gtk4_shortcuts_is_table_header_cell(cells[0]) ||
                          gtk4_shortcuts_is_table_header_cell(cells[1]));
                gtk_box_append(GTK_BOX(table),
                               gtk4_shortcuts_make_table_row(cells[0], cells[1], header));
                table_rows += 1;
            }
            for (guint c = 0; c < G_N_ELEMENTS(cells); c += 1) {
                g_free(cells[c]);
            }
            last_blank = FALSE;
            g_free(trimmed);
            continue;
        }

        table = NULL;
        table_rows = 0;

        if (trimmed[0] == '\0') {
            if (!last_blank) {
                GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
                gtk_widget_set_size_request(spacer, -1, 4);
                gtk_box_append(GTK_BOX(content), spacer);
            }
            last_blank = TRUE;
            g_free(trimmed);
            continue;
        }

        if (g_str_has_prefix(trimmed, "# ")) {
            gchar *text = gtk4_strip_backticks(trimmed + 2);
            gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_label(text, "shortcut-h1", TRUE));
            g_free(text);
        } else if (g_str_has_prefix(trimmed, "## ")) {
            gchar *text = gtk4_strip_backticks(trimmed + 3);
            gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_label(text, "shortcut-h2", TRUE));
            g_free(text);
        } else if (g_str_has_prefix(trimmed, "### ")) {
            gchar *text = gtk4_strip_backticks(trimmed + 4);
            gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_label(text, "shortcut-h3", TRUE));
            g_free(text);
        } else if (g_str_has_prefix(trimmed, "- ") || g_str_has_prefix(trimmed, "* ")) {
            gchar *item = gtk4_strip_backticks(trimmed + 2);
            gchar *display = g_strdup_printf("• %s", item);
            gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_label(display, "shortcut-bullet", TRUE));
            g_free(display);
            g_free(item);
        } else {
            gsize len = strlen(trimmed);
            gchar *plain = gtk4_strip_backticks(trimmed);
            const gchar *css_class = (len > 0 && trimmed[len - 1] == ':')
                                         ? "shortcut-subtitle"
                                         : "shortcut-note";
            gtk_box_append(GTK_BOX(content), gtk4_shortcuts_make_label(plain, css_class, TRUE));
            g_free(plain);
        }

        last_blank = FALSE;
        g_free(trimmed);
    }

    if (code) {
        g_string_free(code, TRUE);
    }
    g_strfreev(lines);
}

static GtkWidget *gtk4_build_night_tab(Gtk4State *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new("Night Light Control");
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *status = gtk_label_new("Night Light ready.");
    GtkWidget *auto_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *auto_label = gtk_label_new("Auto Night Light");
    GtkWidget *auto_switch = gtk_switch_new();
    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1500, 6500, 100);
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *apply = gtk_button_new_with_label("Apply Manual");
    GtkWidget *warmer = gtk_button_new_with_label("Warmer");
    GtkWidget *cooler = gtk_button_new_with_label("Cooler");
    GtkWidget *disable = gtk_button_new_with_label("Turn Off");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(status), 0.0f);
    gtk_widget_add_css_class(panel, "lcu-panel");
    gtk_widget_add_css_class(title, "title-2");

    gtk_label_set_xalign(GTK_LABEL(auto_label), 0.0f);
    gtk_widget_set_hexpand(auto_label, TRUE);
    gtk_switch_set_active(GTK_SWITCH(auto_switch), TRUE);
    gtk_range_set_value(GTK_RANGE(scale), 4200);
    gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_LEFT);

    gtk_box_append(GTK_BOX(auto_row), auto_label);
    gtk_box_append(GTK_BOX(auto_row), auto_switch);
    gtk_box_append(GTK_BOX(buttons), apply);
    gtk_box_append(GTK_BOX(buttons), warmer);
    gtk_box_append(GTK_BOX(buttons), cooler);
    gtk_box_append(GTK_BOX(buttons), disable);
    gtk_box_append(GTK_BOX(buttons), refresh);

    gtk_box_append(GTK_BOX(panel), status);
    gtk_box_append(GTK_BOX(panel), auto_row);
    gtk_box_append(GTK_BOX(panel), scale);
    gtk_box_append(GTK_BOX(panel), buttons);

    gtk_box_append(GTK_BOX(root), title);
    gtk_box_append(GTK_BOX(root), panel);

    state->night_status = status;
    state->night_scale = scale;
    state->night_auto_switch = auto_switch;
    g_signal_connect(apply, "clicked", G_CALLBACK(gtk4_on_night_apply), state);
    g_signal_connect(warmer, "clicked", G_CALLBACK(gtk4_on_night_warmer), state);
    g_signal_connect(cooler, "clicked", G_CALLBACK(gtk4_on_night_cooler), state);
    g_signal_connect(disable, "clicked", G_CALLBACK(gtk4_on_night_disable), state);
    g_signal_connect(refresh, "clicked", G_CALLBACK(gtk4_on_night_refresh), state);
    g_signal_connect(auto_switch, "state-set", G_CALLBACK(gtk4_on_night_auto_toggled), state);
    return root;
}

static GtkWidget *gtk4_build_audio_tab(Gtk4State *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new("Audio Control");
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *status = gtk_label_new("Audio ready.");
    GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 150, 1);
    GtkWidget *buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *apply = gtk_button_new_with_label("Apply Volume");
    GtkWidget *minus = gtk_button_new_with_label("-5%");
    GtkWidget *plus = gtk_button_new_with_label("+5%");
    GtkWidget *mute = gtk_button_new_with_label("Mute / Unmute");
    GtkWidget *refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *mixer = gtk_button_new_with_label("Open Pavucontrol");

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_add_css_class(panel, "lcu-panel");
    gtk_widget_add_css_class(title, "title-2");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(status), 0.0f);
    gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
    gtk_range_set_value(GTK_RANGE(scale), 20);

    gtk_box_append(GTK_BOX(buttons), apply);
    gtk_box_append(GTK_BOX(buttons), minus);
    gtk_box_append(GTK_BOX(buttons), plus);
    gtk_box_append(GTK_BOX(buttons), mute);
    gtk_box_append(GTK_BOX(buttons), refresh);
    gtk_box_append(GTK_BOX(buttons), mixer);

    gtk_box_append(GTK_BOX(panel), status);
    gtk_box_append(GTK_BOX(panel), scale);
    gtk_box_append(GTK_BOX(panel), buttons);
    gtk_box_append(GTK_BOX(root), title);
    gtk_box_append(GTK_BOX(root), panel);

    state->audio_status = status;
    state->audio_scale = scale;
    g_object_set_data(G_OBJECT(minus), "delta", "-5");
    g_object_set_data(G_OBJECT(plus), "delta", "+5");
    g_signal_connect(apply, "clicked", G_CALLBACK(gtk4_on_audio_apply), state);
    g_signal_connect(minus, "clicked", G_CALLBACK(gtk4_on_audio_adjust), state);
    g_signal_connect(plus, "clicked", G_CALLBACK(gtk4_on_audio_adjust), state);
    g_signal_connect(mute, "clicked", G_CALLBACK(gtk4_on_audio_toggle_mute), state);
    g_signal_connect(refresh, "clicked", G_CALLBACK(gtk4_on_audio_refresh), state);
    g_signal_connect(mixer, "clicked", G_CALLBACK(gtk4_on_audio_open_mixer), state);
    return root;
}

static GtkWidget *gtk4_build_utilities_tab(Gtk4State *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new("Utilities");
    GtkWidget *subtitle = gtk_label_new("Fast launch pad for daily Linux tools, overlays, and animation helpers.");
    GtkWidget *scroll = gtk_scrolled_window_new();
    GtkWidget *grid = gtk_grid_new();
    gchar *launch_q = g_shell_quote(state->launch_dir ? state->launch_dir : ".");
    gchar *shots_q = g_shell_quote(state->shots_dir ? state->shots_dir : ".");
    gchar *cmd_workspace = g_strdup_printf("xdg-open %s", launch_q);
    gchar *cmd_screens = g_strdup_printf("xdg-open %s", shots_q);
    gchar *cmd_build_spot = gtk4_repo_shell(state, "./build_cursor_spotlight.sh");
    gchar *cmd_spot = gtk4_repo_shell(state, "./build/bin/cursor_spotlight --radius 180 --dim 0.68 --fps 50");
    gchar *cmd_anchor = gtk4_repo_shell(state, "./presenter_dash.sh anchor");
    gchar *cmd_dash = gtk4_repo_shell(state, "./presenter_dash.sh dash");
    gchar *cmd_dot = gtk4_repo_shell(state, "./presenter_dash.sh dot");
    gchar *cmd_arrow = gtk4_repo_shell(state, "./presenter_dash.sh arrow");
    gchar *cmd_install_gromit = gtk4_repo_shell(state, "./install_gromit_profile.sh");
    gchar *cmd_shortcuts = gtk4_repo_shell(state, "xdg-open SHORTCUTS_CHEATSHEET.md");
    gchar *cmd_manim_shell = gtk4_repo_shell(state, "./manim_tools.sh term-shell");
    gchar *cmd_manim_version = gtk4_repo_shell(state, "./manim_tools.sh term-version");
    gchar *cmd_manim_smoke = gtk4_repo_shell(state, "./manim_tools.sh term-smoke");
    GtkWidget *buttons[21];
    const gchar *titles[21] = {
        "Pavucontrol", "CC Switch", "Flameshot", "Network", "Bluetooth", "Workspace",
        "Screenshots", "Terminator", "Cursor Spotlight", "Build Spotlight", "Gromit Draw",
        "Gromit Clear", "Dash Anchor", "Dash Segment", "Dot Segment", "Arrow Segment",
        "Install Gromit Profile", "Shortcut Cheat Sheet", "Manim Workspace", "Manim Version", "Manim Smoke"
    };
    const gchar *subs[21] = {
        "Audio mixer and routing", "Project/context switch helper", "Capture area screenshot", "Connection editor and details",
        "Devices and pairing", "Open LinuxUtilities folder", "Open Screenshots folder", "Open terminal workspace",
        "Toggle cursor highlight overlay", "Compile spotlight binary", "Toggle screen drawing mode", "Clear all current strokes",
        "Set animated path anchor", "Animated dashed segment to cursor", "Animated dotted segment to cursor",
        "Animated arrow segment to cursor", "Install compatible draw profile", "Open full key/mouse/shell shortcuts",
        "Open ~/Workspace/manim with venv activated", "Run manim --version in terminal", "Render smoke.py Smoke in terminal"
    };
    const gchar *cmds[21] = {
        "pavucontrol", "bash -lc \"command -v cc-switch >/dev/null 2>&1 && cc-switch || true\"",
        "flameshot gui", "nm-connection-editor", "blueman-manager", NULL, NULL, "terminator",
        NULL, NULL, "gromit-mpx --toggle", "gromit-mpx --clear", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
    };

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_add_css_class(title, "title-2");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), grid);

    for (gint i = 0; i < 21; i += 1) {
        const gchar *command = cmds[i];
        if (i == 5) {
            command = cmd_workspace;
        } else if (i == 6) {
            command = cmd_screens;
        } else if (i == 8) {
            command = cmd_spot;
        } else if (i == 9) {
            command = cmd_build_spot;
        } else if (i == 12) {
            command = cmd_anchor;
        } else if (i == 13) {
            command = cmd_dash;
        } else if (i == 14) {
            command = cmd_dot;
        } else if (i == 15) {
            command = cmd_arrow;
        } else if (i == 16) {
            command = cmd_install_gromit;
        } else if (i == 17) {
            command = cmd_shortcuts;
        } else if (i == 18) {
            command = cmd_manim_shell;
        } else if (i == 19) {
            command = cmd_manim_version;
        } else if (i == 20) {
            command = cmd_manim_smoke;
        }
        buttons[i] = gtk4_make_utility_button(state, titles[i], subs[i], command, titles[i]);
        gtk_grid_attach(GTK_GRID(grid), buttons[i], i % 2, i / 2, 1, 1);
    }

    gtk_box_append(GTK_BOX(root), title);
    gtk_box_append(GTK_BOX(root), subtitle);
    gtk_box_append(GTK_BOX(root), scroll);

    g_free(launch_q);
    g_free(shots_q);
    g_free(cmd_workspace);
    g_free(cmd_screens);
    g_free(cmd_build_spot);
    g_free(cmd_spot);
    g_free(cmd_anchor);
    g_free(cmd_dash);
    g_free(cmd_dot);
    g_free(cmd_arrow);
    g_free(cmd_install_gromit);
    g_free(cmd_shortcuts);
    g_free(cmd_manim_shell);
    g_free(cmd_manim_version);
    g_free(cmd_manim_smoke);
    return root;
}

static GtkWidget *gtk4_build_shortcuts_tab(Gtk4State *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new("Shortcut Playbook");
    GtkWidget *subtitle = gtk_label_new("Rendered keymaps for presenter flow, capture flow, and workspace commands.");
    GtkWidget *panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *scroller = gtk_scrolled_window_new();
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gchar *file = g_build_filename(state->launch_dir ? state->launch_dir : ".", "SHORTCUTS_CHEATSHEET.md", NULL);
    gchar *contents = NULL;
    gsize len = 0;

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_add_css_class(panel, "lcu-panel");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(subtitle), TRUE);
    gtk_widget_add_css_class(subtitle, "dim-label");
    gtk_widget_add_css_class(content, "shortcut-content");
    gtk_widget_set_margin_top(content, 6);
    gtk_widget_set_margin_bottom(content, 6);
    gtk_widget_set_margin_start(content, 8);
    gtk_widget_set_margin_end(content, 8);
    gtk_widget_set_hexpand(content, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), content);

    if (g_file_get_contents(file, &contents, &len, NULL) && contents && len > 0) {
        gtk4_shortcuts_append_markdown_widget(content, contents);
    } else {
        gtk4_shortcuts_append_markdown_widget(
            content,
            "Shortcut cheat sheet not found.\n\n"
            "- Use Utilities -> Shortcut Cheat Sheet to open the markdown file.\n"
            "- Expected file: `SHORTCUTS_CHEATSHEET.md`");
    }

    gtk_box_append(GTK_BOX(panel), scroller);
    gtk_box_append(GTK_BOX(root), title);
    gtk_box_append(GTK_BOX(root), subtitle);
    gtk_box_append(GTK_BOX(root), panel);
    g_free(file);
    g_free(contents);
    return root;
}

static void gtk4_on_capture_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_capture_new(user_data);
}

static void gtk4_on_edit_selected_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_edit_selected(user_data);
}

static void gtk4_on_editor_undo_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_undo_last(user_data);
}

static void gtk4_on_editor_clear_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_clear_marks(user_data);
}

static void gtk4_on_editor_save_png_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_save_annotated(user_data);
}

static void gtk4_on_editor_save_svg_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_editor_save_svg(user_data);
}

static void gtk4_on_editor_stroke_changed(GtkRange *range, gpointer user_data) {
    Gtk4State *state = user_data;
    if (!state || !range) {
        return;
    }
    state->editor_stroke_width = gtk_range_get_value(range);
}

static void gtk4_on_editor_color_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *hex = g_object_get_data(G_OBJECT(button), "color-hex");
    const gchar *name = g_object_get_data(G_OBJECT(button), "color-name");
    gtk4_set_editor_color(state, hex ? hex : "#ef5b5b", name ? name : "Custom");
}

static void gtk4_on_refresh_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gtk4_reload(user_data);
}

static void gtk4_on_open_folder_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    gchar *quoted = NULL;
    gchar *cmd = NULL;
    (void)button;
    if (!state || !state->shots_dir) {
        return;
    }
    quoted = g_shell_quote(state->shots_dir);
    cmd = g_strdup_printf("xdg-open %s", quoted);
    gtk4_spawn_with_feedback(state, cmd, "Opening screenshots folder...");
    g_free(cmd);
    g_free(quoted);
}

static void gtk4_on_delete_clicked(GtkButton *button, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)button;
    gtk4_delete_selected(state);
    gtk4_reload(state);
    gtk4_clear_editor(state);
}

static void gtk4_on_search_changed(GtkEditable *editable, gpointer user_data) {
    (void)editable;
    gtk4_reload(user_data);
}

static void gtk4_on_thumb_selection_changed(GtkFlowBox *box, gpointer user_data) {
    Gtk4State *state = user_data;
    GPtrArray *paths = NULL;
    (void)box;
    gtk4_update_selection_ui(state);
    paths = gtk4_selected_paths(state);
    if (paths->len == 1) {
        gtk4_editor_load_image(state, g_ptr_array_index(paths, 0));
    }
    g_ptr_array_free(paths, TRUE);
}

static void gtk4_on_thumb_activated(GtkFlowBox *box, GtkFlowBoxChild *child, gpointer user_data) {
    Gtk4State *state = user_data;
    GtkWidget *row = NULL;
    const gchar *path = NULL;
    (void)box;
    if (!state || !child) {
        return;
    }
    row = gtk_flow_box_child_get_child(child);
    path = row ? g_object_get_data(G_OBJECT(row), "shot-path") : NULL;
    if (path) {
        gtk4_editor_load_image(state, path);
    }
}

static gboolean gtk4_on_window_key(GtkEventControllerKey *controller,
                                   guint keyval,
                                   guint keycode,
                                   GdkModifierType state,
                                   gpointer user_data) {
    Gtk4State *app = user_data;
    (void)controller;
    (void)keycode;
    if (!app || !app->thumb_flow) {
        return FALSE;
    }
    if (app->notebook && gtk_notebook_get_current_page(GTK_NOTEBOOK(app->notebook)) != app->screenshots_page) {
        return FALSE;
    }

    if ((state & GDK_CONTROL_MASK) && (keyval == GDK_KEY_a || keyval == GDK_KEY_A)) {
        for (GtkWidget *child = gtk_widget_get_first_child(app->thumb_flow); child; child = gtk_widget_get_next_sibling(child)) {
            if (GTK_IS_FLOW_BOX_CHILD(child)) {
                gtk_flow_box_select_child(GTK_FLOW_BOX(app->thumb_flow), GTK_FLOW_BOX_CHILD(child));
            }
        }
        gtk4_update_selection_ui(app);
        return TRUE;
    }
    if ((state & GDK_CONTROL_MASK) && (keyval == GDK_KEY_z || keyval == GDK_KEY_Z)) {
        gtk4_editor_undo_last(app);
        return TRUE;
    }
    if ((state & GDK_CONTROL_MASK) && (keyval == GDK_KEY_s || keyval == GDK_KEY_S)) {
        gtk4_editor_save_annotated(app);
        return TRUE;
    }
    if (keyval == GDK_KEY_1) {
        gtk4_set_active_tool(app, GTK4_TOOL_SELECT);
        return TRUE;
    }
    if (keyval == GDK_KEY_2) {
        gtk4_set_active_tool(app, GTK4_TOOL_ARROW);
        return TRUE;
    }
    if (keyval == GDK_KEY_3) {
        gtk4_set_active_tool(app, GTK4_TOOL_LINE);
        return TRUE;
    }
    if (keyval == GDK_KEY_4) {
        gtk4_set_active_tool(app, GTK4_TOOL_RECT);
        return TRUE;
    }
    if (keyval == GDK_KEY_5) {
        gtk4_set_active_tool(app, GTK4_TOOL_CALLOUT);
        return TRUE;
    }
    if (keyval == GDK_KEY_6) {
        gtk4_set_active_tool(app, GTK4_TOOL_TEXT);
        return TRUE;
    }
    if (keyval == GDK_KEY_7) {
        gtk4_set_active_tool(app, GTK4_TOOL_STAMP);
        return TRUE;
    }
    if (keyval == GDK_KEY_Escape) {
        gtk_flow_box_unselect_all(GTK_FLOW_BOX(app->thumb_flow));
        gtk4_update_selection_ui(app);
        return TRUE;
    }
    if (keyval == GDK_KEY_Delete || keyval == GDK_KEY_KP_Delete) {
        gtk4_delete_selected(app);
        gtk4_reload(app);
        gtk4_clear_editor(app);
        return TRUE;
    }

    return FALSE;
}

static GtkWidget *gtk4_build_screenshots_tab(Gtk4State *state) {
    static const char *thumb_presets[] = { "Small", "Medium", "Large", NULL };
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *title = gtk_label_new("Screenshot Studio");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *btn_capture = gtk_button_new_with_label("Capture New");
    GtkWidget *btn_edit = gtk_button_new_with_label("Edit Selected");
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_open_folder = gtk_button_new_with_label("Open Folder");
    GtkWidget *btn_delete = gtk_button_new_with_label("Delete Selected");
    GtkWidget *search = gtk_search_entry_new();
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *studio_shell = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *editor_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *editor_top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *editor_top_actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *btn_dock_toggle = gtk_button_new_with_label("Dock Left");
    GtkWidget *btn_toggle_styles = gtk_button_new_with_label("Hide Styles");
    GtkWidget *btn_toggle_thumbs = gtk_button_new_with_label("Hide Thumbs");
    GtkWidget *btn_thumbs_bigger = gtk_button_new_with_label("Thumbs +");
    GtkWidget *btn_thumbs_smaller = gtk_button_new_with_label("Thumbs -");
    GtkWidget *btn_zoom_reset = gtk_button_new_with_label("Fit 100%");
    GtkWidget *tool_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *quick_styles_scroller = gtk_scrolled_window_new();
    GtkWidget *quick_styles_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *style_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *style_label = gtk_label_new("Style");
    GtkWidget *stroke_label = gtk_label_new("Stroke");
    GtkWidget *stroke_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 14.0, 1.0);
    GtkWidget *color_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *editor_path = gtk_label_new("No image selected");
    GtkWidget *editor_area = gtk_drawing_area_new();
    GtkWidget *editor_status = gtk_label_new("Editor ready.");
    GtkWidget *editor_dock = gtk_scrolled_window_new();
    GtkWidget *editor_dock_body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *dock_title = gtk_label_new("Actions");
    GtkWidget *dock_capture = gtk_button_new_with_label("Capture");
    GtkWidget *dock_edit = gtk_button_new_with_label("Edit Selected");
    GtkWidget *dock_undo = gtk_button_new_with_label("Undo");
    GtkWidget *dock_clear = gtk_button_new_with_label("Clear");
    GtkWidget *dock_save_png = gtk_button_new_with_label("Save Annotated Copy");
    GtkWidget *dock_save_svg = gtk_button_new_with_label("Save SVG Copy");
    GtkWidget *dock_help = gtk_label_new("Shortcuts: 1 Select, 2 Arrow, 3 Line, 4 Rect, 5 Callout, 6 Text, 7 Stamp, Ctrl+Z undo, Ctrl+S save PNG.");
    GtkWidget *props_sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *props_title = gtk_label_new("Arrow properties");
    GtkWidget *props_stack = gtk_stack_new();
    GtkWidget *props_select = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_arrow = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_line = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_rect = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_callout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_stamp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *arrow_style_combo = gtk_drop_down_new_from_strings(GTK4_ARROW_STYLE_LABELS);
    GtkWidget *arrow_head_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 6.0, 72.0, 1.0);
    GtkWidget *arrow_angle_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 10.0, 80.0, 1.0);
    GtkWidget *arrow_shadow_check = gtk_check_button_new_with_label("Arrow shadow");
    GtkWidget *arrow_shadow_offset_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 24.0, 0.5);
    GtkWidget *rect_round_check = gtk_check_button_new_with_label("Rounded corners");
    GtkWidget *rect_fill_check = gtk_check_button_new_with_label("Fill rectangle");
    GtkWidget *rect_fill_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.05, 0.95, 0.01);
    GtkWidget *rect_shadow_check = gtk_check_button_new_with_label("Rectangle shadow");
    GtkWidget *callout_style_combo = gtk_drop_down_new_from_strings(GTK4_CALLOUT_STYLE_LABELS);
    GtkWidget *callout_fill_check = gtk_check_button_new_with_label("Fill callout");
    GtkWidget *callout_fill_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.05, 0.95, 0.01);
    GtkWidget *callout_shadow_check = gtk_check_button_new_with_label("Callout shadow");
    GtkWidget *stamp_combo = gtk_drop_down_new_from_strings(GTK4_STAMP_LABELS);
    GtkWidget *auto_step_check = gtk_check_button_new_with_label("Auto step numbering");
    GtkWidget *link_step_check = gtk_check_button_new_with_label("Link steps with arrows");
    GtkWidget *step_label = gtk_label_new("Next step: 1");
    GtkWidget *text_entry = gtk_entry_new();
    GtkWidget *btn_apply_text = gtk_button_new_with_label("Apply Text");
    GtkWidget *font_style_combo = gtk_drop_down_new_from_strings(GTK4_FONT_STYLE_LABELS);
    GtkWidget *font_size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 10.0, 72.0, 1.0);
    GtkWidget *text_stroke_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 8.0, 0.5);
    GtkWidget *text_fill_check = gtk_check_button_new_with_label("Text fill");
    GtkWidget *text_stroke_check = gtk_check_button_new_with_label("Text stroke");
    GtkWidget *text_shadow_check = gtk_check_button_new_with_label("Text shadow");
    GtkWidget *browser_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *browser_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *thumb_controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *thumb_size_label = gtk_label_new("Thumb size");
    GtkWidget *thumb_preset_label = gtk_label_new("Preset");
    GtkWidget *thumb_preset_dropdown = gtk_drop_down_new_from_strings(thumb_presets);
    GtkWidget *thumb_size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 170.0, 440.0, 10.0);
    GtkWidget *thumb_scroll = gtk_scrolled_window_new();
    GtkWidget *thumb_flow = gtk_flow_box_new();
    GtkWidget *selected = gtk_label_new("No images selected.");
    GtkWidget *paths_scroll = gtk_scrolled_window_new();
    GtkWidget *paths_view = gtk_text_view_new();
    GtkWidget *tool_select = NULL;
    GtkWidget *tool_arrow = NULL;
    GtkWidget *tool_line = NULL;
    GtkWidget *tool_rect = NULL;
    GtkWidget *tool_callout = NULL;
    GtkWidget *tool_text = NULL;
    GtkWidget *tool_stamp = NULL;
    GtkWidget *qs_arrow_classic = NULL;
    GtkWidget *qs_arrow_bold = NULL;
    GtkWidget *qs_arrow_pointer = NULL;
    GtkWidget *qs_arrow_dashed = NULL;
    GtkWidget *qs_arrow_dotted = NULL;
    GtkWidget *qs_arrow_double = NULL;
    GtkWidget *qs_callout_red = NULL;
    GtkWidget *qs_callout_green = NULL;
    GtkWidget *qs_callout_right = NULL;
    GtkWidget *qs_callout_highlight = NULL;
    GtkWidget *qs_callout_shadow = NULL;
    GtkWidget *qs_rect_rounded = NULL;
    GtkWidget *qs_rect_shadow = NULL;
    GtkWidget *qs_step_badge = NULL;
    GtkWidget *qs_step_badge_blue = NULL;
    GtkWidget *qs_step_badge_linked = NULL;
    GtkWidget *qs_text_bold = NULL;
    GtkWidget *qs_text_outline = NULL;
    GtkWidget *status = gtk_label_new("Screenshot tab ready.");
    GtkEventController *motion = NULL;
    GtkEventController *scroll = NULL;
    GtkGesture *click = NULL;
    GtkEventController *key = NULL;
    gchar *icons_dir = NULL;
    gchar *icon_select = NULL;
    gchar *icon_arrow = NULL;
    gchar *icon_line = NULL;
    gchar *icon_rect = NULL;
    gchar *icon_callout = NULL;
    gchar *icon_text = NULL;
    gchar *icon_step = NULL;
    gchar *styles_dir = NULL;

    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);
    gtk_paned_set_wide_handle(GTK_PANED(split), TRUE);
    gtk_paned_set_wide_handle(GTK_PANED(studio_shell), TRUE);
    gtk_paned_set_resize_start_child(GTK_PANED(split), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(split), TRUE);
    gtk_paned_set_shrink_start_child(GTK_PANED(split), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(split), TRUE);
    gtk_paned_set_resize_start_child(GTK_PANED(studio_shell), TRUE);
    gtk_paned_set_resize_end_child(GTK_PANED(studio_shell), FALSE);
    gtk_paned_set_shrink_start_child(GTK_PANED(studio_shell), TRUE);
    gtk_paned_set_shrink_end_child(GTK_PANED(studio_shell), FALSE);

    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_widget_add_css_class(title, "title-2");
    gtk_widget_add_css_class(toolbar, "lcu-panel");
    gtk_widget_add_css_class(editor_dock, "lcu-surface");
    gtk_widget_add_css_class(editor_dock_body, "lcu-panel");
    gtk_widget_add_css_class(browser_box, "lcu-panel");
    gtk_widget_add_css_class(editor_box, "lcu-panel");
    gtk_label_set_xalign(GTK_LABEL(status), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(selected), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(editor_path), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(editor_status), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(style_label), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(stroke_label), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(thumb_size_label), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(thumb_preset_label), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(dock_title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(props_title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(dock_help), 0.0f);
    gtk_widget_set_hexpand(editor_top_row, TRUE);
    gtk_widget_set_halign(editor_top_actions, GTK_ALIGN_END);
    gtk_label_set_wrap(GTK_LABEL(dock_help), TRUE);
    gtk_widget_set_hexpand(browser_box, TRUE);
    gtk_widget_set_vexpand(browser_box, TRUE);
    gtk_widget_set_size_request(browser_box, -1, 220);
    gtk_widget_set_tooltip_text(split, "Drag the horizontal divider to resize editor and thumbnail browser.");
    gtk_widget_set_tooltip_text(studio_shell, "Drag the vertical divider to resize canvas and properties dock.");
    gtk_scale_set_draw_value(GTK_SCALE(thumb_size_scale), TRUE);
    gtk_widget_set_hexpand(thumb_size_scale, TRUE);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(thumb_preset_dropdown), 1);

    gtk_editable_set_text(GTK_EDITABLE(search), "");
    gtk_search_entry_set_placeholder_text(GTK_SEARCH_ENTRY(search), "Filter thumbnails by filename...");
    gtk_widget_set_hexpand(search, TRUE);
    gtk_box_append(GTK_BOX(toolbar), btn_capture);
    gtk_box_append(GTK_BOX(toolbar), btn_edit);
    gtk_box_append(GTK_BOX(toolbar), btn_open_folder);
    gtk_box_append(GTK_BOX(toolbar), btn_delete);
    gtk_box_append(GTK_BOX(toolbar), btn_refresh);
    gtk_box_append(GTK_BOX(toolbar), search);

    gtk_box_append(GTK_BOX(editor_top_actions), btn_dock_toggle);
    gtk_box_append(GTK_BOX(editor_top_actions), btn_toggle_styles);
    gtk_box_append(GTK_BOX(editor_top_actions), btn_toggle_thumbs);
    gtk_box_append(GTK_BOX(editor_top_actions), btn_thumbs_bigger);
    gtk_box_append(GTK_BOX(editor_top_actions), btn_thumbs_smaller);
    gtk_box_append(GTK_BOX(editor_top_actions), btn_zoom_reset);
    gtk_box_append(GTK_BOX(editor_top_row), tool_row);
    gtk_box_append(GTK_BOX(editor_top_row), editor_top_actions);

    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(thumb_flow), GTK_SELECTION_MULTIPLE);
    gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(thumb_flow), FALSE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(thumb_flow), 2);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(thumb_flow), 6);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(thumb_flow), 10);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(thumb_flow), 10);
    gtk_widget_set_focusable(thumb_flow, TRUE);
    gtk_widget_set_tooltip_text(thumb_flow, "Ctrl+A select all, Delete remove selected.");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(thumb_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(thumb_scroll, -1, 220);
    gtk_widget_set_vexpand(thumb_scroll, TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(thumb_scroll), thumb_flow);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(paths_view), GTK_WRAP_CHAR);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(paths_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(paths_scroll, -1, 56);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(paths_scroll), paths_view);

    gtk_box_append(GTK_BOX(thumb_controls), thumb_size_label);
    gtk_box_append(GTK_BOX(thumb_controls), thumb_size_scale);
    gtk_box_append(GTK_BOX(thumb_controls), thumb_preset_label);
    gtk_box_append(GTK_BOX(thumb_controls), thumb_preset_dropdown);
    gtk_box_append(GTK_BOX(browser_info), selected);
    gtk_box_append(GTK_BOX(browser_info), thumb_controls);
    gtk_box_append(GTK_BOX(browser_info), paths_scroll);
    gtk_box_append(GTK_BOX(browser_box), thumb_scroll);
    gtk_box_append(GTK_BOX(browser_box), browser_info);

    icons_dir = g_build_filename(state->launch_dir ? state->launch_dir : ".", "assets", "icons", NULL);
    icon_select = g_build_filename(icons_dir, "tool-select.svg", NULL);
    icon_arrow = g_build_filename(icons_dir, "tool-arrow.svg", NULL);
    icon_line = g_build_filename(icons_dir, "tool-line.svg", NULL);
    icon_rect = g_build_filename(icons_dir, "tool-rect.svg", NULL);
    icon_callout = g_build_filename(icons_dir, "tool-callout.svg", NULL);
    icon_text = g_build_filename(icons_dir, "tool-text.svg", NULL);
    icon_step = g_build_filename(icons_dir, "tool-step.svg", NULL);
    tool_select = gtk4_make_tool_button("select", "Select", icon_select);
    tool_arrow = gtk4_make_tool_button("arrow", "Arrow", icon_arrow);
    tool_line = gtk4_make_tool_button("line", "Line", icon_line);
    tool_rect = gtk4_make_tool_button("rect", "Rect", icon_rect);
    tool_callout = gtk4_make_tool_button("callout", "Callout", icon_callout);
    tool_text = gtk4_make_tool_button("text", "Text", icon_text);
    tool_stamp = gtk4_make_tool_button("stamp", "Step", icon_step);
    g_free(icon_select);
    g_free(icon_arrow);
    g_free(icon_line);
    g_free(icon_rect);
    g_free(icon_callout);
    g_free(icon_text);
    g_free(icon_step);
    g_free(icons_dir);

    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_arrow), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_line), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_rect), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_callout), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_text), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_stamp), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(tool_arrow), TRUE);
    gtk_box_append(GTK_BOX(tool_row), tool_select);
    gtk_box_append(GTK_BOX(tool_row), tool_arrow);
    gtk_box_append(GTK_BOX(tool_row), tool_line);
    gtk_box_append(GTK_BOX(tool_row), tool_rect);
    gtk_box_append(GTK_BOX(tool_row), tool_callout);
    gtk_box_append(GTK_BOX(tool_row), tool_text);
    gtk_box_append(GTK_BOX(tool_row), tool_stamp);
    gtk_widget_set_hexpand(tool_row, TRUE);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(quick_styles_scroller), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(quick_styles_scroller), quick_styles_row);
    gtk_widget_set_hexpand(quick_styles_scroller, TRUE);
    gtk_widget_set_size_request(quick_styles_scroller, -1, 112);
    {
        gchar *style_arrow_classic = NULL;
        gchar *style_arrow_bold = NULL;
        gchar *style_arrow_pointer = NULL;
        gchar *style_arrow_dashed = NULL;
        gchar *style_arrow_dotted = NULL;
        gchar *style_arrow_double = NULL;
        gchar *style_callout_red = NULL;
        gchar *style_callout_green = NULL;
        gchar *style_callout_right = NULL;
        gchar *style_callout_highlight = NULL;
        gchar *style_callout_shadow = NULL;
        gchar *style_rect_rounded = NULL;
        gchar *style_rect_shadow = NULL;
        gchar *style_step_badge = NULL;
        gchar *style_step_badge_blue = NULL;
        gchar *style_step_badge_linked = NULL;
        gchar *style_text = NULL;

        styles_dir = g_build_filename(state->launch_dir ? state->launch_dir : ".", "assets", "svg", NULL);
        style_arrow_classic = g_build_filename(styles_dir, "icons", "tool-arrow-classic.svg", NULL);
        style_arrow_bold = g_build_filename(styles_dir, "icons", "tool-arrow-bold.svg", NULL);
        style_arrow_pointer = g_build_filename(styles_dir, "icons", "tool-arrow-classic.svg", NULL);
        style_arrow_dashed = g_build_filename(styles_dir, "blocks", "block-dashed.svg", NULL);
        style_arrow_dotted = g_build_filename(styles_dir, "icons", "tool-arrow-dotted.svg", NULL);
        style_arrow_double = g_build_filename(styles_dir, "icons", "tool-arrow-double.svg", NULL);
        style_callout_red = g_build_filename(styles_dir, "callouts", "callout-left.svg", NULL);
        style_callout_green = g_build_filename(styles_dir, "callouts", "callout-top.svg", NULL);
        style_callout_right = g_build_filename(styles_dir, "callouts", "callout-right.svg", NULL);
        style_callout_highlight = g_build_filename(styles_dir, "callouts", "highlight-pill.svg", NULL);
        style_callout_shadow = g_build_filename(styles_dir, "callouts", "box-shadow.svg", NULL);
        style_rect_rounded = g_build_filename(styles_dir, "blocks", "block-rounded.svg", NULL);
        style_rect_shadow = g_build_filename(styles_dir, "blocks", "block-shadow.svg", NULL);
        style_step_badge = g_build_filename(styles_dir, "callouts", "step-badge.svg", NULL);
        style_step_badge_blue = g_build_filename(styles_dir, "callouts", "step-badge-blue.svg", NULL);
        style_step_badge_linked = g_build_filename(styles_dir, "callouts", "step-badge-link.svg", NULL);
        style_text = g_build_filename(styles_dir, "icons", "tool-text.svg", NULL);

        qs_arrow_classic = gtk4_make_quick_style_button("arrow_classic", "Classic", style_arrow_classic);
        qs_arrow_bold = gtk4_make_quick_style_button("arrow_bold", "Bold", style_arrow_bold);
        qs_arrow_pointer = gtk4_make_quick_style_button("arrow_pointer", "Pointer", style_arrow_pointer);
        qs_arrow_dashed = gtk4_make_quick_style_button("arrow_dashed", "Dashed", style_arrow_dashed);
        qs_arrow_dotted = gtk4_make_quick_style_button("arrow_dotted", "Dotted", style_arrow_dotted);
        qs_arrow_double = gtk4_make_quick_style_button("arrow_double", "Double", style_arrow_double);
        qs_callout_red = gtk4_make_quick_style_button("callout_red", "Callout Red", style_callout_red);
        qs_callout_green = gtk4_make_quick_style_button("callout_green", "Callout Green", style_callout_green);
        qs_callout_right = gtk4_make_quick_style_button("callout_right", "Callout Right", style_callout_right);
        qs_callout_highlight = gtk4_make_quick_style_button("callout_highlight", "Highlight", style_callout_highlight);
        qs_callout_shadow = gtk4_make_quick_style_button("callout_shadow", "Callout Shadow", style_callout_shadow);
        qs_rect_rounded = gtk4_make_quick_style_button("rect_rounded", "Rect Rounded", style_rect_rounded);
        qs_rect_shadow = gtk4_make_quick_style_button("rect_shadow", "Rect Shadow", style_rect_shadow);
        qs_step_badge = gtk4_make_quick_style_button("step_badge", "Step Red", style_step_badge);
        qs_step_badge_blue = gtk4_make_quick_style_button("step_badge_blue", "Step Blue", style_step_badge_blue);
        qs_step_badge_linked = gtk4_make_quick_style_button("step_badge_linked", "Step Link", style_step_badge_linked);
        qs_text_bold = gtk4_make_quick_style_button("text_bold", "Text Bold", style_text);
        qs_text_outline = gtk4_make_quick_style_button("text_outline", "Text Out", style_text);

        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_classic);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_bold);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_pointer);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_dashed);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_dotted);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_arrow_double);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_callout_red);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_callout_green);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_callout_right);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_callout_highlight);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_callout_shadow);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_rect_rounded);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_rect_shadow);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_step_badge);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_step_badge_blue);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_step_badge_linked);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_text_bold);
        gtk_box_append(GTK_BOX(quick_styles_row), qs_text_outline);

        g_free(style_arrow_classic);
        g_free(style_arrow_bold);
        g_free(style_arrow_pointer);
        g_free(style_arrow_dashed);
        g_free(style_arrow_dotted);
        g_free(style_arrow_double);
        g_free(style_callout_red);
        g_free(style_callout_green);
        g_free(style_callout_right);
        g_free(style_callout_highlight);
        g_free(style_callout_shadow);
        g_free(style_rect_rounded);
        g_free(style_rect_shadow);
        g_free(style_step_badge);
        g_free(style_step_badge_blue);
        g_free(style_step_badge_linked);
        g_free(style_text);
        g_free(styles_dir);
    }

    gtk_range_set_value(GTK_RANGE(stroke_scale), 4.0);
    gtk_scale_set_draw_value(GTK_SCALE(stroke_scale), TRUE);
    gtk_widget_set_hexpand(stroke_scale, TRUE);
    gtk_box_append(GTK_BOX(style_row), style_label);
    gtk_box_append(GTK_BOX(style_row), stroke_label);
    gtk_box_append(GTK_BOX(style_row), stroke_scale);
    gtk_box_append(GTK_BOX(style_row), color_row);
    {
        const struct {
            const gchar *name;
            const gchar *hex;
            const gchar *label;
        } colors[] = {
            { "Red", "#ef5b5b", "Red" },
            { "Cyan", "#4dd3ff", "Cyan" },
            { "Yellow", "#ffd84d", "Yellow" },
            { "Green", "#6edc6e", "Green" },
            { "White", "#f8f9ff", "White" }
        };
        for (guint i = 0; i < G_N_ELEMENTS(colors); i += 1) {
            GtkWidget *btn = gtk_button_new_with_label(colors[i].label);
            g_object_set_data_full(G_OBJECT(btn), "color-hex", g_strdup(colors[i].hex), g_free);
            g_object_set_data_full(G_OBJECT(btn), "color-name", g_strdup(colors[i].name), g_free);
            gtk_box_append(GTK_BOX(color_row), btn);
            g_signal_connect(btn, "clicked", G_CALLBACK(gtk4_on_editor_color_clicked), state);
        }
    }

    gtk_stack_set_transition_type(GTK_STACK(props_stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(props_stack), 120);
    {
        GtkWidget *l1 = gtk_label_new("Pick an object to move/inspect.");
        GtkWidget *l2 = gtk_label_new("Selection mode does not draw.");
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_box_append(GTK_BOX(props_select), l1);
        gtk_box_append(GTK_BOX(props_select), l2);
    }
    {
        GtkWidget *l1 = gtk_label_new("Arrow preset");
        GtkWidget *l2 = gtk_label_new("Arrow head size");
        GtkWidget *l3 = gtk_label_new("Arrow head angle");
        GtkWidget *l4 = gtk_label_new("Arrow shadow offset");
        GtkWidget *l5 = gtk_label_new("Use Stroke slider for arrow width.");
        gtk_drop_down_set_selected(GTK_DROP_DOWN(arrow_style_combo), 0);
        gtk_range_set_value(GTK_RANGE(arrow_head_scale), 16.0);
        gtk_range_set_value(GTK_RANGE(arrow_angle_scale), 26.0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arrow_shadow_check), FALSE);
        gtk_range_set_value(GTK_RANGE(arrow_shadow_offset_scale), 3.0);
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l3), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l4), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l5), 0.0f);
        gtk_scale_set_draw_value(GTK_SCALE(arrow_head_scale), TRUE);
        gtk_scale_set_draw_value(GTK_SCALE(arrow_angle_scale), TRUE);
        gtk_scale_set_draw_value(GTK_SCALE(arrow_shadow_offset_scale), TRUE);
        gtk_box_append(GTK_BOX(props_arrow), l1);
        gtk_box_append(GTK_BOX(props_arrow), arrow_style_combo);
        gtk_box_append(GTK_BOX(props_arrow), l2);
        gtk_box_append(GTK_BOX(props_arrow), arrow_head_scale);
        gtk_box_append(GTK_BOX(props_arrow), l3);
        gtk_box_append(GTK_BOX(props_arrow), arrow_angle_scale);
        gtk_box_append(GTK_BOX(props_arrow), arrow_shadow_check);
        gtk_box_append(GTK_BOX(props_arrow), l4);
        gtk_box_append(GTK_BOX(props_arrow), arrow_shadow_offset_scale);
        gtk_box_append(GTK_BOX(props_arrow), l5);
    }
    {
        GtkWidget *l1 = gtk_label_new("Line: straight segment.");
        GtkWidget *l2 = gtk_label_new("Use Stroke slider for thickness.");
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_box_append(GTK_BOX(props_line), l1);
        gtk_box_append(GTK_BOX(props_line), l2);
    }
    {
        GtkWidget *l1 = gtk_label_new("Rectangle options");
        GtkWidget *l2 = gtk_label_new("Rectangle fill opacity");
        GtkWidget *l3 = gtk_label_new("Use Stroke slider for border width.");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_round_check), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_fill_check), FALSE);
        gtk_range_set_value(GTK_RANGE(rect_fill_scale), 0.24);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_shadow_check), FALSE);
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l3), 0.0f);
        gtk_scale_set_draw_value(GTK_SCALE(rect_fill_scale), TRUE);
        gtk_box_append(GTK_BOX(props_rect), l1);
        gtk_box_append(GTK_BOX(props_rect), rect_round_check);
        gtk_box_append(GTK_BOX(props_rect), rect_fill_check);
        gtk_box_append(GTK_BOX(props_rect), l2);
        gtk_box_append(GTK_BOX(props_rect), rect_fill_scale);
        gtk_box_append(GTK_BOX(props_rect), rect_shadow_check);
        gtk_box_append(GTK_BOX(props_rect), l3);
    }
    {
        GtkWidget *l1 = gtk_label_new("Callout style");
        GtkWidget *l2 = gtk_label_new("Callout fill opacity");
        GtkWidget *l3 = gtk_label_new("Use Stroke slider for outline width.");
        gtk_drop_down_set_selected(GTK_DROP_DOWN(callout_style_combo), 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(callout_fill_check), FALSE);
        gtk_range_set_value(GTK_RANGE(callout_fill_scale), 0.24);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(callout_shadow_check), FALSE);
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l3), 0.0f);
        gtk_scale_set_draw_value(GTK_SCALE(callout_fill_scale), TRUE);
        gtk_box_append(GTK_BOX(props_callout), l1);
        gtk_box_append(GTK_BOX(props_callout), callout_style_combo);
        gtk_box_append(GTK_BOX(props_callout), callout_fill_check);
        gtk_box_append(GTK_BOX(props_callout), l2);
        gtk_box_append(GTK_BOX(props_callout), callout_fill_scale);
        gtk_box_append(GTK_BOX(props_callout), callout_shadow_check);
        gtk_box_append(GTK_BOX(props_callout), l3);
    }
    {
        GtkWidget *l1 = gtk_label_new("Text content");
        GtkWidget *l2 = gtk_label_new("Font style");
        GtkWidget *l3 = gtk_label_new("Font size");
        GtkWidget *l4 = gtk_label_new("Text stroke width");
        gtk_entry_set_placeholder_text(GTK_ENTRY(text_entry), "Type label...");
        gtk_drop_down_set_selected(GTK_DROP_DOWN(font_style_combo),
                                   gtk4_dropdown_index_for_id(GTK4_FONT_STYLE_IDS,
                                                              G_N_ELEMENTS(GTK4_FONT_STYLE_IDS),
                                                              "bold"));
        gtk_range_set_value(GTK_RANGE(font_size_scale), 24.0);
        gtk_scale_set_draw_value(GTK_SCALE(font_size_scale), TRUE);
        gtk_range_set_value(GTK_RANGE(text_stroke_scale), 2.0);
        gtk_scale_set_draw_value(GTK_SCALE(text_stroke_scale), TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_fill_check), TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_stroke_check), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_shadow_check), TRUE);
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l3), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l4), 0.0f);
        gtk_box_append(GTK_BOX(props_text), l1);
        gtk_box_append(GTK_BOX(props_text), text_entry);
        gtk_box_append(GTK_BOX(props_text), btn_apply_text);
        gtk_box_append(GTK_BOX(props_text), l2);
        gtk_box_append(GTK_BOX(props_text), font_style_combo);
        gtk_box_append(GTK_BOX(props_text), l3);
        gtk_box_append(GTK_BOX(props_text), font_size_scale);
        gtk_box_append(GTK_BOX(props_text), l4);
        gtk_box_append(GTK_BOX(props_text), text_stroke_scale);
        gtk_box_append(GTK_BOX(props_text), text_fill_check);
        gtk_box_append(GTK_BOX(props_text), text_stroke_check);
        gtk_box_append(GTK_BOX(props_text), text_shadow_check);
    }
    {
        GtkWidget *l1 = gtk_label_new("Stamp value");
        GtkWidget *l2 = gtk_label_new("Use Auto for incremental step numbers.");
        gtk_drop_down_set_selected(GTK_DROP_DOWN(stamp_combo), 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_step_check), TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(link_step_check), FALSE);
        gtk_label_set_xalign(GTK_LABEL(l1), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(l2), 0.0f);
        gtk_label_set_xalign(GTK_LABEL(step_label), 0.0f);
        gtk_box_append(GTK_BOX(props_stamp), l1);
        gtk_box_append(GTK_BOX(props_stamp), stamp_combo);
        gtk_box_append(GTK_BOX(props_stamp), auto_step_check);
        gtk_box_append(GTK_BOX(props_stamp), link_step_check);
        gtk_box_append(GTK_BOX(props_stamp), step_label);
        gtk_box_append(GTK_BOX(props_stamp), l2);
    }
    gtk_stack_add_named(GTK_STACK(props_stack), props_select, "select");
    gtk_stack_add_named(GTK_STACK(props_stack), props_arrow, "arrow");
    gtk_stack_add_named(GTK_STACK(props_stack), props_line, "line");
    gtk_stack_add_named(GTK_STACK(props_stack), props_rect, "rect");
    gtk_stack_add_named(GTK_STACK(props_stack), props_callout, "callout");
    gtk_stack_add_named(GTK_STACK(props_stack), props_text, "text");
    gtk_stack_add_named(GTK_STACK(props_stack), props_stamp, "stamp");

    gtk_widget_set_hexpand(editor_area, TRUE);
    gtk_widget_set_vexpand(editor_area, TRUE);
    gtk_widget_set_size_request(editor_area, -1, 400);
    gtk_widget_add_css_class(editor_area, "lcu-surface");
    gtk_box_append(GTK_BOX(editor_box), editor_top_row);
    gtk_box_append(GTK_BOX(editor_box), quick_styles_scroller);
    gtk_box_append(GTK_BOX(editor_box), style_row);
    gtk_box_append(GTK_BOX(editor_box), editor_path);
    gtk_box_append(GTK_BOX(editor_box), editor_area);
    gtk_box_append(GTK_BOX(editor_box), editor_status);
    gtk_widget_set_hexpand(editor_box, TRUE);
    gtk_widget_set_vexpand(editor_box, TRUE);

    gtk_widget_set_size_request(editor_dock, 280, -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor_dock), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(editor_dock), editor_dock_body);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_title);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_capture);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_edit);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_undo);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_clear);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_save_png);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_save_svg);
    gtk_box_append(GTK_BOX(editor_dock_body), props_sep);
    gtk_box_append(GTK_BOX(editor_dock_body), props_title);
    gtk_box_append(GTK_BOX(editor_dock_body), props_stack);
    gtk_box_append(GTK_BOX(editor_dock_body), dock_help);

    gtk_paned_set_start_child(GTK_PANED(studio_shell), editor_box);
    gtk_paned_set_end_child(GTK_PANED(studio_shell), editor_dock);
    state->studio_split_pos_saved = gtk4_clamp_studio_split_position(
        state,
        state->studio_split_pos_saved > 0 ? state->studio_split_pos_saved : 840);
    gtk_paned_set_position(GTK_PANED(studio_shell), state->studio_split_pos_saved);
    gtk_widget_set_hexpand(studio_shell, TRUE);
    gtk_widget_set_vexpand(studio_shell, TRUE);

    gtk_paned_set_start_child(GTK_PANED(split), studio_shell);
    gtk_paned_set_end_child(GTK_PANED(split), browser_box);
    state->split_pos_saved = gtk4_clamp_editor_split_position(
        state,
        state->split_pos_saved > 0 ? state->split_pos_saved : 430);
    gtk_paned_set_position(GTK_PANED(split), state->split_pos_saved);
    gtk_widget_set_hexpand(split, TRUE);
    gtk_widget_set_vexpand(split, TRUE);

    gtk_box_append(GTK_BOX(root), title);
    gtk_box_append(GTK_BOX(root), toolbar);
    gtk_box_append(GTK_BOX(root), split);
    gtk_box_append(GTK_BOX(root), status);

    state->status_label = status;
    state->selected_label = selected;
    state->paths_view = paths_view;
    state->search_entry = search;
    state->thumb_flow = thumb_flow;
    state->thumb_size_scale = thumb_size_scale;
    state->thumb_preset_dropdown = thumb_preset_dropdown;
    state->editor_area = editor_area;
    state->editor_status = editor_status;
    state->editor_path = editor_path;
    state->tool_select_btn = tool_select;
    state->tool_arrow_btn = tool_arrow;
    state->tool_line_btn = tool_line;
    state->tool_rect_btn = tool_rect;
    state->tool_callout_btn = tool_callout;
    state->tool_text_btn = tool_text;
    state->tool_stamp_btn = tool_stamp;
    state->editor_stroke_scale = stroke_scale;
    state->editor_props_stack = props_stack;
    state->editor_props_title = props_title;
    state->editor_styles_scroller = quick_styles_scroller;
    state->editor_browser_box = browser_box;
    state->editor_split = split;
    state->editor_studio_shell = studio_shell;
    state->editor_dock = editor_dock;
    state->editor_main_box = editor_box;
    state->editor_toggle_styles_btn = btn_toggle_styles;
    state->editor_toggle_thumbs_btn = btn_toggle_thumbs;
    state->editor_dock_toggle_btn = btn_dock_toggle;
    state->arrow_style_combo = arrow_style_combo;
    state->arrow_head_scale = arrow_head_scale;
    state->arrow_angle_scale = arrow_angle_scale;
    state->arrow_shadow_check = arrow_shadow_check;
    state->arrow_shadow_offset_scale = arrow_shadow_offset_scale;
    state->rect_round_check = rect_round_check;
    state->rect_fill_check = rect_fill_check;
    state->rect_fill_scale = rect_fill_scale;
    state->rect_shadow_check = rect_shadow_check;
    state->callout_style_combo = callout_style_combo;
    state->callout_fill_check = callout_fill_check;
    state->callout_fill_scale = callout_fill_scale;
    state->callout_shadow_check = callout_shadow_check;
    state->text_entry = text_entry;
    state->text_apply_btn = btn_apply_text;
    state->font_style_combo = font_style_combo;
    state->font_size_scale = font_size_scale;
    state->text_stroke_scale = text_stroke_scale;
    state->text_fill_check = text_fill_check;
    state->text_stroke_check = text_stroke_check;
    state->text_shadow_check = text_shadow_check;
    state->stamp_combo = stamp_combo;
    state->auto_step_check = auto_step_check;
    state->link_step_check = link_step_check;
    state->step_label = step_label;
    state->active_tool = GTK4_TOOL_ARROW;
    state->zoom_factor = 1.0;
    state->editor_stroke_width = 4.0;
    state->editor_arrow_style = GTK4_ARROW_STYLE_CLASSIC;
    state->editor_arrow_head_size = 16.0;
    state->editor_arrow_head_angle_deg = 26.0;
    state->editor_arrow_shadow = FALSE;
    state->editor_arrow_shadow_offset = 3.0;
    state->editor_rect_rounded = FALSE;
    state->editor_rect_fill = FALSE;
    state->editor_rect_fill_alpha = 0.24;
    state->editor_rect_shadow = FALSE;
    state->editor_callout_style = GTK4_CALLOUT_STYLE_ROUNDED;
    state->editor_callout_fill = FALSE;
    state->editor_callout_fill_alpha = 0.24;
    state->editor_callout_shadow = FALSE;
    state->editor_text_style = GTK4_TEXT_STYLE_BOLD;
    state->editor_text_size = 24.0;
    state->editor_text_stroke_width = 2.0;
    state->editor_text_fill_enabled = TRUE;
    state->editor_text_stroke_enabled = FALSE;
    state->editor_text_shadow_enabled = TRUE;
    state->editor_text_fill_opacity = 1.0;
    state->editor_stamp_radius = 17.0;
    state->editor_auto_step = TRUE;
    state->editor_link_steps = FALSE;
    state->editor_next_step = 1;
    gtk4_set_thumb_preview_width(state, state->thumb_preview_width > 0 ? state->thumb_preview_width : 270);
    gdk_rgba_parse(&state->editor_color, "#ef5b5b");
    gtk4_apply_dock_layout(state);
    gtk4_apply_editor_panel_visibility(state);

    g_signal_connect(btn_capture, "clicked", G_CALLBACK(gtk4_on_capture_clicked), state);
    g_signal_connect(btn_edit, "clicked", G_CALLBACK(gtk4_on_edit_selected_clicked), state);
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(gtk4_on_refresh_clicked), state);
    g_signal_connect(btn_open_folder, "clicked", G_CALLBACK(gtk4_on_open_folder_clicked), state);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(gtk4_on_delete_clicked), state);
    g_signal_connect(btn_dock_toggle, "clicked", G_CALLBACK(gtk4_on_dock_toggle_clicked), state);
    g_signal_connect(btn_toggle_styles, "clicked", G_CALLBACK(gtk4_on_toggle_styles_clicked), state);
    g_signal_connect(btn_toggle_thumbs, "clicked", G_CALLBACK(gtk4_on_toggle_thumbs_clicked), state);
    g_signal_connect(btn_thumbs_bigger, "clicked", G_CALLBACK(gtk4_on_thumb_region_bigger), state);
    g_signal_connect(btn_thumbs_smaller, "clicked", G_CALLBACK(gtk4_on_thumb_region_smaller), state);
    g_signal_connect(btn_zoom_reset, "clicked", G_CALLBACK(gtk4_on_zoom_reset_clicked), state);
    g_signal_connect(dock_capture, "clicked", G_CALLBACK(gtk4_on_capture_clicked), state);
    g_signal_connect(dock_edit, "clicked", G_CALLBACK(gtk4_on_edit_selected_clicked), state);
    g_signal_connect(dock_undo, "clicked", G_CALLBACK(gtk4_on_editor_undo_clicked), state);
    g_signal_connect(dock_clear, "clicked", G_CALLBACK(gtk4_on_editor_clear_clicked), state);
    g_signal_connect(dock_save_png, "clicked", G_CALLBACK(gtk4_on_editor_save_png_clicked), state);
    g_signal_connect(dock_save_svg, "clicked", G_CALLBACK(gtk4_on_editor_save_svg_clicked), state);
    g_signal_connect(stroke_scale, "value-changed", G_CALLBACK(gtk4_on_editor_stroke_changed), state);
    g_signal_connect(stroke_scale, "value-changed", G_CALLBACK(gtk4_on_line_props_changed), state);
    g_signal_connect(stroke_scale, "value-changed", G_CALLBACK(gtk4_on_arrow_props_changed), state);
    g_signal_connect(stroke_scale, "value-changed", G_CALLBACK(gtk4_on_rect_props_changed), state);
    g_signal_connect(stroke_scale, "value-changed", G_CALLBACK(gtk4_on_callout_props_changed), state);
    g_signal_connect(search, "changed", G_CALLBACK(gtk4_on_search_changed), state);
    g_signal_connect(thumb_size_scale, "value-changed", G_CALLBACK(gtk4_on_thumb_size_changed), state);
    g_signal_connect(thumb_preset_dropdown, "notify::selected", G_CALLBACK(gtk4_on_thumb_preset_selected), state);
    g_signal_connect(split, "notify::position", G_CALLBACK(gtk4_on_split_position_notify), state);
    g_signal_connect(studio_shell, "notify::position", G_CALLBACK(gtk4_on_split_position_notify), state);
    g_signal_connect(thumb_flow, "selected-children-changed", G_CALLBACK(gtk4_on_thumb_selection_changed), state);
    g_signal_connect(thumb_flow, "child-activated", G_CALLBACK(gtk4_on_thumb_activated), state);
    g_signal_connect(tool_select, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_arrow, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_line, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_rect, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_callout, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_text, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_stamp, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(arrow_style_combo, "notify::selected", G_CALLBACK(gtk4_on_arrow_dropdown_selected), state);
    g_signal_connect(arrow_head_scale, "value-changed", G_CALLBACK(gtk4_on_arrow_props_changed), state);
    g_signal_connect(arrow_angle_scale, "value-changed", G_CALLBACK(gtk4_on_arrow_props_changed), state);
    g_signal_connect(arrow_shadow_check, "toggled", G_CALLBACK(gtk4_on_arrow_props_changed), state);
    g_signal_connect(arrow_shadow_offset_scale, "value-changed", G_CALLBACK(gtk4_on_arrow_props_changed), state);
    g_signal_connect(rect_round_check, "toggled", G_CALLBACK(gtk4_on_rect_props_changed), state);
    g_signal_connect(rect_fill_check, "toggled", G_CALLBACK(gtk4_on_rect_props_changed), state);
    g_signal_connect(rect_fill_scale, "value-changed", G_CALLBACK(gtk4_on_rect_props_changed), state);
    g_signal_connect(rect_shadow_check, "toggled", G_CALLBACK(gtk4_on_rect_props_changed), state);
    g_signal_connect(callout_style_combo, "notify::selected", G_CALLBACK(gtk4_on_callout_dropdown_selected), state);
    g_signal_connect(callout_fill_check, "toggled", G_CALLBACK(gtk4_on_callout_props_changed), state);
    g_signal_connect(callout_fill_scale, "value-changed", G_CALLBACK(gtk4_on_callout_props_changed), state);
    g_signal_connect(callout_shadow_check, "toggled", G_CALLBACK(gtk4_on_callout_props_changed), state);
    g_signal_connect(font_style_combo, "notify::selected", G_CALLBACK(gtk4_on_font_dropdown_selected), state);
    g_signal_connect(font_size_scale, "value-changed", G_CALLBACK(gtk4_on_text_props_changed), state);
    g_signal_connect(text_stroke_scale, "value-changed", G_CALLBACK(gtk4_on_text_props_changed), state);
    g_signal_connect(text_fill_check, "toggled", G_CALLBACK(gtk4_on_text_props_changed), state);
    g_signal_connect(text_stroke_check, "toggled", G_CALLBACK(gtk4_on_text_props_changed), state);
    g_signal_connect(text_shadow_check, "toggled", G_CALLBACK(gtk4_on_text_props_changed), state);
    g_signal_connect(stamp_combo, "notify::selected", G_CALLBACK(gtk4_on_stamp_dropdown_selected), state);
    g_signal_connect(auto_step_check, "toggled", G_CALLBACK(gtk4_on_stamp_props_changed), state);
    g_signal_connect(link_step_check, "toggled", G_CALLBACK(gtk4_on_stamp_props_changed), state);
    g_signal_connect(btn_apply_text, "clicked", G_CALLBACK(gtk4_on_apply_text_clicked), state);
    g_signal_connect(text_entry, "activate", G_CALLBACK(gtk4_on_apply_text_clicked), state);
    g_signal_connect(qs_arrow_classic, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_arrow_bold, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_arrow_pointer, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_arrow_dashed, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_arrow_dotted, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_arrow_double, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_callout_red, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_callout_green, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_callout_right, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_callout_highlight, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_callout_shadow, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_rect_rounded, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_rect_shadow, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_step_badge, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_step_badge_blue, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_step_badge_linked, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_text_bold, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    g_signal_connect(qs_text_outline, "clicked", G_CALLBACK(gtk4_on_quick_style_clicked), state);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(editor_area), gtk4_editor_draw, state, NULL);

    click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), 0);
    g_signal_connect(click, "pressed", G_CALLBACK(gtk4_on_canvas_pressed), state);
    g_signal_connect(click, "released", G_CALLBACK(gtk4_on_canvas_released), state);
    gtk_widget_add_controller(editor_area, GTK_EVENT_CONTROLLER(click));

    motion = gtk_event_controller_motion_new();
    g_signal_connect(motion, "motion", G_CALLBACK(gtk4_on_canvas_motion), state);
    gtk_widget_add_controller(editor_area, motion);

    scroll = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL | GTK_EVENT_CONTROLLER_SCROLL_DISCRETE);
    g_signal_connect(scroll, "scroll", G_CALLBACK(gtk4_on_canvas_scroll), state);
    gtk_widget_add_controller(editor_area, scroll);

    key = gtk_event_controller_key_new();
    g_signal_connect(key, "key-pressed", G_CALLBACK(gtk4_on_window_key), state);
    gtk_widget_add_controller(state->window, key);

    gtk4_update_properties_panel(state);
    gtk4_sync_editor_property_widgets(state);

    return root;
}

static void gtk4_on_notebook_switch_page(GtkNotebook *notebook,
                                         GtkWidget *page,
                                         guint page_num,
                                         gpointer user_data) {
    Gtk4State *state = user_data;
    const gchar *tab_text = gtk_notebook_get_tab_label_text(notebook, page);
    gchar *msg = g_strdup_printf("Active tab: %s (%u)", tab_text ? tab_text : "unknown", page_num);
    gtk4_set_global_status(state, msg);
    g_free(msg);
}

static GtkWidget *gtk4_build_ui(Gtk4State *state) {
    static const char *themes[] = { "Dark", "Light", NULL };
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *title = gtk_label_new("Linux Utilities Control Center");
    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *theme_label = gtk_label_new("Theme");
    GtkWidget *theme_dd = gtk_drop_down_new_from_strings(themes);
    GtkWidget *notebook = gtk_notebook_new();
    GtkWidget *global_status = gtk_label_new("Ready.");
    GtkWidget *tab_night = gtk4_build_night_tab(state);
    GtkWidget *tab_audio = gtk4_build_audio_tab(state);
    GtkWidget *tab_utils = gtk4_build_utilities_tab(state);
    GtkWidget *tab_shortcuts = gtk4_build_shortcuts_tab(state);
    GtkWidget *tab_screens = gtk4_build_screenshots_tab(state);

    gtk_widget_add_css_class(root, "lcu-root");
    gtk_widget_add_css_class(header, "lcu-header");
    gtk_widget_add_css_class(title, "lcu-title");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(theme_label), 1.0f);
    gtk_label_set_xalign(GTK_LABEL(global_status), 0.0f);
    gtk_widget_set_margin_start(global_status, 12);
    gtk_widget_set_margin_end(global_status, 12);
    gtk_widget_set_margin_top(global_status, 6);
    gtk_widget_set_margin_bottom(global_status, 8);
    gtk_widget_add_css_class(global_status, "dim-label");
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_widget_set_halign(theme_dd, GTK_ALIGN_END);

    gtk_box_append(GTK_BOX(header), title);
    gtk_box_append(GTK_BOX(header), spacer);
    gtk_box_append(GTK_BOX(header), theme_label);
    gtk_box_append(GTK_BOX(header), theme_dd);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_night, gtk_label_new("Night Light"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_audio, gtk_label_new("Audio"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_utils, gtk_label_new("Utilities"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_shortcuts, gtk_label_new("Shortcuts"));
    state->screenshots_page = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_screens, gtk_label_new("Screenshots"));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), state->screenshots_page);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);
    state->notebook = notebook;
    state->global_status = global_status;

    gtk_drop_down_set_selected(GTK_DROP_DOWN(theme_dd), 0);
    g_signal_connect(theme_dd, "notify::selected", G_CALLBACK(gtk4_on_theme_selected), state);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(gtk4_on_notebook_switch_page), state);

    gtk_box_append(GTK_BOX(root), header);
    gtk_box_append(GTK_BOX(root), notebook);
    gtk_box_append(GTK_BOX(root), global_status);
    return root;
}

static gboolean gtk4_post_activate_idle(gpointer data) {
    Gtk4State *state = data;
    if (!state) {
        return G_SOURCE_REMOVE;
    }
    gtk4_reload(state);
    gtk4_night_refresh(state);
    gtk4_audio_refresh(state);
    return G_SOURCE_REMOVE;
}

static gboolean gtk4_on_window_close_request(GtkWindow *window, gpointer user_data) {
    Gtk4State *state = user_data;
    (void)window;
    if (!state) {
        return FALSE;
    }
    if (state->ui_state_save_source_id != 0) {
        g_source_remove(state->ui_state_save_source_id);
        state->ui_state_save_source_id = 0;
    }
    gtk4_ui_state_save(state);
    return FALSE;
}

static void gtk4_state_free(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->ui_state_save_source_id != 0) {
        g_source_remove(state->ui_state_save_source_id);
        state->ui_state_save_source_id = 0;
    }
    if (state->editor_pixbuf) {
        g_object_unref(state->editor_pixbuf);
    }
    if (state->marks) {
        g_ptr_array_free(state->marks, TRUE);
    }
    if (state->css_provider) {
        g_object_unref(state->css_provider);
    }
    g_free(state->launch_dir);
    g_free(state->shots_dir);
    g_free(state->editor_image_path);
    g_free(state->ui_state_file);
    g_free(state);
}

static void gtk4_on_activate(GtkApplication *app, gpointer user_data) {
    Gtk4State *state = user_data;
    GtkWidget *root = NULL;

    state->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(state->window), "Linux Utilities Control Center (GTK4 Tabs Fix)");
    gtk_window_set_default_size(GTK_WINDOW(state->window), 1260, 820);
    state->marks = g_ptr_array_new_with_free_func(gtk4_free_mark);
    gtk4_apply_theme(TRUE);
    gtk4_apply_css(state);

    root = gtk4_build_ui(state);
    gtk_window_set_child(GTK_WINDOW(state->window), root);
    g_signal_connect(state->window, "close-request", G_CALLBACK(gtk4_on_window_close_request), state);
    gtk_window_present(GTK_WINDOW(state->window));
    g_idle_add(gtk4_post_activate_idle, state);
}

int main(int argc, char **argv) {
    Gtk4State *state = g_new0(Gtk4State, 1);
    gint status = 0;
    gchar *target_dir = NULL;
    gchar *target_basename = NULL;
    gchar *candidate_shots = NULL;
    gchar *argv0_only[] = { NULL, NULL };

    if (argc > 1 && argv[1] && g_file_test(argv[1], G_FILE_TEST_IS_DIR)) {
        target_dir = g_canonicalize_filename(argv[1], NULL);
    } else {
        target_dir = g_get_current_dir();
    }
    state->launch_dir = g_strdup(target_dir);
    target_basename = g_path_get_basename(target_dir);
    candidate_shots = g_build_filename(target_dir, "Screenshots", NULL);
    if (g_file_test(candidate_shots, G_FILE_TEST_IS_DIR)) {
        state->shots_dir = g_strdup(candidate_shots);
    } else if ((target_basename && g_ascii_strcasecmp(target_basename, "Screenshots") == 0)) {
        state->shots_dir = g_strdup(target_dir);
    } else {
        state->shots_dir = g_strdup(candidate_shots);
    }
    state->thumb_preview_width = 270;
    state->thumb_preview_height = CLAMP((gint)lrint((gdouble)state->thumb_preview_width * 0.62), 110, 300);
    state->split_pos_saved = 430;
    state->studio_split_pos_saved = 840;
    state->studio_dock_width_saved = 280;
    state->editor_styles_visible = TRUE;
    state->editor_thumbs_visible = TRUE;
    state->editor_dock_right = TRUE;
    state->ui_state_file = gtk4_default_ui_state_path();
    gtk4_ui_state_load(state);

    state->app = gtk_application_new("com.antshiv.linuxutils.controlcenter.gtk4", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(state->app, "activate", G_CALLBACK(gtk4_on_activate), state);
    argv0_only[0] = argv[0];
    status = g_application_run(G_APPLICATION(state->app), 1, argv0_only);

    g_object_unref(state->app);
    g_free(target_dir);
    g_free(target_basename);
    g_free(candidate_shots);
    gtk4_state_free(state);
    return status;
}
