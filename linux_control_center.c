#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <cairo-svg.h>
#include <math.h>
#include <stdarg.h>

enum {
    COL_PIXBUF = 0,
    COL_NAME,
    COL_PATH,
    N_COLS
};

typedef struct {
    gchar *name;
    gchar *path;
    gint64 mtime;
} ImageEntry;

typedef enum {
    SHOT_MARK_ARROW = 0,
    SHOT_MARK_STAMP,
    SHOT_MARK_TEXT,
    SHOT_MARK_RECT
} ShotMarkType;

enum {
    SHOT_HANDLE_NONE = 0,
    SHOT_HANDLE_ARROW_START = 1,
    SHOT_HANDLE_ARROW_END = 2,
    SHOT_HANDLE_MOVE = 3,
    SHOT_HANDLE_RECT_START = 4,
    SHOT_HANDLE_RECT_END = 5
};

typedef struct {
    ShotMarkType type;
    gdouble x1;
    gdouble y1;
    gdouble x2;
    gdouble y2;
    gdouble stroke_width;
    gdouble arrow_head_len;
    gdouble arrow_head_spread;
    gchar *text;
    GdkRGBA color;
} ShotMark;

typedef struct {
    GtkApplication *app;
    GtkWidget *window;
    GtkCssProvider *css_provider;
    gboolean dark_theme;

    GtkWidget *night_status;
    GtkWidget *night_meta_label;
    GtkWidget *night_temp_label;
    GtkWidget *night_scale;
    GtkWidget *night_auto_switch;
    gboolean night_switch_syncing;

    GtkWidget *audio_status;
    GtkWidget *audio_volume_label;
    GtkWidget *audio_scale;

    GtkListStore *shots_store;
    GtkWidget *shots_icon_view;
    GtkWidget *shots_status;
    GtkWidget *shots_selected;
    GtkWidget *shots_paths_view;
    GtkWidget *shots_folder_label;
    GtkWidget *shots_editor_canvas;
    GtkWidget *shots_editor_status;
    GtkWidget *shots_editor_tool_combo;
    GtkWidget *shots_editor_stamp_combo;
    GtkWidget *shots_editor_text_entry;
    GtkWidget *shots_editor_color_btn;
    GtkWidget *shots_editor_arrow_width_scale;
    GtkWidget *shots_editor_arrow_head_len_scale;
    GtkWidget *shots_editor_arrow_head_angle_scale;
    GtkWidget *shots_editor_auto_step_check;
    GtkWidget *shots_editor_step_label;
    GtkWidget *shots_browser_info_revealer;
    GtkWidget *shots_browser_toggle_btn;
    GtkWidget *shots_browser_split_paned;
    GtkWidget *shots_editor_main_paned;
    GtkWidget *shots_editor_canvas_panel;
    GtkWidget *shots_editor_sidebar;
    GtkWidget *shots_editor_sidebar_scroller;
    GtkWidget *shots_editor_dock_toggle_btn;
    GtkWidget *shots_editor_path_label;
    GdkPixbuf *shots_editor_pixbuf;
    gchar *shots_editor_image_path;
    GPtrArray *shots_editor_marks;
    gint shots_editor_selected_mark_index;
    gint shots_editor_selected_arrow_handle;
    gboolean shots_editor_edit_dragging;
    gdouble shots_editor_edit_anchor_x;
    gdouble shots_editor_edit_anchor_y;
    gdouble shots_editor_edit_origin_x1;
    gdouble shots_editor_edit_origin_y1;
    gdouble shots_editor_edit_origin_x2;
    gdouble shots_editor_edit_origin_y2;
    gboolean shots_editor_dragging;
    ShotMarkType shots_editor_drag_mark_type;
    gdouble shots_editor_drag_start_x;
    gdouble shots_editor_drag_start_y;
    gdouble shots_editor_drag_cur_x;
    gdouble shots_editor_drag_cur_y;
    gdouble shots_editor_draw_scale;
    gdouble shots_editor_draw_offset_x;
    gdouble shots_editor_draw_offset_y;
    gint shots_editor_next_step;
    gboolean shots_editor_style_syncing;
    gboolean shots_editor_tools_docked_right;

    GtkWidget *utils_status;

    gchar *repo_root;
    gchar *launch_dir;
    gchar *redshift_script;
    gchar *import_script;
    gchar *shots_dir;
} AppState;

static void shots_reload(AppState *state);
static void shots_editor_show_color_palette_menu(AppState *state, GdkEventButton *event);

static gboolean ends_with_image_ext(const gchar *name) {
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

static gboolean dir_contains_images(const gchar *dir_path) {
    GDir *dir = NULL;
    const gchar *name = NULL;

    if (!dir_path || !g_file_test(dir_path, G_FILE_TEST_IS_DIR)) {
        return FALSE;
    }

    dir = g_dir_open(dir_path, 0, NULL);
    if (!dir) {
        return FALSE;
    }

    while ((name = g_dir_read_name(dir)) != NULL) {
        if (ends_with_image_ext(name)) {
            g_dir_close(dir);
            return TRUE;
        }
    }

    g_dir_close(dir);
    return FALSE;
}

static void free_image_entry(gpointer data) {
    ImageEntry *entry = data;
    if (!entry) {
        return;
    }
    g_free(entry->name);
    g_free(entry->path);
    g_free(entry);
}

static gint compare_entries_desc(gconstpointer a, gconstpointer b) {
    const ImageEntry *ea = *(ImageEntry * const *)a;
    const ImageEntry *eb = *(ImageEntry * const *)b;
    if (ea->mtime == eb->mtime) {
        return g_strcmp0(ea->name, eb->name);
    }
    return (ea->mtime < eb->mtime) ? 1 : -1;
}

static void apply_css_theme(AppState *state, gboolean dark_mode) {
    const gchar *dark_css =
        "* {"
        "  text-shadow: none;"
        "  -gtk-icon-shadow: none;"
        "}"
        ".app-window, .app-root, .panel-root, .app-notebook, .app-notebook > stack, .app-notebook > stack > * {"
        "  background-color: #121720;"
        "  color: #d7dae0;"
        "  font-family: 'Ubuntu', 'Noto Sans', sans-serif;"
        "  font-size: 11px;"
        "}"
        ".app-window {"
        "  border: 0;"
        "}"
        ".app-window decoration {"
        "  background: #121720;"
        "  border: 1px solid #2f3847;"
        "  box-shadow: none;"
        "}"
        ".app-header {"
        "  background: #1c2330;"
        "  color: #e6e9ef;"
        "  border-bottom: 1px solid #2f3847;"
        "}"
        ".app-header label {"
        "  color: #e6e9ef;"
        "}"
        ".theme-strip {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0;"
        "  margin: 0;"
        "}"
        ".theme-strip-label {"
        "  color: #c7d2e3;"
        "  font-size: 11px;"
        "  margin-right: 6px;"
        "}"
        ".theme-strip combobox, .theme-strip .theme-strip-combo {"
        "  background: transparent;"
        "  border: 0;"
        "  box-shadow: none;"
        "  padding: 0;"
        "  background-image: none;"
        "}"
        ".theme-strip combobox box {"
        "  background: transparent;"
        "  border: 0;"
        "  box-shadow: none;"
        "  padding: 0;"
        "}"
        ".theme-strip combobox button {"
        "  background: #1f2734;"
        "  color: #dfe6f2;"
        "  border: 1px solid #3b4a61;"
        "  border-radius: 7px;"
        "  min-height: 30px;"
        "  padding: 2px 8px;"
        "  box-shadow: none;"
        "  background-image: none;"
        "}"
        ".theme-strip combobox button:hover {"
        "  background: #273246;"
        "}"
        ".app-root {"
        "  padding: 0;"
        "}"
        ".panel-root {"
        "  padding: 16px;"
        "  background: transparent;"
        "}"
        ".app-notebook {"
        "  border: 0;"
        "  padding: 0;"
        "}"
        ".app-notebook > header {"
        "  background: #1c2330;"
        "  border-bottom: 1px solid #2f3847;"
        "  padding: 6px 10px 0 10px;"
        "}"
        ".app-notebook > header > tabs > tab {"
        "  background: #232b39;"
        "  color: #c8ccd6;"
        "  border: 1px solid #374153;"
        "  border-bottom: none;"
        "  border-radius: 9px 9px 0 0;"
        "  padding: 7px 14px;"
        "  margin-right: 6px;"
        "}"
        ".app-notebook > header > tabs > tab:checked {"
        "  background: #121720;"
        "  color: #ffffff;"
        "  border-color: #4e5f79;"
        "}"
        ".app-notebook > stack {"
        "  background-color: #121720;"
        "  border-top: 1px solid #2f3847;"
        "  margin: 0;"
        "  padding: 0;"
        "}"
        ".app-root label, .panel-root label {"
        "  color: #d7dae0;"
        "}"
        "button {"
        "  background: #2b3443;"
        "  color: #e2e6ed;"
        "  border: 1px solid #46546c;"
        "  border-radius: 8px;"
        "  padding: 7px 12px;"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "  text-shadow: none;"
        "  outline-style: none;"
        "  outline-width: 0;"
        "}"
        "button:hover {"
        "  background: #354156;"
        "  background-image: none;"
        "  box-shadow: none;"
        "}"
        "button:active {"
        "  background: #3e4c63;"
        "  background-image: none;"
        "  box-shadow: none;"
        "}"
        ".app-header button, .app-header button:hover, .app-header button:active {"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "}"
        "combobox box, combobox button {"
        "  background: #242c3a;"
        "  color: #e2e6ed;"
        "  border: 1px solid #46546c;"
        "  border-radius: 7px;"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "}"
        "switch {"
        "  background: #2f3949;"
        "  border: 1px solid #47546a;"
        "  border-radius: 11px;"
        "}"
        "switch:checked {"
        "  background: #2377c9;"
        "  border-color: #3888d5;"
        "}"
        "switch slider {"
        "  background: #f3f6fb;"
        "  border: 1px solid #4e5f79;"
        "}"
        "entry, textview, textview text, iconview {"
        "  background: #0f141d;"
        "  color: #d7dae0;"
        "}"
        "scrolledwindow.surface > viewport {"
        "  background: #0f141d;"
        "}"
        ".surface {"
        "  border: 1px solid #2d3748;"
        "  border-radius: 10px;"
        "}"
        ".split-pane > separator {"
        "  min-height: 10px;"
        "  min-width: 10px;"
        "  background: #273246;"
        "  border: 1px solid #3c4c64;"
        "  border-radius: 6px;"
        "}"
        ".split-pane > separator:hover {"
        "  background: #2e3f58;"
        "  border-color: #4f6690;"
        "}"
        "scale trough {"
        "  background: #3b4556;"
        "  min-height: 6px;"
        "  border-radius: 3px;"
        "}"
        "scale highlight {"
        "  background: #0e639c;"
        "  border-radius: 3px;"
        "}"
        "scale slider {"
        "  background: #f3f3f3;"
        "  border: 1px solid #0e639c;"
        "}"
        ".hero-title {"
        "  font-size: 17px;"
        "  font-weight: 700;"
        "  color: #ffffff;"
        "}"
        ".hero-sub {"
        "  font-size: 11px;"
        "  color: #aab3c2;"
        "}"
        ".status-pill {"
        "  background: #1f3f5f;"
        "  color: #d7ebff;"
        "  border: 1px solid #35689a;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "}"
        ".card {"
        "  background: #1a202b;"
        "  border: 1px solid #303a4a;"
        "  border-radius: 10px;"
        "  box-shadow: none;"
        "}"
        ".toggle-row {"
        "  background: #111823;"
        "  border: 1px solid #2b3a53;"
        "  border-radius: 10px;"
        "  padding: 8px 10px;"
        "}"
        ".toggle-row label {"
        "  color: #bcd0ea;"
        "  font-weight: 600;"
        "}"
        ".action-row button {"
        "  margin: 0 6px 6px 0;"
        "  min-height: 24px;"
        "  font-weight: 600;"
        "}"
        ".action-btn {"
        "  padding: 2px 4px;"
        "  border-radius: 2px;"
        "  min-height: 24px;"
        "  font-weight: 600;"
        "}"
        ".action-btn:hover {"
        "  box-shadow: 0 0 0 1px rgba(110, 170, 235, 0.42), 0 0 8px rgba(64, 130, 210, 0.28);"
        "}"
        ".action-btn:focus {"
        "  box-shadow: 0 0 0 1px rgba(135, 190, 255, 0.55), 0 0 10px rgba(80, 145, 220, 0.33);"
        "}"
        ".accent-btn {"
        "  background: #0e639c;"
        "  color: #ffffff;"
        "  border-color: #2f78b7;"
        "  background-image: none;"
        "  box-shadow: 0 0 0 1px rgba(55, 135, 210, 0.45), 0 0 10px rgba(35, 110, 180, 0.32);"
        "}"
        ".accent-btn:hover {"
        "  background: #1177bb;"
        "  background-image: none;"
        "  box-shadow: 0 0 0 1px rgba(90, 165, 235, 0.55), 0 0 12px rgba(40, 125, 205, 0.38);"
        "}"
        ".meta-info {"
        "  color: #a7b5c8;"
        "}"
        ".utility-btn {"
        "  min-height: 64px;"
        "  border-radius: 10px;"
        "  text-shadow: none;"
        "}"
        ".utility-btn label {"
        "  font-weight: 600;"
        "}";
    const gchar *light_css =
        "* {"
        "  text-shadow: none;"
        "  -gtk-icon-shadow: none;"
        "}"
        ".app-window, .app-root, .panel-root, .app-notebook, .app-notebook > stack, .app-notebook > stack > * {"
        "  background-color: #f3f7fd;"
        "  color: #1d2b3a;"
        "  font-family: 'Ubuntu', 'Noto Sans', sans-serif;"
        "  font-size: 11px;"
        "}"
        ".app-window {"
        "  border: 0;"
        "}"
        ".app-window decoration {"
        "  background: #f3f7fd;"
        "  border: 1px solid #c5d4e9;"
        "  box-shadow: none;"
        "}"
        ".app-header {"
        "  background: #e7eef9;"
        "  color: #1d2b3a;"
        "  border-bottom: 1px solid #c5d4e9;"
        "}"
        ".app-header label {"
        "  color: #1d2b3a;"
        "}"
        ".theme-strip {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0;"
        "  margin: 0;"
        "}"
        ".theme-strip-label {"
        "  color: #2f4865;"
        "  font-size: 11px;"
        "  margin-right: 6px;"
        "}"
        ".theme-strip combobox, .theme-strip .theme-strip-combo {"
        "  background: transparent;"
        "  border: 0;"
        "  box-shadow: none;"
        "  padding: 0;"
        "  background-image: none;"
        "}"
        ".theme-strip combobox box {"
        "  background: transparent;"
        "  border: 0;"
        "  box-shadow: none;"
        "  padding: 0;"
        "}"
        ".theme-strip combobox button {"
        "  background: #ffffff;"
        "  color: #1d2b3a;"
        "  border: 1px solid #b7cbe4;"
        "  border-radius: 7px;"
        "  min-height: 30px;"
        "  padding: 2px 8px;"
        "  box-shadow: none;"
        "  background-image: none;"
        "}"
        ".theme-strip combobox button:hover {"
        "  background: #eef4fd;"
        "}"
        ".app-root {"
        "  padding: 0;"
        "}"
        ".panel-root {"
        "  padding: 16px;"
        "  background: transparent;"
        "}"
        ".app-notebook {"
        "  border: 0;"
        "  padding: 0;"
        "}"
        ".app-notebook > header {"
        "  background: #e7eef9;"
        "  border-bottom: 1px solid #c5d4e9;"
        "  padding: 6px 10px 0 10px;"
        "}"
        ".app-notebook > header > tabs > tab {"
        "  background: #d9e4f3;"
        "  color: #32465d;"
        "  border: 1px solid #bdd0e6;"
        "  border-bottom: none;"
        "  border-radius: 9px 9px 0 0;"
        "  padding: 7px 14px;"
        "  margin-right: 6px;"
        "}"
        ".app-notebook > header > tabs > tab:checked {"
        "  background: #f3f7fd;"
        "  color: #1d2b3a;"
        "  border-color: #a8bfdc;"
        "}"
        ".app-notebook > stack {"
        "  background-color: #f3f7fd;"
        "  border-top: 1px solid #c5d4e9;"
        "  margin: 0;"
        "  padding: 0;"
        "}"
        ".app-root label, .panel-root label {"
        "  color: #1d2b3a;"
        "}"
        "button {"
        "  background: #ffffff;"
        "  color: #1d2b3a;"
        "  border: 1px solid #c0d0e6;"
        "  border-radius: 8px;"
        "  padding: 7px 12px;"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "  text-shadow: none;"
        "  outline-style: none;"
        "  outline-width: 0;"
        "}"
        "button:hover {"
        "  background: #edf4fd;"
        "  background-image: none;"
        "  box-shadow: none;"
        "}"
        "button:active {"
        "  background: #e2edf9;"
        "  background-image: none;"
        "  box-shadow: none;"
        "}"
        ".app-header button, .app-header button:hover, .app-header button:active {"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "}"
        "combobox box, combobox button {"
        "  background: #ffffff;"
        "  color: #1d2b3a;"
        "  border: 1px solid #c0d0e6;"
        "  border-radius: 7px;"
        "  background-image: none;"
        "  border-image: none;"
        "  box-shadow: none;"
        "}"
        "switch {"
        "  background: #dce7f6;"
        "  border: 1px solid #b7cbe3;"
        "  border-radius: 11px;"
        "}"
        "switch:checked {"
        "  background: #4f9add;"
        "  border-color: #3f89cd;"
        "}"
        "switch slider {"
        "  background: #ffffff;"
        "  border: 1px solid #a7bfdc;"
        "}"
        "entry, textview, textview text, iconview {"
        "  background: #ffffff;"
        "  color: #1d2b3a;"
        "}"
        "scrolledwindow.surface > viewport {"
        "  background: #ffffff;"
        "}"
        ".surface {"
        "  border: 1px solid #c5d5ea;"
        "  border-radius: 10px;"
        "}"
        ".split-pane > separator {"
        "  min-height: 10px;"
        "  min-width: 10px;"
        "  background: #d6e4f5;"
        "  border: 1px solid #b4cae3;"
        "  border-radius: 6px;"
        "}"
        ".split-pane > separator:hover {"
        "  background: #c8dbf1;"
        "  border-color: #9ebcdc;"
        "}"
        "scale trough {"
        "  background: #ccd8ea;"
        "  min-height: 6px;"
        "  border-radius: 3px;"
        "}"
        "scale highlight {"
        "  background: #3b82c4;"
        "  border-radius: 3px;"
        "}"
        "scale slider {"
        "  background: #ffffff;"
        "  border: 1px solid #3b82c4;"
        "}"
        ".hero-title {"
        "  font-size: 17px;"
        "  font-weight: 700;"
        "  color: #1d2b3a;"
        "}"
        ".hero-sub {"
        "  font-size: 11px;"
        "  color: #3f5a75;"
        "}"
        ".status-pill {"
        "  background: #d9eafb;"
        "  color: #1d2b3a;"
        "  border: 1px solid #afcbe8;"
        "  border-radius: 8px;"
        "  padding: 8px 12px;"
        "}"
        ".card {"
        "  background: #ffffff;"
        "  border: 1px solid #c6d7ec;"
        "  border-radius: 10px;"
        "  box-shadow: none;"
        "}"
        ".toggle-row {"
        "  background: #edf4fd;"
        "  border: 1px solid #c5d8ee;"
        "  border-radius: 10px;"
        "  padding: 8px 10px;"
        "}"
        ".toggle-row label {"
        "  color: #2a4765;"
        "  font-weight: 600;"
        "}"
        ".action-row button {"
        "  margin: 0 6px 6px 0;"
        "  min-height: 24px;"
        "  font-weight: 600;"
        "}"
        ".action-btn {"
        "  padding: 2px 4px;"
        "  border-radius: 2px;"
        "  min-height: 24px;"
        "  font-weight: 600;"
        "}"
        ".action-btn:hover {"
        "  box-shadow: 0 0 0 1px rgba(90, 145, 220, 0.42), 0 0 8px rgba(75, 130, 205, 0.24);"
        "}"
        ".action-btn:focus {"
        "  box-shadow: 0 0 0 1px rgba(105, 165, 235, 0.52), 0 0 10px rgba(80, 140, 210, 0.30);"
        "}"
        ".accent-btn {"
        "  background: #3e88cc;"
        "  color: #ffffff;"
        "  border-color: #347aba;"
        "  background-image: none;"
        "  box-shadow: 0 0 0 1px rgba(82, 145, 220, 0.45), 0 0 9px rgba(72, 135, 205, 0.26);"
        "}"
        ".accent-btn:hover {"
        "  background: #4c95d8;"
        "  background-image: none;"
        "  box-shadow: 0 0 0 1px rgba(94, 158, 232, 0.52), 0 0 11px rgba(84, 146, 214, 0.30);"
        "}"
        ".meta-info {"
        "  color: #3d5875;"
        "}"
        ".utility-btn {"
        "  min-height: 64px;"
        "  border-radius: 10px;"
        "}"
        ".utility-btn label {"
        "  font-weight: 600;"
        "}";

    const gchar *css = dark_mode ? dark_css : light_css;
    GError *error = NULL;

    if (!state->css_provider) {
        state->css_provider = gtk_css_provider_new();
        gtk_style_context_add_provider_for_screen(
            gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(state->css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }

    gtk_css_provider_load_from_data(state->css_provider, css, -1, &error);
    if (error) {
        g_warning("Failed to load CSS: %s", error->message);
        g_error_free(error);
    }

    if (gtk_settings_get_default()) {
        g_object_set(gtk_settings_get_default(),
                     "gtk-application-prefer-dark-theme",
                     dark_mode,
                     NULL);
    }

    state->dark_theme = dark_mode;
}

static void on_theme_combo_changed(GtkComboBoxText *combo, gpointer user_data) {
    AppState *state = user_data;
    const gchar *id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    gboolean dark_mode = TRUE;

    if (id && g_strcmp0(id, "light") == 0) {
        dark_mode = FALSE;
    }
    apply_css_theme(state, dark_mode);
}

static void set_label_markupf(GtkWidget *label, const gchar *fmt, ...) {
    va_list args;
    gchar *text = NULL;

    va_start(args, fmt);
    text = g_strdup_vprintf(fmt, args);
    va_end(args);

    gtk_label_set_markup(GTK_LABEL(label), text);
    g_free(text);
}

static void set_label_text_trimmed(GtkWidget *label, const gchar *text, const gchar *fallback) {
    gchar *copy = NULL;
    const gchar *final_text = fallback ? fallback : "";

    if (text) {
        copy = g_strdup(text);
        g_strstrip(copy);
        if (copy[0] != '\0') {
            final_text = copy;
        }
    }

    gtk_label_set_text(GTK_LABEL(label), final_text);
    g_free(copy);
}

static gchar *extract_status_value(const gchar *text, const gchar *prefix) {
    gchar **lines = NULL;
    gchar *value = NULL;
    gsize prefix_len = 0;

    if (!text || !prefix) {
        return NULL;
    }

    prefix_len = strlen(prefix);
    lines = g_strsplit(text, "\n", -1);
    for (gint i = 0; lines && lines[i]; ++i) {
        if (g_str_has_prefix(lines[i], prefix)) {
            const gchar *start = lines[i] + prefix_len;
            while (*start == ' ' || *start == '\t') {
                start++;
            }
            value = g_strdup(start);
            break;
        }
    }

    g_strfreev(lines);
    return value;
}

static gboolean command_is_available(const gchar *command_line) {
    gchar **argv = NULL;
    gint argc = 0;
    GError *error = NULL;
    gchar *found = NULL;
    gboolean ok = FALSE;

    if (!command_line || *command_line == '\0') {
        return FALSE;
    }

    if (!g_shell_parse_argv(command_line, &argc, &argv, &error)) {
        g_clear_error(&error);
        return FALSE;
    }

    if (argc > 0 && argv[0]) {
        found = g_find_program_in_path(argv[0]);
        ok = (found != NULL);
    }

    g_free(found);
    g_strfreev(argv);
    return ok;
}

static void style_action_button(GtkWidget *button, gboolean accent) {
    GtkStyleContext *ctx = gtk_widget_get_style_context(button);
    gtk_style_context_add_class(ctx, "action-btn");
    if (accent) {
        gtk_style_context_add_class(ctx, "accent-btn");
    }
}

static gchar *run_command_capture(const gchar *command, gint *exit_status) {
    gchar *stdout_text = NULL;
    gchar *stderr_text = NULL;
    gchar *result = NULL;
    GError *error = NULL;
    gint status = 0;

    if (!g_spawn_command_line_sync(command, &stdout_text, &stderr_text, &status, &error)) {
        result = g_strdup_printf("Failed to run: %s", error ? error->message : "unknown error");
        g_clear_error(&error);
        g_free(stdout_text);
        g_free(stderr_text);
        if (exit_status) {
            *exit_status = -1;
        }
        return result;
    }

    if (exit_status) {
        *exit_status = status;
    }

    if (stderr_text && *stderr_text) {
        result = g_strconcat(stdout_text ? stdout_text : "", stderr_text, NULL);
    } else {
        result = g_strdup(stdout_text ? stdout_text : "");
    }

    g_free(stdout_text);
    g_free(stderr_text);
    return result;
}

static void set_clipboard_text(GtkWidget *widget, const gchar *text) {
    const gchar *payload = text ? text : "";
    GtkClipboard *clipboard = gtk_widget_get_clipboard(widget, GDK_SELECTION_CLIPBOARD);
    GtkClipboard *primary = gtk_widget_get_clipboard(widget, GDK_SELECTION_PRIMARY);
    gtk_clipboard_set_text(clipboard, payload, -1);
    gtk_clipboard_store(clipboard);
    gtk_clipboard_set_text(primary, payload, -1);
}

static void launch_in_background(const gchar *command) {
    GError *error = NULL;
    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Launch failed (%s): %s", command, error ? error->message : "unknown");
        g_clear_error(&error);
    }
}

static gboolean parse_manual_temp(const gchar *text, gint *temp_out) {
    const gchar *needle = "Manual mode: ";
    const gchar *start = NULL;
    gchar *end = NULL;
    long value = 0;

    if (!text) {
        return FALSE;
    }

    start = g_strstr_len(text, -1, needle);
    if (!start) {
        return FALSE;
    }
    start += strlen(needle);

    if (!g_ascii_isdigit(*start)) {
        return FALSE;
    }

    value = g_ascii_strtoll(start, &end, 10);
    if (!end || *end != 'K' || value < 1000 || value > 25000) {
        return FALSE;
    }

    *temp_out = (gint)value;
    return TRUE;
}

static void night_update_slider_label(AppState *state) {
    gint temp = (gint)gtk_range_get_value(GTK_RANGE(state->night_scale));
    set_label_markupf(state->night_temp_label, "Target Temperature: <b>%dK</b>", temp);
}

static void night_sync_switch_from_output(AppState *state, const gchar *output) {
    gchar *auto_process = extract_status_value(output, "Auto/background process:");
    gboolean active = FALSE;

    if (!state->night_auto_switch) {
        g_free(auto_process);
        return;
    }

    if (auto_process &&
        (g_strcmp0(auto_process, "running") == 0 ||
         g_strcmp0(auto_process, "on") == 0 ||
         g_strcmp0(auto_process, "enabled") == 0)) {
        active = TRUE;
    }

    state->night_switch_syncing = TRUE;
    gtk_switch_set_active(GTK_SWITCH(state->night_auto_switch), active);
    state->night_switch_syncing = FALSE;
    g_free(auto_process);
}

static void night_update_meta_from_output(AppState *state, const gchar *output) {
    gchar *local_time = extract_status_value(output, "Local time:");
    gchar *period = extract_status_value(output, "Solar period:");
    gchar *location = extract_status_value(output, "Solar location:");
    gchar *schedule = extract_status_value(output, "Schedule:");
    GString *meta = g_string_new(NULL);

    if (local_time && *local_time) {
        g_string_append(meta, local_time);
    }
    if (period && *period) {
        if (meta->len > 0) {
            g_string_append(meta, "  •  ");
        }
        g_string_append_printf(meta, "Sun: %s", period);
    }
    if (location && *location) {
        if (meta->len > 0) {
            g_string_append(meta, "  •  ");
        }
        g_string_append_printf(meta, "Location: %s", location);
    }
    if (schedule && *schedule) {
        if (meta->len > 0) {
            g_string_append(meta, "  •  ");
        }
        g_string_append(meta, schedule);
    }

    if (state->night_meta_label) {
        if (meta->len > 0) {
            gtk_label_set_text(GTK_LABEL(state->night_meta_label), meta->str);
        } else {
            gtk_label_set_text(GTK_LABEL(state->night_meta_label),
                               "No solar data. Set REDSHIFT_LOCATION in redshift.sh for local day/night tracking.");
        }
    }

    g_string_free(meta, TRUE);
    g_free(local_time);
    g_free(period);
    g_free(location);
    g_free(schedule);
}

static void night_refresh_status(AppState *state) {
    gchar *quoted = g_shell_quote(state->redshift_script);
    gchar *cmd = g_strdup_printf("%s status", quoted);
    gchar *output = run_command_capture(cmd, NULL);
    gint manual_temp = 0;
    gchar *mode = extract_status_value(output, "Mode:");
    gchar *manual = extract_status_value(output, "Manual mode:");
    gchar *auto_process = extract_status_value(output, "Auto/background process:");
    GString *primary = g_string_new(NULL);

    if (mode && *mode) {
        g_string_append_printf(primary, "Mode: %s", mode);
    }
    if (manual && *manual) {
        if (primary->len > 0) {
            g_string_append(primary, "  •  ");
        }
        g_string_append_printf(primary, "Manual: %s", manual);
    }
    if (auto_process && *auto_process) {
        if (primary->len > 0) {
            g_string_append(primary, "  •  ");
        }
        g_string_append_printf(primary, "Auto: %s", auto_process);
    }

    if (primary->len > 0) {
        set_label_text_trimmed(state->night_status, primary->str, "No status available.");
    } else if (output && *output) {
        set_label_text_trimmed(state->night_status, output, "No status available.");
    } else {
        set_label_text_trimmed(state->night_status, NULL, "No status available.");
    }

    night_update_meta_from_output(state, output);
    night_sync_switch_from_output(state, output);

    if (parse_manual_temp(output, &manual_temp)) {
        gtk_range_set_value(GTK_RANGE(state->night_scale), manual_temp);
    }
    night_update_slider_label(state);

    g_string_free(primary, TRUE);
    g_free(mode);
    g_free(manual);
    g_free(auto_process);
    g_free(output);
    g_free(cmd);
    g_free(quoted);
}

static void night_run_command(AppState *state, const gchar *subcommand) {
    gchar *quoted_script = g_shell_quote(state->redshift_script);
    gchar *cmd = g_strdup_printf("%s %s", quoted_script, subcommand);
    gint status = 0;
    gchar *output = run_command_capture(cmd, &status);

    if (status != 0 && output) {
        set_label_text_trimmed(state->night_status, output, "Command failed.");
    }

    night_refresh_status(state);

    g_free(output);
    g_free(cmd);
    g_free(quoted_script);
}

static void on_night_scale_changed(GtkRange *range, gpointer user_data) {
    (void)range;
    AppState *state = user_data;
    night_update_slider_label(state);
}

static void on_night_apply_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    gint temp = (gint)gtk_range_get_value(GTK_RANGE(state->night_scale));
    gchar *subcmd = g_strdup_printf("set %d", temp);
    night_run_command(state, subcmd);
    g_free(subcmd);
}

static void on_night_off_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    night_run_command(user_data, "off");
}

static void on_night_warm_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    night_run_command(user_data, "warm");
}

static void on_night_cool_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    night_run_command(user_data, "cool");
}

static void on_night_status_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    night_refresh_status(user_data);
}

static void on_night_auto_switch_notify(GObject *obj, GParamSpec *pspec, gpointer user_data) {
    AppState *state = user_data;
    gboolean active = FALSE;
    (void)pspec;

    if (!state || state->night_switch_syncing) {
        return;
    }

    active = gtk_switch_get_active(GTK_SWITCH(obj));
    if (active) {
        night_run_command(state, "auto");
    } else {
        night_run_command(state, "off");
    }
}

static gboolean parse_first_percent(const gchar *text, gint *volume_out) {
    const gchar *p = text;
    gchar *end = NULL;
    long value = 0;

    if (!text || !volume_out) {
        return FALSE;
    }

    while (*p) {
        if (g_ascii_isdigit(*p)) {
            value = g_ascii_strtoll(p, &end, 10);
            if (end && *end == '%' && value >= 0 && value <= 200) {
                *volume_out = (gint)value;
                return TRUE;
            }
        }
        p++;
    }
    return FALSE;
}

static void audio_update_slider_label(AppState *state) {
    gint percent = (gint)gtk_range_get_value(GTK_RANGE(state->audio_scale));
    set_label_markupf(state->audio_volume_label, "Target Volume: <b>%d%%</b>", percent);
}

static void audio_refresh_status(AppState *state) {
    gint st_sink = 0;
    gint st_vol = 0;
    gint st_mute = 0;
    gint parsed_vol = 0;
    gchar *sink = run_command_capture("pactl get-default-sink", &st_sink);
    gchar *vol = run_command_capture("pactl get-sink-volume @DEFAULT_SINK@", &st_vol);
    gchar *mute = run_command_capture("pactl get-sink-mute @DEFAULT_SINK@", &st_mute);
    GString *text = g_string_new(NULL);

    if (st_sink != 0 || st_vol != 0 || st_mute != 0) {
        g_string_append(text, "Audio tools unavailable. Install pulseaudio-utils and pavucontrol.");
    } else {
        g_string_append_printf(text,
                               "Sink: %sVolume: %sMute: %s",
                               sink ? sink : "",
                               vol ? vol : "",
                               mute ? mute : "");
        if (parse_first_percent(vol, &parsed_vol)) {
            gtk_range_set_value(GTK_RANGE(state->audio_scale), parsed_vol);
        }
    }

    if (state->audio_status) {
        set_label_text_trimmed(state->audio_status, text->str, "Audio status unavailable.");
    }
    audio_update_slider_label(state);

    g_free(sink);
    g_free(vol);
    g_free(mute);
    g_string_free(text, TRUE);
}

static void on_audio_scale_changed(GtkRange *range, gpointer user_data) {
    (void)range;
    audio_update_slider_label(user_data);
}

static void on_audio_apply_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    gint value = (gint)gtk_range_get_value(GTK_RANGE(state->audio_scale));
    gchar *cmd = g_strdup_printf("pactl set-sink-volume @DEFAULT_SINK@ %d%%", value);
    gint status = 0;
    gchar *output = run_command_capture(cmd, &status);

    if (status != 0 && output) {
        set_label_text_trimmed(state->audio_status, output, "Volume apply failed.");
    }
    audio_refresh_status(state);

    g_free(output);
    g_free(cmd);
}

static void on_audio_down_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gint status = 0;
    gchar *output = run_command_capture("pactl set-sink-volume @DEFAULT_SINK@ -5%", &status);
    if (status != 0) {
        g_warning("Audio down failed: %s", output ? output : "unknown");
    }
    g_free(output);
    audio_refresh_status(user_data);
}

static void on_audio_up_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gint status = 0;
    gchar *output = run_command_capture("pactl set-sink-volume @DEFAULT_SINK@ +5%", &status);
    if (status != 0) {
        g_warning("Audio up failed: %s", output ? output : "unknown");
    }
    g_free(output);
    audio_refresh_status(user_data);
}

static void on_audio_mute_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    gint status = 0;
    gchar *output = run_command_capture("pactl set-sink-mute @DEFAULT_SINK@ toggle", &status);
    if (status != 0) {
        g_warning("Audio mute toggle failed: %s", output ? output : "unknown");
    }
    g_free(output);
    audio_refresh_status(user_data);
}

static void on_audio_refresh_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    audio_refresh_status(user_data);
}

static void on_audio_open_pavucontrol_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    gint status = 0;
    gchar *which = run_command_capture("command -v pavucontrol", &status);
    (void)button;
    if (status == 0 && which && *which) {
        launch_in_background("pavucontrol");
    } else {
        set_label_text_trimmed(state->audio_status,
                               "pavucontrol not found. Install with: sudo apt install pavucontrol",
                               "pavucontrol not found.");
    }
    g_free(which);
}

static void free_shot_mark(gpointer data) {
    ShotMark *mark = data;
    if (!mark) {
        return;
    }
    g_free(mark->text);
    g_free(mark);
}

static void shots_editor_ensure_marks(AppState *state) {
    if (!state) {
        return;
    }
    if (!state->shots_editor_marks) {
        state->shots_editor_marks = g_ptr_array_new_with_free_func(free_shot_mark);
    }
}

static void shots_editor_set_status(AppState *state, const gchar *text) {
    if (!state || !state->shots_editor_status) {
        return;
    }
    set_label_text_trimmed(state->shots_editor_status, text, "Editor ready.");
}

static gboolean parse_positive_int_value(const gchar *text, gint *value_out) {
    const gchar *start = text;
    gchar *end = NULL;
    glong value = 0;

    if (!text) {
        return FALSE;
    }
    while (*start && g_ascii_isspace(*start)) {
        start++;
    }
    if (!g_ascii_isdigit(*start)) {
        return FALSE;
    }

    value = g_ascii_strtoll(start, &end, 10);
    if (!end) {
        return FALSE;
    }
    while (*end && g_ascii_isspace(*end)) {
        end++;
    }
    if (*end != '\0') {
        return FALSE;
    }
    if (value < 1 || value > G_MAXINT) {
        return FALSE;
    }
    if (value_out) {
        *value_out = (gint)value;
    }
    return TRUE;
}

static void shots_editor_update_step_label(AppState *state) {
    gchar *msg = NULL;
    gint next = 1;

    if (!state || !state->shots_editor_step_label) {
        return;
    }
    if (state->shots_editor_next_step > 0) {
        next = state->shots_editor_next_step;
    }
    msg = g_strdup_printf("Next step: %d", next);
    gtk_label_set_text(GTK_LABEL(state->shots_editor_step_label), msg);
    g_free(msg);
}

static gdouble shots_editor_get_arrow_width_control(AppState *state) {
    if (state && state->shots_editor_arrow_width_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_width_scale));
    }
    return 6.0;
}

static gdouble shots_editor_get_arrow_head_len_control(AppState *state) {
    if (state && state->shots_editor_arrow_head_len_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_head_len_scale));
    }
    return 20.0;
}

static gdouble shots_editor_get_arrow_head_spread_control(AppState *state) {
    gdouble degrees = 25.0;
    if (state && state->shots_editor_arrow_head_angle_scale) {
        degrees = gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_head_angle_scale));
    }
    return degrees * (G_PI / 180.0);
}

static void shots_editor_apply_arrow_style_to_mark(AppState *state, ShotMark *mark) {
    if (!mark || mark->type != SHOT_MARK_ARROW) {
        return;
    }
    mark->stroke_width = shots_editor_get_arrow_width_control(state);
    mark->arrow_head_len = shots_editor_get_arrow_head_len_control(state);
    mark->arrow_head_spread = shots_editor_get_arrow_head_spread_control(state);
}

static ShotMark *shots_editor_get_selected_mark(AppState *state) {
    if (!state || state->shots_editor_selected_mark_index < 0) {
        return NULL;
    }
    shots_editor_ensure_marks(state);
    if ((guint)state->shots_editor_selected_mark_index >= state->shots_editor_marks->len) {
        return NULL;
    }
    return g_ptr_array_index(state->shots_editor_marks, state->shots_editor_selected_mark_index);
}

static void shots_editor_sync_controls_from_selected_arrow(AppState *state) {
    ShotMark *mark = shots_editor_get_selected_mark(state);
    gdouble width = 6.0;
    gdouble head_len = 20.0;
    gdouble head_angle_deg = 25.0;

    if (!state || !mark || mark->type != SHOT_MARK_ARROW) {
        return;
    }
    if (mark->stroke_width > 0.0) {
        width = mark->stroke_width;
    }
    if (mark->arrow_head_len > 0.0) {
        head_len = mark->arrow_head_len;
    }
    if (mark->arrow_head_spread > 0.0) {
        head_angle_deg = mark->arrow_head_spread * (180.0 / G_PI);
    }

    state->shots_editor_style_syncing = TRUE;
    if (state->shots_editor_arrow_width_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_width_scale), width);
    }
    if (state->shots_editor_arrow_head_len_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_head_len_scale), head_len);
    }
    if (state->shots_editor_arrow_head_angle_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_head_angle_scale), head_angle_deg);
    }
    state->shots_editor_style_syncing = FALSE;
}

static void shots_editor_sync_controls_from_selected_mark(AppState *state) {
    ShotMark *mark = shots_editor_get_selected_mark(state);

    if (!state || !mark) {
        return;
    }

    state->shots_editor_style_syncing = TRUE;
    if (state->shots_editor_color_btn) {
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), &mark->color);
    }
    if (state->shots_editor_text_entry &&
        (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_STAMP || mark->type == SHOT_MARK_RECT)) {
        gtk_entry_set_text(GTK_ENTRY(state->shots_editor_text_entry), mark->text ? mark->text : "");
    }
    state->shots_editor_style_syncing = FALSE;

    if (mark->type == SHOT_MARK_ARROW) {
        shots_editor_sync_controls_from_selected_arrow(state);
    }
}

static gboolean shots_editor_apply_color_to_selected(AppState *state, const GdkRGBA *color, gboolean sync_picker) {
    ShotMark *mark = shots_editor_get_selected_mark(state);

    if (!state || !mark || !color) {
        return FALSE;
    }

    mark->color = *color;
    if (sync_picker && state->shots_editor_color_btn) {
        state->shots_editor_style_syncing = TRUE;
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), color);
        state->shots_editor_style_syncing = FALSE;
    }
    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
    return TRUE;
}

static void shots_editor_set_selected_mark(AppState *state, gint index) {
    if (!state) {
        return;
    }
    shots_editor_ensure_marks(state);

    if (index < 0 || (guint)index >= state->shots_editor_marks->len) {
        state->shots_editor_selected_mark_index = -1;
    } else {
        state->shots_editor_selected_mark_index = index;
        shots_editor_sync_controls_from_selected_mark(state);
    }
    state->shots_editor_selected_arrow_handle = 0;
    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
}

static void shots_editor_recompute_next_step(AppState *state) {
    gint max_step = 0;

    if (!state) {
        return;
    }
    shots_editor_ensure_marks(state);

    for (guint i = 0; i < state->shots_editor_marks->len; ++i) {
        ShotMark *mark = g_ptr_array_index(state->shots_editor_marks, i);
        gint value = 0;
        if (!mark || mark->type != SHOT_MARK_STAMP) {
            continue;
        }
        if (parse_positive_int_value(mark->text, &value) && value > max_step) {
            max_step = value;
        }
    }
    state->shots_editor_next_step = max_step + 1;
    if (state->shots_editor_next_step < 1) {
        state->shots_editor_next_step = 1;
    }
    shots_editor_update_step_label(state);
}

static void shots_editor_clear_marks(AppState *state) {
    if (!state) {
        return;
    }
    shots_editor_ensure_marks(state);
    while (state->shots_editor_marks->len > 0) {
        g_ptr_array_remove_index(state->shots_editor_marks, state->shots_editor_marks->len - 1);
    }
    state->shots_editor_edit_dragging = FALSE;
    state->shots_editor_dragging = FALSE;
    shots_editor_set_selected_mark(state, -1);
    shots_editor_recompute_next_step(state);
    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
}

static const gchar *shots_editor_get_active_tool(AppState *state) {
    const gchar *id = NULL;

    if (!state || !state->shots_editor_tool_combo) {
        return "arrow";
    }

    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(state->shots_editor_tool_combo));
    return id ? id : "arrow";
}

static gboolean shots_editor_widget_to_image(AppState *state,
                                             gdouble widget_x,
                                             gdouble widget_y,
                                             gdouble *image_x,
                                             gdouble *image_y,
                                             gboolean clamp) {
    gdouble x = 0.0;
    gdouble y = 0.0;
    gint width = 0;
    gint height = 0;

    if (!state || !state->shots_editor_pixbuf || state->shots_editor_draw_scale <= 0.0) {
        return FALSE;
    }

    width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
    height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);

    x = (widget_x - state->shots_editor_draw_offset_x) / state->shots_editor_draw_scale;
    y = (widget_y - state->shots_editor_draw_offset_y) / state->shots_editor_draw_scale;

    if (clamp) {
        if (x < 0.0) {
            x = 0.0;
        }
        if (y < 0.0) {
            y = 0.0;
        }
        if (x > (gdouble)(width - 1)) {
            x = (gdouble)(width - 1);
        }
        if (y > (gdouble)(height - 1)) {
            y = (gdouble)(height - 1);
        }
    } else if (x < 0.0 || y < 0.0 || x > (gdouble)(width - 1) || y > (gdouble)(height - 1)) {
        return FALSE;
    }

    if (image_x) {
        *image_x = x;
    }
    if (image_y) {
        *image_y = y;
    }
    return TRUE;
}

static gdouble shots_editor_dist_sq(gdouble x1, gdouble y1, gdouble x2, gdouble y2) {
    gdouble dx = x1 - x2;
    gdouble dy = y1 - y2;
    return dx * dx + dy * dy;
}

static gdouble shots_editor_point_segment_distance(gdouble px,
                                                   gdouble py,
                                                   gdouble x1,
                                                   gdouble y1,
                                                   gdouble x2,
                                                   gdouble y2) {
    gdouble seg_dx = x2 - x1;
    gdouble seg_dy = y2 - y1;
    gdouble seg_len_sq = seg_dx * seg_dx + seg_dy * seg_dy;
    gdouble t = 0.0;
    gdouble proj_x = 0.0;
    gdouble proj_y = 0.0;

    if (seg_len_sq <= 1e-9) {
        return sqrt(shots_editor_dist_sq(px, py, x1, y1));
    }

    t = ((px - x1) * seg_dx + (py - y1) * seg_dy) / seg_len_sq;
    if (t < 0.0) {
        t = 0.0;
    }
    if (t > 1.0) {
        t = 1.0;
    }
    proj_x = x1 + t * seg_dx;
    proj_y = y1 + t * seg_dy;
    return sqrt(shots_editor_dist_sq(px, py, proj_x, proj_y));
}

static gint shots_editor_find_mark_at_point(AppState *state, gdouble x, gdouble y, gint *arrow_handle_out) {
    const gdouble endpoint_threshold = 14.0;
    const gdouble line_threshold = 10.0;

    shots_editor_ensure_marks(state);
    if (!state || !state->shots_editor_marks) {
        return -1;
    }

    if (arrow_handle_out) {
        *arrow_handle_out = 0;
    }

    for (gint i = (gint)state->shots_editor_marks->len - 1; i >= 0; --i) {
        ShotMark *mark = g_ptr_array_index(state->shots_editor_marks, (guint)i);
        if (!mark) {
            continue;
        }

        if (mark->type == SHOT_MARK_ARROW) {
            if (sqrt(shots_editor_dist_sq(x, y, mark->x2, mark->y2)) <= endpoint_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = 2;
                }
                return i;
            }
            if (sqrt(shots_editor_dist_sq(x, y, mark->x1, mark->y1)) <= endpoint_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = 1;
                }
                return i;
            }
            if (shots_editor_point_segment_distance(x, y, mark->x1, mark->y1, mark->x2, mark->y2) <= line_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = 3;
                }
                return i;
            }
            continue;
        }

        if (mark->type == SHOT_MARK_STAMP) {
            if (sqrt(shots_editor_dist_sq(x, y, mark->x1, mark->y1)) <= 24.0) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_MOVE;
                }
                return i;
            }
            continue;
        }

        if (mark->type == SHOT_MARK_RECT) {
            gdouble left = MIN(mark->x1, mark->x2);
            gdouble right = MAX(mark->x1, mark->x2);
            gdouble top = MIN(mark->y1, mark->y2);
            gdouble bottom = MAX(mark->y1, mark->y2);

            if (sqrt(shots_editor_dist_sq(x, y, mark->x1, mark->y1)) <= endpoint_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_RECT_START;
                }
                return i;
            }
            if (sqrt(shots_editor_dist_sq(x, y, mark->x2, mark->y2)) <= endpoint_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_RECT_END;
                }
                return i;
            }
            if (x >= left && x <= right && y >= top && y <= bottom) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_MOVE;
                }
                return i;
            }
            continue;
        }

        if (mark->type == SHOT_MARK_TEXT) {
            gdouble text_w = 14.0 * (gdouble)MAX(1, (gint)strlen(mark->text ? mark->text : "Note"));
            gdouble text_h = 28.0;
            if (x >= mark->x1 && x <= mark->x1 + text_w &&
                y <= mark->y1 && y >= mark->y1 - text_h) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_MOVE;
                }
                return i;
            }
        }
    }

    return -1;
}

static void shots_editor_draw_arrow(cairo_t *cr, const ShotMark *mark) {
    gdouble angle = 0.0;
    gdouble head = 20.0;
    gdouble spread = G_PI / 7.0;
    gdouble width = 6.0;

    if (!cr || !mark) {
        return;
    }

    if (mark->stroke_width > 0.0) {
        width = mark->stroke_width;
    }
    if (mark->arrow_head_len > 0.0) {
        head = mark->arrow_head_len;
    }
    if (mark->arrow_head_spread > 0.0) {
        spread = mark->arrow_head_spread;
    }

    angle = atan2(mark->y2 - mark->y1, mark->x2 - mark->x1);

    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 1.0);
    cairo_set_line_width(cr, width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_move_to(cr, mark->x1, mark->y1);
    cairo_line_to(cr, mark->x2, mark->y2);
    cairo_stroke(cr);

    cairo_move_to(cr, mark->x2, mark->y2);
    cairo_line_to(cr,
                  mark->x2 - head * cos(angle - spread),
                  mark->y2 - head * sin(angle - spread));
    cairo_line_to(cr,
                  mark->x2 - head * cos(angle + spread),
                  mark->y2 - head * sin(angle + spread));
    cairo_close_path(cr);
    cairo_fill(cr);
}

static void shots_editor_draw_stamp(cairo_t *cr, const ShotMark *mark) {
    const gchar *text = (mark && mark->text && *mark->text) ? mark->text : "1";
    cairo_text_extents_t extents = {0};
    gdouble radius = 22.0;

    if (!cr || !mark) {
        return;
    }

    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 0.95);
    cairo_arc(cr, mark->x1, mark->y1, radius, 0, 2 * G_PI);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.95);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20.0);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr,
                  mark->x1 - (extents.width / 2.0 + extents.x_bearing),
                  mark->y1 - (extents.height / 2.0 + extents.y_bearing));
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_show_text(cr, text);
}

static void shots_editor_draw_text(cairo_t *cr, const ShotMark *mark) {
    const gchar *text = (mark && mark->text && *mark->text) ? mark->text : "Note";

    if (!cr || !mark) {
        return;
    }

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 24.0);

    cairo_move_to(cr, mark->x1 + 2.0, mark->y1 + 2.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.45);
    cairo_show_text(cr, text);

    cairo_move_to(cr, mark->x1, mark->y1);
    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 1.0);
    cairo_show_text(cr, text);
}

static void shots_editor_draw_rect(cairo_t *cr, const ShotMark *mark) {
    gdouble left = 0.0;
    gdouble right = 0.0;
    gdouble top = 0.0;
    gdouble bottom = 0.0;
    gdouble width = 0.0;
    gdouble height = 0.0;
    const gchar *text = NULL;

    if (!cr || !mark) {
        return;
    }

    left = MIN(mark->x1, mark->x2);
    right = MAX(mark->x1, mark->x2);
    top = MIN(mark->y1, mark->y2);
    bottom = MAX(mark->y1, mark->y2);
    width = right - left;
    height = bottom - top;
    if (width < 2.0 || height < 2.0) {
        return;
    }

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.28);
    cairo_rectangle(cr, left + 4.0, top + 4.0, width, height);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 0.18);
    cairo_rectangle(cr, left, top, width, height);
    cairo_fill_preserve(cr);

    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 1.0);
    cairo_set_line_width(cr, mark->stroke_width > 0.0 ? mark->stroke_width : 3.0);
    cairo_stroke(cr);

    text = (mark->text && *mark->text) ? mark->text : "Callout";
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 19.0);
    cairo_move_to(cr, left + 12.0, top + 30.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.4);
    cairo_show_text(cr, text);
    cairo_move_to(cr, left + 10.0, top + 28.0);
    cairo_set_source_rgba(cr, 0.98, 0.99, 1.0, 0.95);
    cairo_show_text(cr, text);
}

static void shots_editor_draw_mark(cairo_t *cr, const ShotMark *mark) {
    if (!cr || !mark) {
        return;
    }
    switch (mark->type) {
        case SHOT_MARK_ARROW:
            shots_editor_draw_arrow(cr, mark);
            break;
        case SHOT_MARK_STAMP:
            shots_editor_draw_stamp(cr, mark);
            break;
        case SHOT_MARK_TEXT:
            shots_editor_draw_text(cr, mark);
            break;
        case SHOT_MARK_RECT:
            shots_editor_draw_rect(cr, mark);
            break;
        default:
            break;
    }
}

static void shots_editor_draw_marks(AppState *state, cairo_t *cr, gboolean include_preview_arrow) {
    if (!state || !cr) {
        return;
    }

    shots_editor_ensure_marks(state);
    for (guint i = 0; i < state->shots_editor_marks->len; ++i) {
        ShotMark *mark = g_ptr_array_index(state->shots_editor_marks, i);
        shots_editor_draw_mark(cr, mark);
    }

    if (include_preview_arrow && state->shots_editor_dragging) {
        ShotMark preview = {0};
        GdkRGBA color = {0};
        gchar *text = NULL;

        if (state->shots_editor_color_btn) {
            gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), &color);
        } else {
            color.red = 0.95;
            color.green = 0.24;
            color.blue = 0.24;
            color.alpha = 1.0;
        }

        preview.type = state->shots_editor_drag_mark_type;
        preview.x1 = state->shots_editor_drag_start_x;
        preview.y1 = state->shots_editor_drag_start_y;
        preview.x2 = state->shots_editor_drag_cur_x;
        preview.y2 = state->shots_editor_drag_cur_y;
        preview.stroke_width = shots_editor_get_arrow_width_control(state);
        preview.arrow_head_len = shots_editor_get_arrow_head_len_control(state);
        preview.arrow_head_spread = shots_editor_get_arrow_head_spread_control(state);
        preview.color = color;
        if (preview.type == SHOT_MARK_RECT && state->shots_editor_text_entry) {
            text = g_strdup(gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));
            if (text) {
                g_strstrip(text);
            }
            preview.text = (text && *text) ? text : "Callout";
        }
        shots_editor_draw_mark(cr, &preview);
        g_free(text);
    }
}

static void shots_editor_draw_selection_overlay(AppState *state, cairo_t *cr) {
    ShotMark *mark = shots_editor_get_selected_mark(state);

    if (!state || !cr || !mark) {
        return;
    }

    if (mark->type == SHOT_MARK_ARROW) {
        cairo_set_line_width(cr, 2.0);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.75);
        cairo_move_to(cr, mark->x1, mark->y1);
        cairo_line_to(cr, mark->x2, mark->y2);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.08, 0.55, 0.98, 0.95);
        cairo_arc(cr, mark->x1, mark->y1, 6.0, 0, 2 * G_PI);
        cairo_fill(cr);
        cairo_arc(cr, mark->x2, mark->y2, 6.0, 0, 2 * G_PI);
        cairo_fill(cr);
    } else if (mark->type == SHOT_MARK_STAMP) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);
        cairo_set_line_width(cr, 2.0);
        cairo_arc(cr, mark->x1, mark->y1, 26.0, 0, 2 * G_PI);
        cairo_stroke(cr);
    } else if (mark->type == SHOT_MARK_TEXT) {
        gdouble w = 14.0 * (gdouble)MAX(1, (gint)strlen(mark->text ? mark->text : "Note"));
        gdouble h = 28.0;
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);
        cairo_set_line_width(cr, 2.0);
        cairo_rectangle(cr, mark->x1 - 3.0, mark->y1 - h, w + 6.0, h + 6.0);
        cairo_stroke(cr);
    } else if (mark->type == SHOT_MARK_RECT) {
        gdouble left = MIN(mark->x1, mark->x2);
        gdouble right = MAX(mark->x1, mark->x2);
        gdouble top = MIN(mark->y1, mark->y2);
        gdouble bottom = MAX(mark->y1, mark->y2);

        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);
        cairo_set_line_width(cr, 2.0);
        cairo_rectangle(cr, left - 2.0, top - 2.0, (right - left) + 4.0, (bottom - top) + 4.0);
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.08, 0.55, 0.98, 0.95);
        cairo_arc(cr, mark->x1, mark->y1, 5.5, 0, 2 * G_PI);
        cairo_fill(cr);
        cairo_arc(cr, mark->x2, mark->y2, 5.5, 0, 2 * G_PI);
        cairo_fill(cr);
    }
}

static gboolean on_shots_editor_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    AppState *state = user_data;
    GtkAllocation alloc = {0};
    gint img_w = 0;
    gint img_h = 0;
    gdouble scale = 1.0;
    gdouble draw_w = 0.0;
    gdouble draw_h = 0.0;

    gtk_widget_get_allocation(widget, &alloc);

    cairo_set_source_rgb(cr, 0.11, 0.14, 0.18);
    cairo_paint(cr);

    if (!state || !state->shots_editor_pixbuf) {
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 15.0);
        cairo_set_source_rgba(cr, 0.82, 0.85, 0.91, 0.85);
        cairo_move_to(cr, 18.0, (gdouble)alloc.height / 2.0);
        cairo_show_text(cr, "Select one screenshot to open the quick editor.");
        return FALSE;
    }

    img_w = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
    img_h = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
    scale = MIN((gdouble)alloc.width / (gdouble)img_w, (gdouble)alloc.height / (gdouble)img_h);
    if (scale <= 0.0) {
        scale = 1.0;
    }
    draw_w = (gdouble)img_w * scale;
    draw_h = (gdouble)img_h * scale;

    state->shots_editor_draw_scale = scale;
    state->shots_editor_draw_offset_x = ((gdouble)alloc.width - draw_w) / 2.0;
    state->shots_editor_draw_offset_y = ((gdouble)alloc.height - draw_h) / 2.0;

    cairo_save(cr);
    cairo_translate(cr, state->shots_editor_draw_offset_x, state->shots_editor_draw_offset_y);
    cairo_scale(cr, scale, scale);
    gdk_cairo_set_source_pixbuf(cr, state->shots_editor_pixbuf, 0, 0);
    cairo_rectangle(cr, 0, 0, img_w, img_h);
    cairo_fill(cr);
    shots_editor_draw_marks(state, cr, TRUE);
    shots_editor_draw_selection_overlay(state, cr);
    cairo_restore(cr);

    return FALSE;
}

static gboolean on_shots_editor_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppState *state = user_data;
    const gchar *tool = NULL;
    gdouble ix = 0.0;
    gdouble iy = 0.0;
    gint hit_index = -1;
    gint arrow_handle = 0;
    GdkRGBA color = {0.95, 0.24, 0.24, 1.0};

    (void)widget;
    if (!state || !state->shots_editor_pixbuf) {
        return FALSE;
    }

    if (!shots_editor_widget_to_image(state, event->x, event->y, &ix, &iy, FALSE)) {
        return FALSE;
    }

    if (event->button == 3) {
        hit_index = shots_editor_find_mark_at_point(state, ix, iy, &arrow_handle);
        if (hit_index >= 0) {
            shots_editor_set_selected_mark(state, hit_index);
        }
        shots_editor_show_color_palette_menu(state, event);
        return TRUE;
    }
    if (event->button != 1) {
        return FALSE;
    }

    if (state->shots_editor_color_btn) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), &color);
    }

    shots_editor_ensure_marks(state);
    tool = shots_editor_get_active_tool(state);
    if (g_strcmp0(tool, "select") == 0) {
        hit_index = shots_editor_find_mark_at_point(state, ix, iy, &arrow_handle);
        if (hit_index < 0) {
            shots_editor_set_selected_mark(state, -1);
            shots_editor_set_status(state, "No annotation at click point.");
            return TRUE;
        }

        shots_editor_set_selected_mark(state, hit_index);
        if ((guint)hit_index < state->shots_editor_marks->len) {
            ShotMark *mark = g_ptr_array_index(state->shots_editor_marks, (guint)hit_index);
            if (mark && arrow_handle > SHOT_HANDLE_NONE) {
                state->shots_editor_selected_arrow_handle = arrow_handle;
                state->shots_editor_edit_dragging = TRUE;
                state->shots_editor_edit_anchor_x = ix;
                state->shots_editor_edit_anchor_y = iy;
                state->shots_editor_edit_origin_x1 = mark->x1;
                state->shots_editor_edit_origin_y1 = mark->y1;
                state->shots_editor_edit_origin_x2 = mark->x2;
                state->shots_editor_edit_origin_y2 = mark->y2;

                if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_START) {
                    shots_editor_set_status(state, "Editing arrow start. Drag the blue handle.");
                } else if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_END) {
                    shots_editor_set_status(state, "Editing arrow head. Drag the tip handle.");
                } else if (mark->type == SHOT_MARK_ARROW) {
                    shots_editor_set_status(state, "Editing arrow path. Drag to move the full arrow.");
                } else if (mark->type == SHOT_MARK_RECT && arrow_handle == SHOT_HANDLE_RECT_START) {
                    shots_editor_set_status(state, "Editing callout anchor corner.");
                } else if (mark->type == SHOT_MARK_RECT && arrow_handle == SHOT_HANDLE_RECT_END) {
                    shots_editor_set_status(state, "Editing callout opposite corner.");
                } else {
                    shots_editor_set_status(state, "Editing annotation position.");
                }
            } else {
                shots_editor_set_status(state, "Annotation selected.");
            }
        }
        return TRUE;
    }

    if (g_strcmp0(tool, "arrow") == 0) {
        shots_editor_set_selected_mark(state, -1);
        state->shots_editor_dragging = TRUE;
        state->shots_editor_drag_mark_type = SHOT_MARK_ARROW;
        state->shots_editor_drag_start_x = ix;
        state->shots_editor_drag_start_y = iy;
        state->shots_editor_drag_cur_x = ix;
        state->shots_editor_drag_cur_y = iy;
        gtk_widget_queue_draw(state->shots_editor_canvas);
        return TRUE;
    }

    if (g_strcmp0(tool, "rect") == 0) {
        shots_editor_set_selected_mark(state, -1);
        state->shots_editor_dragging = TRUE;
        state->shots_editor_drag_mark_type = SHOT_MARK_RECT;
        state->shots_editor_drag_start_x = ix;
        state->shots_editor_drag_start_y = iy;
        state->shots_editor_drag_cur_x = ix;
        state->shots_editor_drag_cur_y = iy;
        gtk_widget_queue_draw(state->shots_editor_canvas);
        return TRUE;
    }

    if (g_strcmp0(tool, "stamp") == 0) {
        ShotMark *mark = g_new0(ShotMark, 1);
        gchar *stamp_text = NULL;
        gboolean auto_step = FALSE;

        shots_editor_set_selected_mark(state, -1);
        mark->type = SHOT_MARK_STAMP;
        mark->x1 = ix;
        mark->y1 = iy;
        mark->x2 = ix;
        mark->y2 = iy;
        mark->color = color;
        auto_step = state->shots_editor_auto_step_check &&
                    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shots_editor_auto_step_check));
        if (auto_step) {
            gint step = state->shots_editor_next_step > 0 ? state->shots_editor_next_step : 1;
            mark->text = g_strdup_printf("%d", step);
        } else {
            stamp_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(state->shots_editor_stamp_combo));
            mark->text = stamp_text ? stamp_text : g_strdup("1");
        }

        g_ptr_array_add(state->shots_editor_marks, mark);
        shots_editor_set_selected_mark(state, (gint)state->shots_editor_marks->len - 1);
        shots_editor_recompute_next_step(state);
        gtk_widget_queue_draw(state->shots_editor_canvas);
        shots_editor_set_status(state, auto_step ? "Auto step stamp added." : "Stamp added.");
        return TRUE;
    }

    if (g_strcmp0(tool, "text") == 0) {
        ShotMark *mark = g_new0(ShotMark, 1);
        gchar *text = g_strdup(gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));

        shots_editor_set_selected_mark(state, -1);
        if (!text) {
            text = g_strdup("Note");
        } else {
            g_strstrip(text);
        }
        if (*text == '\0') {
            g_free(text);
            text = g_strdup("Note");
        }

        mark->type = SHOT_MARK_TEXT;
        mark->x1 = ix;
        mark->y1 = iy;
        mark->x2 = ix;
        mark->y2 = iy;
        mark->color = color;
        mark->text = text;
        g_ptr_array_add(state->shots_editor_marks, mark);
        shots_editor_set_selected_mark(state, (gint)state->shots_editor_marks->len - 1);
        gtk_widget_queue_draw(state->shots_editor_canvas);
        shots_editor_set_status(state, "Text annotation added.");
        return TRUE;
    }

    return FALSE;
}

static gboolean on_shots_editor_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data) {
    AppState *state = user_data;
    gdouble ix = 0.0;
    gdouble iy = 0.0;

    (void)widget;
    if (!state || !state->shots_editor_pixbuf) {
        return FALSE;
    }

    if (state->shots_editor_edit_dragging) {
        ShotMark *mark = shots_editor_get_selected_mark(state);
        if (!mark || mark->type != SHOT_MARK_ARROW) {
            if (!mark) {
                return FALSE;
            }
        }
        if (!shots_editor_widget_to_image(state, event->x, event->y, &ix, &iy, TRUE)) {
            return FALSE;
        }

        if (mark->type == SHOT_MARK_ARROW) {
            if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_ARROW_START) {
                mark->x1 = ix;
                mark->y1 = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_ARROW_END) {
                mark->x2 = ix;
                mark->y2 = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_MOVE) {
                gint width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
                gint height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
                gdouble dx = ix - state->shots_editor_edit_anchor_x;
                gdouble dy = iy - state->shots_editor_edit_anchor_y;
                gdouble min_dx = -MIN(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble max_dx = (gdouble)(width - 1) - MAX(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble min_dy = -MIN(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);
                gdouble max_dy = (gdouble)(height - 1) - MAX(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);

                if (dx < min_dx) {
                    dx = min_dx;
                }
                if (dx > max_dx) {
                    dx = max_dx;
                }
                if (dy < min_dy) {
                    dy = min_dy;
                }
                if (dy > max_dy) {
                    dy = max_dy;
                }

                mark->x1 = state->shots_editor_edit_origin_x1 + dx;
                mark->y1 = state->shots_editor_edit_origin_y1 + dy;
                mark->x2 = state->shots_editor_edit_origin_x2 + dx;
                mark->y2 = state->shots_editor_edit_origin_y2 + dy;
            }
        } else if (mark->type == SHOT_MARK_RECT) {
            if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_RECT_START) {
                mark->x1 = ix;
                mark->y1 = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_RECT_END) {
                mark->x2 = ix;
                mark->y2 = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_MOVE) {
                gint width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
                gint height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
                gdouble dx = ix - state->shots_editor_edit_anchor_x;
                gdouble dy = iy - state->shots_editor_edit_anchor_y;
                gdouble min_dx = -MIN(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble max_dx = (gdouble)(width - 1) - MAX(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble min_dy = -MIN(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);
                gdouble max_dy = (gdouble)(height - 1) - MAX(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);

                if (dx < min_dx) {
                    dx = min_dx;
                }
                if (dx > max_dx) {
                    dx = max_dx;
                }
                if (dy < min_dy) {
                    dy = min_dy;
                }
                if (dy > max_dy) {
                    dy = max_dy;
                }

                mark->x1 = state->shots_editor_edit_origin_x1 + dx;
                mark->y1 = state->shots_editor_edit_origin_y1 + dy;
                mark->x2 = state->shots_editor_edit_origin_x2 + dx;
                mark->y2 = state->shots_editor_edit_origin_y2 + dy;
            }
        } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_MOVE) {
            gint width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
            gint height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
            gdouble dx = ix - state->shots_editor_edit_anchor_x;
            gdouble dy = iy - state->shots_editor_edit_anchor_y;
            gdouble new_x = state->shots_editor_edit_origin_x1 + dx;
            gdouble new_y = state->shots_editor_edit_origin_y1 + dy;

            if (new_x < 0.0) {
                new_x = 0.0;
            }
            if (new_y < 0.0) {
                new_y = 0.0;
            }
            if (new_x > (gdouble)(width - 1)) {
                new_x = (gdouble)(width - 1);
            }
            if (new_y > (gdouble)(height - 1)) {
                new_y = (gdouble)(height - 1);
            }

            mark->x1 = new_x;
            mark->y1 = new_y;
            mark->x2 = new_x;
            mark->y2 = new_y;
        }

        gtk_widget_queue_draw(state->shots_editor_canvas);
        return TRUE;
    }

    if (!state->shots_editor_dragging) {
        return FALSE;
    }

    if (!shots_editor_widget_to_image(state, event->x, event->y, &ix, &iy, TRUE)) {
        return FALSE;
    }

    state->shots_editor_drag_cur_x = ix;
    state->shots_editor_drag_cur_y = iy;
    gtk_widget_queue_draw(state->shots_editor_canvas);
    return TRUE;
}

static gboolean on_shots_editor_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    AppState *state = user_data;
    GdkRGBA color = {0.95, 0.24, 0.24, 1.0};
    gdouble dx = 0.0;
    gdouble dy = 0.0;

    (void)widget;
    if (!state || event->button != 1) {
        return FALSE;
    }

    if (state->shots_editor_edit_dragging) {
        ShotMark *mark = shots_editor_get_selected_mark(state);
        state->shots_editor_edit_dragging = FALSE;
        state->shots_editor_selected_arrow_handle = 0;
        if (mark && mark->type == SHOT_MARK_RECT) {
            shots_editor_set_status(state, "Callout updated.");
        } else if (mark && mark->type == SHOT_MARK_ARROW) {
            shots_editor_set_status(state, "Arrow updated.");
        } else {
            shots_editor_set_status(state, "Annotation updated.");
        }
        gtk_widget_queue_draw(state->shots_editor_canvas);
        return TRUE;
    }

    if (!state->shots_editor_dragging) {
        return FALSE;
    }

    state->shots_editor_dragging = FALSE;
    if (state->shots_editor_color_btn) {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), &color);
    }

    dx = state->shots_editor_drag_cur_x - state->shots_editor_drag_start_x;
    dy = state->shots_editor_drag_cur_y - state->shots_editor_drag_start_y;
    if (state->shots_editor_drag_mark_type == SHOT_MARK_ARROW && sqrt(dx * dx + dy * dy) >= 3.0) {
        ShotMark *mark = g_new0(ShotMark, 1);
        mark->type = SHOT_MARK_ARROW;
        mark->x1 = state->shots_editor_drag_start_x;
        mark->y1 = state->shots_editor_drag_start_y;
        mark->x2 = state->shots_editor_drag_cur_x;
        mark->y2 = state->shots_editor_drag_cur_y;
        mark->color = color;
        shots_editor_apply_arrow_style_to_mark(state, mark);
        shots_editor_ensure_marks(state);
        g_ptr_array_add(state->shots_editor_marks, mark);
        shots_editor_set_selected_mark(state, (gint)state->shots_editor_marks->len - 1);
        shots_editor_set_status(state, "Arrow added.");
    } else if (state->shots_editor_drag_mark_type == SHOT_MARK_RECT &&
               fabs(dx) >= 6.0 && fabs(dy) >= 6.0) {
        ShotMark *mark = g_new0(ShotMark, 1);
        gchar *label = NULL;

        mark->type = SHOT_MARK_RECT;
        mark->x1 = state->shots_editor_drag_start_x;
        mark->y1 = state->shots_editor_drag_start_y;
        mark->x2 = state->shots_editor_drag_cur_x;
        mark->y2 = state->shots_editor_drag_cur_y;
        mark->color = color;
        mark->stroke_width = MAX(2.0, shots_editor_get_arrow_width_control(state) * 0.5);

        if (state->shots_editor_text_entry) {
            label = g_strdup(gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));
            if (label) {
                g_strstrip(label);
            }
        }
        if (label && *label) {
            mark->text = label;
            label = NULL;
        } else {
            mark->text = g_strdup("Callout");
        }
        g_free(label);

        shots_editor_ensure_marks(state);
        g_ptr_array_add(state->shots_editor_marks, mark);
        shots_editor_set_selected_mark(state, (gint)state->shots_editor_marks->len - 1);
        shots_editor_set_status(state, "Callout box added.");
    }

    gtk_widget_queue_draw(state->shots_editor_canvas);
    return TRUE;
}

static gboolean shots_editor_load_image(AppState *state, const gchar *path) {
    GdkPixbuf *pixbuf = NULL;
    GError *error = NULL;
    gchar *basename = NULL;
    gchar *path_msg = NULL;

    if (!state || !path || !*path) {
        return FALSE;
    }

    pixbuf = gdk_pixbuf_new_from_file(path, &error);
    if (!pixbuf) {
        shots_editor_set_status(state,
                                error && error->message ? error->message : "Failed to load image.");
        g_clear_error(&error);
        return FALSE;
    }

    if (state->shots_editor_pixbuf) {
        g_object_unref(state->shots_editor_pixbuf);
    }
    state->shots_editor_pixbuf = pixbuf;
    g_free(state->shots_editor_image_path);
    state->shots_editor_image_path = g_strdup(path);
    state->shots_editor_dragging = FALSE;
    shots_editor_clear_marks(state);

    if (state->shots_editor_path_label) {
        basename = g_path_get_basename(path);
        path_msg = g_strdup_printf("Editing: %s", basename ? basename : path);
        gtk_label_set_text(GTK_LABEL(state->shots_editor_path_label), path_msg);
        g_free(path_msg);
        g_free(basename);
    }

    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
    shots_editor_set_status(state, "Image loaded. Use Select tool to edit arrow start/path/head.");
    return TRUE;
}

static gchar *shots_editor_make_output_path(AppState *state, const gchar *extension) {
    gchar *basename = NULL;
    gchar *stem = NULL;
    gchar *dot = NULL;
    GDateTime *now = NULL;
    gchar *stamp = NULL;
    gchar *filename = NULL;
    gchar *path = NULL;

    if (!state || !state->shots_editor_image_path) {
        return NULL;
    }

    basename = g_path_get_basename(state->shots_editor_image_path);
    dot = g_strrstr(basename, ".");
    stem = dot ? g_strndup(basename, (gsize)(dot - basename)) : g_strdup(basename);
    now = g_date_time_new_now_local();
    stamp = g_date_time_format(now, "%Y%m%d-%H%M%S");
    filename = g_strdup_printf("%s_annotated_%s.%s",
                               stem,
                               stamp,
                               (extension && *extension) ? extension : "png");
    path = g_build_filename(state->shots_dir ? state->shots_dir : ".", filename, NULL);

    g_free(filename);
    g_free(stamp);
    g_date_time_unref(now);
    g_free(stem);
    g_free(basename);
    return path;
}

static void on_shots_editor_tool_changed(GtkComboBox *combo, gpointer user_data) {
    AppState *state = user_data;
    const gchar *id = gtk_combo_box_get_active_id(combo);

    if (!state) {
        return;
    }
    if (id && g_strcmp0(id, "select") == 0) {
        shots_editor_set_status(state, "Select mode: drag annotations to move/edit. Right-click for quick color palette.");
    } else if (id && g_strcmp0(id, "rect") == 0) {
        shots_editor_set_status(state, "Callout mode: drag to create a shadowed box.");
    }
}

static void on_shots_editor_color_set(GtkColorButton *button, gpointer user_data) {
    AppState *state = user_data;
    GdkRGBA color = {0};

    if (!state || state->shots_editor_style_syncing) {
        return;
    }

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), &color);
    if (shots_editor_apply_color_to_selected(state, &color, FALSE)) {
        shots_editor_set_status(state, "Selected annotation color updated.");
    }
}

static void on_shots_editor_apply_text_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    gchar *text = NULL;

    (void)button;
    if (!state || !state->shots_editor_text_entry) {
        return;
    }
    if (!mark) {
        shots_editor_set_status(state, "Select a text/callout/stamp annotation first.");
        return;
    }
    if (!(mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_STAMP || mark->type == SHOT_MARK_RECT)) {
        shots_editor_set_status(state, "Text edit applies to Text, Stamp, or Callout annotations.");
        return;
    }

    text = g_strdup(gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));
    if (!text) {
        text = g_strdup("");
    } else {
        g_strstrip(text);
    }

    g_free(mark->text);
    if (*text == '\0') {
        if (mark->type == SHOT_MARK_STAMP) {
            mark->text = g_strdup("1");
        } else if (mark->type == SHOT_MARK_RECT) {
            mark->text = g_strdup("Callout");
        } else {
            mark->text = g_strdup("Note");
        }
    } else {
        mark->text = text;
        text = NULL;
    }
    g_free(text);

    gtk_widget_queue_draw(state->shots_editor_canvas);
    shots_editor_recompute_next_step(state);
    shots_editor_set_status(state, "Selected annotation text updated.");
}

static void on_shots_editor_color_preset_activate(GtkMenuItem *item, gpointer user_data) {
    AppState *state = user_data;
    const gchar *spec = g_object_get_data(G_OBJECT(item), "color_spec");
    GdkRGBA color = {0};

    if (!state || !spec || !gdk_rgba_parse(&color, spec)) {
        return;
    }
    if (shots_editor_apply_color_to_selected(state, &color, TRUE)) {
        shots_editor_set_status(state, "Selected annotation color updated from palette.");
    }
}

static void shots_editor_show_color_palette_menu(AppState *state, GdkEventButton *event) {
    GtkWidget *menu = NULL;
    struct {
        const gchar *label;
        const gchar *hex;
    } presets[] = {
        { "Red", "#ef4444" },
        { "Orange", "#f97316" },
        { "Yellow", "#eab308" },
        { "Green", "#22c55e" },
        { "Blue", "#3b82f6" },
        { "Purple", "#8b5cf6" },
        { "White", "#f8fafc" },
        { "Black", "#111827" }
    };

    if (!state || !event) {
        return;
    }

    if (!shots_editor_get_selected_mark(state)) {
        shots_editor_set_status(state, "Right-click palette needs a selected annotation.");
        return;
    }

    menu = gtk_menu_new();
    for (guint i = 0; i < G_N_ELEMENTS(presets); ++i) {
        GtkWidget *item = gtk_menu_item_new_with_label(presets[i].label);
        g_object_set_data_full(G_OBJECT(item), "color_spec", g_strdup(presets[i].hex), g_free);
        g_signal_connect(item, "activate", G_CALLBACK(on_shots_editor_color_preset_activate), state);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    }
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
}

static void on_shots_editor_arrow_style_changed(GtkRange *range, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    (void)range;

    if (!state || state->shots_editor_style_syncing) {
        return;
    }
    if (!mark || (mark->type != SHOT_MARK_ARROW && mark->type != SHOT_MARK_RECT)) {
        return;
    }

    if (mark->type == SHOT_MARK_ARROW) {
        shots_editor_apply_arrow_style_to_mark(state, mark);
    } else {
        mark->stroke_width = MAX(2.0, shots_editor_get_arrow_width_control(state) * 0.5);
    }
    gtk_widget_queue_draw(state->shots_editor_canvas);
    shots_editor_set_status(state, "Selected annotation style updated.");
}

static void on_shots_editor_step_reset_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;
    if (!state) {
        return;
    }
    state->shots_editor_next_step = 1;
    shots_editor_update_step_label(state);
    shots_editor_set_status(state, "Auto step counter reset to 1.");
}

static void shots_editor_apply_dock_layout(AppState *state) {
    GtkWidget *child1 = NULL;
    GtkWidget *child2 = NULL;
    GtkWidget *paned = NULL;
    GtkWidget *canvas_panel = NULL;
    GtkWidget *sidebar_scroller = NULL;

    if (!state ||
        !state->shots_editor_main_paned ||
        !state->shots_editor_canvas_panel ||
        !state->shots_editor_sidebar_scroller) {
        return;
    }

    paned = state->shots_editor_main_paned;
    canvas_panel = state->shots_editor_canvas_panel;
    sidebar_scroller = state->shots_editor_sidebar_scroller;

    g_object_ref(canvas_panel);
    g_object_ref(sidebar_scroller);

    child1 = gtk_paned_get_child1(GTK_PANED(paned));
    if (child1) {
        gtk_container_remove(GTK_CONTAINER(paned), child1);
    }
    child2 = gtk_paned_get_child2(GTK_PANED(paned));
    if (child2) {
        gtk_container_remove(GTK_CONTAINER(paned), child2);
    }

    if (state->shots_editor_tools_docked_right) {
        gtk_paned_pack1(GTK_PANED(paned), canvas_panel, TRUE, FALSE);
        gtk_paned_pack2(GTK_PANED(paned), sidebar_scroller, FALSE, FALSE);
        gtk_paned_set_position(GTK_PANED(paned), 820);
        if (state->shots_editor_dock_toggle_btn) {
            gtk_button_set_label(GTK_BUTTON(state->shots_editor_dock_toggle_btn), "Dock Left");
        }
    } else {
        gtk_paned_pack1(GTK_PANED(paned), sidebar_scroller, FALSE, FALSE);
        gtk_paned_pack2(GTK_PANED(paned), canvas_panel, TRUE, FALSE);
        gtk_paned_set_position(GTK_PANED(paned), 280);
        if (state->shots_editor_dock_toggle_btn) {
            gtk_button_set_label(GTK_BUTTON(state->shots_editor_dock_toggle_btn), "Dock Right");
        }
    }

    g_object_unref(canvas_panel);
    g_object_unref(sidebar_scroller);
    gtk_widget_show_all(paned);
}

static void on_shots_editor_toggle_dock_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!state) {
        return;
    }

    state->shots_editor_tools_docked_right = !state->shots_editor_tools_docked_right;
    shots_editor_apply_dock_layout(state);
    shots_editor_set_status(state,
                            state->shots_editor_tools_docked_right
                                ? "Tools docked on the right."
                                : "Tools docked on the left.");
}

static void on_shots_editor_capture_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!command_is_available("flameshot")) {
        shots_editor_set_status(state, "Flameshot not found. Install it to capture from this tab.");
        return;
    }

    launch_in_background("flameshot gui");
    shots_editor_set_status(state, "Flameshot opened. Save the screenshot, then click Refresh.");
}

static void on_shots_editor_undo_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    shots_editor_ensure_marks(state);
    if (state->shots_editor_marks->len == 0) {
        shots_editor_set_status(state, "No annotations to undo.");
        return;
    }

    g_ptr_array_remove_index(state->shots_editor_marks, state->shots_editor_marks->len - 1);
    shots_editor_set_selected_mark(state, -1);
    shots_editor_recompute_next_step(state);
    gtk_widget_queue_draw(state->shots_editor_canvas);
    shots_editor_set_status(state, "Last annotation removed.");
}

static void on_shots_editor_clear_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    shots_editor_ensure_marks(state);
    if (state->shots_editor_marks->len == 0) {
        shots_editor_set_status(state, "Canvas already clean.");
        return;
    }
    shots_editor_clear_marks(state);
    shots_editor_set_status(state, "All annotations cleared.");
}

static void on_shots_editor_save_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    GdkPixbuf *rendered = NULL;
    gchar *output_path = NULL;
    GError *error = NULL;
    gint width = 0;
    gint height = 0;
    gchar *basename = NULL;
    gchar *msg = NULL;

    (void)button;
    if (!state || !state->shots_editor_pixbuf) {
        shots_editor_set_status(state, "Select an image first.");
        return;
    }

    width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
    height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, state->shots_editor_pixbuf, 0, 0);
    cairo_paint(cr);
    shots_editor_draw_marks(state, cr, FALSE);
    cairo_destroy(cr);
    cr = NULL;

    rendered = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
    cairo_surface_destroy(surface);
    surface = NULL;
    if (!rendered) {
        shots_editor_set_status(state, "Failed to render annotated image.");
        return;
    }

    output_path = shots_editor_make_output_path(state, "png");
    if (!output_path) {
        g_object_unref(rendered);
        shots_editor_set_status(state, "Unable to build output path.");
        return;
    }

    if (!gdk_pixbuf_save(rendered, output_path, "png", &error, NULL)) {
        msg = g_strdup_printf("Save failed: %s", error ? error->message : "unknown error");
        shots_editor_set_status(state, msg);
        g_clear_error(&error);
        g_free(msg);
        g_free(output_path);
        g_object_unref(rendered);
        return;
    }

    basename = g_path_get_basename(output_path);
    msg = g_strdup_printf("Saved annotated copy: %s", basename ? basename : output_path);
    shots_editor_set_status(state, msg);
    g_free(msg);

    shots_reload(state);
    shots_editor_load_image(state, output_path);

    g_free(basename);
    g_free(output_path);
    g_object_unref(rendered);
}

static void on_shots_editor_save_svg_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    cairo_surface_t *surface = NULL;
    cairo_t *cr = NULL;
    cairo_status_t cstatus = CAIRO_STATUS_SUCCESS;
    gchar *output_path = NULL;
    gchar *basename = NULL;
    gchar *msg = NULL;
    gint width = 0;
    gint height = 0;

    (void)button;
    if (!state || !state->shots_editor_pixbuf) {
        shots_editor_set_status(state, "Select an image first.");
        return;
    }

    output_path = shots_editor_make_output_path(state, "svg");
    if (!output_path) {
        shots_editor_set_status(state, "Unable to build SVG output path.");
        return;
    }

    width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
    height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
    surface = cairo_svg_surface_create(output_path, width, height);
    cstatus = cairo_surface_status(surface);
    if (cstatus != CAIRO_STATUS_SUCCESS) {
        msg = g_strdup_printf("SVG surface error: %s", cairo_status_to_string(cstatus));
        shots_editor_set_status(state, msg);
        g_free(msg);
        cairo_surface_destroy(surface);
        g_free(output_path);
        return;
    }

    cr = cairo_create(surface);
    cstatus = cairo_status(cr);
    if (cstatus != CAIRO_STATUS_SUCCESS) {
        msg = g_strdup_printf("SVG context error: %s", cairo_status_to_string(cstatus));
        shots_editor_set_status(state, msg);
        g_free(msg);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        g_free(output_path);
        return;
    }

    gdk_cairo_set_source_pixbuf(cr, state->shots_editor_pixbuf, 0, 0);
    cairo_paint(cr);
    shots_editor_draw_marks(state, cr, FALSE);
    cairo_destroy(cr);
    cairo_surface_flush(surface);
    cairo_surface_finish(surface);
    cstatus = cairo_surface_status(surface);
    cairo_surface_destroy(surface);

    if (cstatus != CAIRO_STATUS_SUCCESS) {
        msg = g_strdup_printf("SVG save failed: %s", cairo_status_to_string(cstatus));
        shots_editor_set_status(state, msg);
        g_free(msg);
        g_free(output_path);
        return;
    }

    basename = g_path_get_basename(output_path);
    msg = g_strdup_printf("Saved SVG copy: %s", basename ? basename : output_path);
    shots_editor_set_status(state, msg);
    g_free(msg);
    g_free(basename);
    g_free(output_path);
    shots_reload(state);
}

static GPtrArray *shots_get_selected_paths(AppState *state) {
    GPtrArray *paths = g_ptr_array_new_with_free_func(g_free);
    GList *selected = gtk_icon_view_get_selected_items(GTK_ICON_VIEW(state->shots_icon_view));

    for (GList *it = selected; it; it = it->next) {
        GtkTreePath *path = it->data;
        GtkTreeIter iter;
        gchar *fullpath = NULL;

        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(state->shots_store), &iter, path)) {
            gtk_tree_model_get(GTK_TREE_MODEL(state->shots_store), &iter, COL_PATH, &fullpath, -1);
            if (fullpath) {
                g_ptr_array_add(paths, fullpath);
            }
        }
    }

    g_list_free_full(selected, (GDestroyNotify)gtk_tree_path_free);
    return paths;
}

static gchar *shots_get_single_selected_path(AppState *state) {
    GPtrArray *paths = shots_get_selected_paths(state);
    gchar *result = NULL;

    if (paths->len == 1) {
        result = g_strdup(g_ptr_array_index(paths, 0));
    }

    g_ptr_array_free(paths, TRUE);
    return result;
}

static void shots_editor_load_from_selection(AppState *state, gboolean quiet) {
    gchar *path = shots_get_single_selected_path(state);

    if (path) {
        shots_editor_load_image(state, path);
        g_free(path);
        return;
    }

    if (!quiet) {
        shots_editor_set_status(state, "Select exactly one image to edit.");
    }
}

static void on_shots_editor_edit_selected_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    shots_editor_load_from_selection(user_data, FALSE);
}

static void shots_update_selection_ui(AppState *state) {
    GPtrArray *paths = shots_get_selected_paths(state);
    GString *joined = g_string_new(NULL);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(state->shots_paths_view));
    gchar *selection_text = NULL;

    if (paths->len == 0) {
        gtk_label_set_text(GTK_LABEL(state->shots_selected), "No images selected.");
    } else {
        selection_text = g_strdup_printf("%u image(s) selected.", paths->len);
        gtk_label_set_text(GTK_LABEL(state->shots_selected), selection_text);
        g_free(selection_text);
    }

    for (guint i = 0; i < paths->len; ++i) {
        const gchar *path = g_ptr_array_index(paths, i);
        g_string_append(joined, path);
        if (i + 1 < paths->len) {
            g_string_append_c(joined, '\n');
        }
    }

    gtk_text_buffer_set_text(buffer, joined->str, -1);

    g_string_free(joined, TRUE);
    g_ptr_array_free(paths, TRUE);
}

static void shots_reload(AppState *state) {
    GDir *dir = NULL;
    const gchar *name = NULL;
    GPtrArray *entries = g_ptr_array_new_with_free_func(free_image_entry);

    if (state->shots_folder_label) {
        gchar *folder_msg = g_strdup_printf("Launch folder: %s\nActive folder: %s",
                                            state->launch_dir ? state->launch_dir : "unknown",
                                            state->shots_dir);
        gtk_label_set_text(GTK_LABEL(state->shots_folder_label), folder_msg);
        g_free(folder_msg);
    }

    gtk_list_store_clear(state->shots_store);
    g_mkdir_with_parents(state->shots_dir, 0755);

    dir = g_dir_open(state->shots_dir, 0, NULL);
    if (!dir) {
        gchar *msg = g_strdup_printf("Could not open screenshot folder: %s", state->shots_dir);
        gtk_label_set_text(GTK_LABEL(state->shots_status), msg);
        g_free(msg);
        shots_update_selection_ui(state);
        g_ptr_array_free(entries, TRUE);
        return;
    }

    while ((name = g_dir_read_name(dir)) != NULL) {
        gchar *path = NULL;
        GStatBuf st = {0};
        ImageEntry *entry = NULL;

        if (!ends_with_image_ext(name)) {
            continue;
        }

        path = g_build_filename(state->shots_dir, name, NULL);
        if (g_stat(path, &st) != 0) {
            g_free(path);
            continue;
        }

        entry = g_new0(ImageEntry, 1);
        entry->name = g_strdup(name);
        entry->path = path;
        entry->mtime = (gint64)st.st_mtime;
        g_ptr_array_add(entries, entry);
    }

    g_dir_close(dir);
    g_ptr_array_sort(entries, compare_entries_desc);

    for (guint i = 0; i < entries->len; ++i) {
        ImageEntry *entry = g_ptr_array_index(entries, i);
        GtkTreeIter iter;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(entry->path, 220, 140, TRUE, NULL);

        if (!pixbuf) {
            GtkIconTheme *theme = gtk_icon_theme_get_default();
            pixbuf = gtk_icon_theme_load_icon(theme, "image-x-generic", 96, 0, NULL);
        }

        gtk_list_store_append(state->shots_store, &iter);
        gtk_list_store_set(state->shots_store, &iter,
                           COL_PIXBUF, pixbuf,
                           COL_NAME, entry->name,
                           COL_PATH, entry->path,
                           -1);

        if (pixbuf) {
            g_object_unref(pixbuf);
        }
    }

    if (entries->len == 0) {
        gchar *msg = g_strdup_printf("No images found in: %s", state->shots_dir);
        gtk_label_set_text(GTK_LABEL(state->shots_status), msg);
        g_free(msg);
    } else {
        gchar *msg = g_strdup_printf("%u image(s) loaded from %s", entries->len, state->shots_dir);
        gtk_label_set_text(GTK_LABEL(state->shots_status), msg);
        g_free(msg);
    }

    g_ptr_array_free(entries, TRUE);
    shots_update_selection_ui(state);
}

static void on_shots_selection_changed(GtkIconView *icon_view, gpointer user_data) {
    AppState *state = user_data;
    (void)icon_view;
    shots_update_selection_ui(state);
    shots_editor_load_from_selection(state, TRUE);
}

static void on_shots_item_activated(GtkIconView *icon_view, GtkTreePath *path, gpointer user_data) {
    (void)icon_view;
    AppState *state = user_data;
    GtkTreeIter iter;
    gchar *fullpath = NULL;

    if (gtk_tree_model_get_iter(GTK_TREE_MODEL(state->shots_store), &iter, path)) {
        gtk_tree_model_get(GTK_TREE_MODEL(state->shots_store), &iter, COL_PATH, &fullpath, -1);
    }

    if (fullpath) {
        shots_editor_load_image(state, fullpath);
        shots_editor_set_status(state, "Image opened in quick editor.");
    }
    g_free(fullpath);
}

static void shots_set_browser_info_visible(AppState *state, gboolean visible) {
    if (!state) {
        return;
    }
    if (state->shots_browser_info_revealer) {
        gtk_revealer_set_reveal_child(GTK_REVEALER(state->shots_browser_info_revealer), visible);
    }
    if (state->shots_browser_split_paned) {
        if (visible) {
            gtk_paned_set_position(GTK_PANED(state->shots_browser_split_paned), 130);
        } else {
            gtk_paned_set_position(GTK_PANED(state->shots_browser_split_paned), 0);
        }
    }
    if (state->shots_browser_toggle_btn) {
        gtk_button_set_label(GTK_BUTTON(state->shots_browser_toggle_btn),
                             visible ? "Hide Selection Shell" : "Show Selection Shell");
    }
}

static void on_shots_toggle_shell_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    gboolean visible = FALSE;

    (void)button;
    if (!state || !state->shots_browser_info_revealer) {
        return;
    }

    visible = gtk_revealer_get_reveal_child(GTK_REVEALER(state->shots_browser_info_revealer));
    shots_set_browser_info_visible(state, !visible);
}

static void on_shots_refresh_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    shots_reload(user_data);
}

static void on_shots_select_all_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;
    gtk_icon_view_select_all(GTK_ICON_VIEW(state->shots_icon_view));
    shots_update_selection_ui(state);
}

static void on_shots_clear_selection_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;
    gtk_icon_view_unselect_all(GTK_ICON_VIEW(state->shots_icon_view));
    shots_update_selection_ui(state);
}

static void on_shots_open_folder_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    gchar *quoted = g_shell_quote(state->shots_dir);
    gchar *cmd = g_strdup_printf("xdg-open %s", quoted);
    launch_in_background(cmd);
    g_free(cmd);
    g_free(quoted);
}

static void on_shots_import_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    gchar *quoted_script = g_shell_quote(state->import_script);
    gchar *inner = g_strdup_printf("%s; echo; read -n1 -rsp \"Press any key to close...\"", quoted_script);
    gchar *quoted_inner = g_shell_quote(inner);
    gchar *cmd = g_strdup_printf("x-terminal-emulator -e /bin/bash -lc %s", quoted_inner);
    launch_in_background(cmd);
    gtk_label_set_text(GTK_LABEL(state->shots_status),
                       "Import script launched in terminal. Refresh after it completes.");
    g_free(cmd);
    g_free(quoted_inner);
    g_free(inner);
    g_free(quoted_script);
}

static void on_shots_copy_paths_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    GPtrArray *paths = shots_get_selected_paths(state);
    GString *text = g_string_new(NULL);

    for (guint i = 0; i < paths->len; ++i) {
        g_string_append(text, g_ptr_array_index(paths, i));
        if (i + 1 < paths->len) {
            g_string_append_c(text, '\n');
        }
    }

    if (text->len > 0) {
        set_clipboard_text(state->window, text->str);
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Copied full path(s) to clipboard and primary selection.");
    } else {
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Select one or more images first.");
    }

    g_string_free(text, TRUE);
    g_ptr_array_free(paths, TRUE);
}

static void on_shots_copy_prompt_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    AppState *state = user_data;
    GPtrArray *paths = shots_get_selected_paths(state);
    GString *text = g_string_new("Please analyze these files:");

    for (guint i = 0; i < paths->len; ++i) {
        const gchar *path = g_ptr_array_index(paths, i);
        g_string_append_printf(text, "\n @%s", path);
    }

    if (paths->len > 0) {
        set_clipboard_text(state->window, text->str);
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Copied AI prompt to clipboard and primary selection.");
    } else {
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Select image(s) to build a prompt.");
    }

    g_string_free(text, TRUE);
    g_ptr_array_free(paths, TRUE);
}

static void on_utility_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    const gchar *label = g_object_get_data(G_OBJECT(button), "utility_label");
    const gchar *command = g_object_get_data(G_OBJECT(button), "utility_command");

    if (!state || !command || !label) {
        return;
    }

    if (!command_is_available(command)) {
        gchar *msg = g_strdup_printf("%s is not installed or not in PATH.", label);
        set_label_text_trimmed(state->utils_status, msg, "Utility unavailable.");
        g_free(msg);
        return;
    }

    launch_in_background(command);
    {
        gchar *msg = g_strdup_printf("Launched %s", label);
        set_label_text_trimmed(state->utils_status, msg, "Launched utility.");
        g_free(msg);
    }
}

static GtkWidget *create_utility_button(const gchar *title, const gchar *subtitle, const gchar *command, AppState *state) {
    GtkWidget *button = NULL;
    GtkWidget *label = NULL;
    gchar *text = g_strdup_printf("%s\n%s", title, subtitle);

    button = gtk_button_new_with_label(text);
    g_free(text);

    label = gtk_bin_get_child(GTK_BIN(button));
    if (label && GTK_IS_LABEL(label)) {
        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD_CHAR);
    }

    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_halign(button, GTK_ALIGN_FILL);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "utility-btn");

    g_object_set_data_full(G_OBJECT(button), "utility_label", g_strdup(title), g_free);
    g_object_set_data_full(G_OBJECT(button), "utility_command", g_strdup(command), g_free);
    g_signal_connect(button, "clicked", G_CALLBACK(on_utility_clicked), state);
    return button;
}

static GtkWidget *build_night_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *subtitle = gtk_label_new(NULL);
    GtkWidget *meta = gtk_label_new(NULL);
    GtkWidget *temp_label = gtk_label_new(NULL);
    GtkWidget *status = gtk_label_new(NULL);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1000, 6500, 50);
    GtkWidget *toggle_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *toggle_label = gtk_label_new("Auto Night Light");
    GtkWidget *auto_switch = gtk_switch_new();
    GtkWidget *row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_apply = gtk_button_new_with_label("Apply Manual");
    GtkWidget *btn_off = gtk_button_new_with_label("Turn Off");
    GtkWidget *btn_warm = gtk_button_new_with_label("Warmer");
    GtkWidget *btn_cool = gtk_button_new_with_label("Cooler");
    GtkWidget *btn_status = gtk_button_new_with_label("Refresh");

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");

    gtk_label_set_text(GTK_LABEL(title), "Night Light Control");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_text(GTK_LABEL(subtitle), "Manual redshift controls with one-click presets.");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "hero-sub");

    gtk_label_set_text(GTK_LABEL(meta), "Loading local time and solar state...");
    gtk_label_set_xalign(GTK_LABEL(meta), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(meta), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(meta), "meta-info");

    set_label_markupf(temp_label, "Target Temperature: <b>4200K</b>");
    gtk_label_set_xalign(GTK_LABEL(temp_label), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(temp_label), "hero-sub");

    gtk_label_set_text(GTK_LABEL(status), "Loading redshift status...");
    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(status), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");

    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_container_set_border_width(GTK_CONTAINER(card), 14);
    gtk_style_context_add_class(gtk_widget_get_style_context(toggle_row), "toggle-row");
    gtk_label_set_xalign(GTK_LABEL(toggle_label), 0.0);
    gtk_widget_set_hexpand(toggle_label, TRUE);
    gtk_widget_set_halign(toggle_label, GTK_ALIGN_START);
    gtk_widget_set_halign(auto_switch, GTK_ALIGN_END);

    gtk_style_context_add_class(gtk_widget_get_style_context(row1), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(row2), "action-row");
    style_action_button(btn_apply, TRUE);
    style_action_button(btn_off, FALSE);
    style_action_button(btn_warm, FALSE);
    style_action_button(btn_cool, FALSE);
    style_action_button(btn_status, FALSE);

    gtk_range_set_value(GTK_RANGE(slider), 4200);
    gtk_scale_set_draw_value(GTK_SCALE(slider), FALSE);

    state->night_temp_label = temp_label;
    state->night_status = status;
    state->night_meta_label = meta;
    state->night_scale = slider;
    state->night_auto_switch = auto_switch;
    night_update_slider_label(state);

    gtk_box_pack_start(GTK_BOX(toggle_row), toggle_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(toggle_row), auto_switch, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(row1), btn_apply, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_warm, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_cool, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(row2), btn_off, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row2), btn_status, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(card), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), toggle_row, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), temp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), slider, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), row1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), row2, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), status, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), meta, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), card, FALSE, FALSE, 0);

    g_signal_connect(slider, "value-changed", G_CALLBACK(on_night_scale_changed), state);
    g_signal_connect(btn_apply, "clicked", G_CALLBACK(on_night_apply_clicked), state);
    g_signal_connect(btn_off, "clicked", G_CALLBACK(on_night_off_clicked), state);
    g_signal_connect(btn_warm, "clicked", G_CALLBACK(on_night_warm_clicked), state);
    g_signal_connect(btn_cool, "clicked", G_CALLBACK(on_night_cool_clicked), state);
    g_signal_connect(btn_status, "clicked", G_CALLBACK(on_night_status_clicked), state);
    g_signal_connect(auto_switch, "notify::active", G_CALLBACK(on_night_auto_switch_notify), state);

    night_refresh_status(state);
    return root;
}

static GtkWidget *build_audio_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *subtitle = gtk_label_new(NULL);
    GtkWidget *volume_label = gtk_label_new(NULL);
    GtkWidget *status = gtk_label_new(NULL);
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 150, 1);
    GtkWidget *row1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *row2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_apply = gtk_button_new_with_label("Apply Volume");
    GtkWidget *btn_down = gtk_button_new_with_label("-5%");
    GtkWidget *btn_up = gtk_button_new_with_label("+5%");
    GtkWidget *btn_mute = gtk_button_new_with_label("Mute / Unmute");
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_pavucontrol = gtk_button_new_with_label("Open Pavucontrol");

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");

    gtk_label_set_text(GTK_LABEL(title), "Audio Control");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_text(GTK_LABEL(subtitle), "Quick system audio controls with direct access to Pavucontrol.");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "hero-sub");
    set_label_markupf(volume_label, "Target Volume: <b>50%%</b>");
    gtk_label_set_xalign(GTK_LABEL(volume_label), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(volume_label), "hero-sub");

    gtk_label_set_text(GTK_LABEL(status), "Loading audio status...");
    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(status), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");

    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_container_set_border_width(GTK_CONTAINER(card), 14);
    gtk_style_context_add_class(gtk_widget_get_style_context(row1), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(row2), "action-row");
    style_action_button(btn_apply, TRUE);
    style_action_button(btn_down, FALSE);
    style_action_button(btn_up, FALSE);
    style_action_button(btn_mute, FALSE);
    style_action_button(btn_refresh, FALSE);
    style_action_button(btn_pavucontrol, FALSE);

    gtk_range_set_value(GTK_RANGE(slider), 50);
    gtk_scale_set_draw_value(GTK_SCALE(slider), FALSE);

    state->audio_status = status;
    state->audio_volume_label = volume_label;
    state->audio_scale = slider;
    audio_update_slider_label(state);

    gtk_box_pack_start(GTK_BOX(row1), btn_apply, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_down, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row1), btn_up, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(row2), btn_mute, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row2), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row2), btn_pavucontrol, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(card), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), volume_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), slider, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), row1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(card), row2, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), status, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), card, FALSE, FALSE, 0);

    g_signal_connect(slider, "value-changed", G_CALLBACK(on_audio_scale_changed), state);
    g_signal_connect(btn_apply, "clicked", G_CALLBACK(on_audio_apply_clicked), state);
    g_signal_connect(btn_down, "clicked", G_CALLBACK(on_audio_down_clicked), state);
    g_signal_connect(btn_up, "clicked", G_CALLBACK(on_audio_up_clicked), state);
    g_signal_connect(btn_mute, "clicked", G_CALLBACK(on_audio_mute_clicked), state);
    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(on_audio_refresh_clicked), state);
    g_signal_connect(btn_pavucontrol, "clicked", G_CALLBACK(on_audio_open_pavucontrol_clicked), state);

    audio_refresh_status(state);
    return root;
}

static GtkWidget *build_utilities_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *subtitle = gtk_label_new(NULL);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *status = gtk_label_new("Click a utility to launch it.");
    GtkWidget *b1 = create_utility_button("Pavucontrol", "Audio mixer and device routing", "pavucontrol", state);
    GtkWidget *b2 = create_utility_button("CC Switch", "Project/context switch helper", "cc-switch", state);
    GtkWidget *b3 = create_utility_button("Flameshot", "Capture area screenshot", "flameshot gui", state);
    GtkWidget *b4 = create_utility_button("Network", "Connection editor and details", "nm-connection-editor", state);
    GtkWidget *b5 = create_utility_button("Bluetooth", "Devices and pairing", "blueman-manager", state);
    GtkWidget *b6 = create_utility_button("Workspace", "Open LinuxUtilities folder", "xdg-open .", state);
    GtkWidget *b7 = create_utility_button("Screenshots", "Open Screenshots folder", "xdg-open Screenshots", state);
    GtkWidget *b8 = create_utility_button("Terminator", "Open terminal workspace", "terminator", state);

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");

    gtk_label_set_text(GTK_LABEL(title), "Utilities");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_text(GTK_LABEL(subtitle), "Fast launch pad for your daily Linux tools.");
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "hero-sub");

    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 12);
    gtk_grid_attach(GTK_GRID(grid), b1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b4, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b5, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b6, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b7, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b8, 1, 3, 1, 1);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, TRUE);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL);

    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(status), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), grid, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(root), status, FALSE, FALSE, 0);

    state->utils_status = status;
    return root;
}

static GtkWidget *build_screenshots_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *folder = gtk_label_new(NULL);
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_select_all = gtk_button_new_with_label("Select All");
    GtkWidget *btn_clear_selection = gtk_button_new_with_label("Clear Selection");
    GtkWidget *btn_toggle_shell = gtk_button_new_with_label("Show Selection Shell");
    GtkWidget *btn_import = gtk_button_new_with_label("Run Import Script");
    GtkWidget *btn_open = gtk_button_new_with_label("Open Folder");
    GtkWidget *workspace_split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *editor_shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *editor_header = gtk_label_new(NULL);
    GtkWidget *editor_main_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *editor_canvas_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *editor_sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *editor_sidebar_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *editor_canvas = gtk_drawing_area_new();
    GtkWidget *editor_path = gtk_label_new("Editing: (none)");
    GtkWidget *editor_status = gtk_label_new("Select one screenshot or click Edit Selected.");
    GtkWidget *browser_shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *browser_split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *browser_info_revealer = gtk_revealer_new();
    GtkWidget *browser_info_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *browser_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *thumbs_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *icon_view = NULL;
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_copy_paths = gtk_button_new_with_label("Copy Path(s)");
    GtkWidget *btn_copy_prompt = gtk_button_new_with_label("Copy Prompt");
    GtkWidget *paths_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *paths_view = gtk_text_view_new();
    GtkWidget *status = gtk_label_new("Loading screenshots...");
    GtkWidget *selected = gtk_label_new("No images selected.");
    GtkWidget *btn_capture = gtk_button_new_with_label("Capture");
    GtkWidget *btn_edit_selected = gtk_button_new_with_label("Edit Selected");
    GtkWidget *btn_undo = gtk_button_new_with_label("Undo");
    GtkWidget *btn_clear = gtk_button_new_with_label("Clear");
    GtkWidget *btn_save = gtk_button_new_with_label("Save Annotated Copy");
    GtkWidget *btn_save_svg = gtk_button_new_with_label("Save SVG Copy");
    GtkWidget *btn_dock_toggle = gtk_button_new_with_label("Dock Left");
    GtkWidget *tool_label = gtk_label_new("Tool");
    GtkWidget *tool_combo = gtk_combo_box_text_new();
    GtkWidget *stamp_label = gtk_label_new("Stamp");
    GtkWidget *stamp_combo = gtk_combo_box_text_new();
    GtkWidget *auto_step_check = gtk_check_button_new_with_label("Auto step numbering");
    GtkWidget *step_label = gtk_label_new("Next step: 1");
    GtkWidget *btn_step_reset = gtk_button_new_with_label("Reset Step");
    GtkWidget *text_label = gtk_label_new("Text");
    GtkWidget *text_entry = gtk_entry_new();
    GtkWidget *btn_apply_text = gtk_button_new_with_label("Apply Text");
    GtkWidget *color_label = gtk_label_new("Color");
    GtkWidget *color_btn = gtk_color_button_new();
    GtkWidget *arrow_width_label = gtk_label_new("Arrow width");
    GtkWidget *arrow_width_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 2, 16, 1);
    GtkWidget *arrow_head_len_label = gtk_label_new("Arrow head size");
    GtkWidget *arrow_head_len_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 8, 60, 1);
    GtkWidget *arrow_head_angle_label = gtk_label_new("Arrow head angle");
    GtkWidget *arrow_head_angle_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 10, 60, 1);
    GdkRGBA default_color = {0.95, 0.24, 0.24, 1.0};

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_shell), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_info), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_shell), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_canvas_panel), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_sidebar), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(actions), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_main_paned), "split-pane");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_split), "split-pane");
    gtk_style_context_add_class(gtk_widget_get_style_context(workspace_split), "split-pane");

    gtk_label_set_text(GTK_LABEL(title), "Screenshot Browser + Quick Annotator");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_xalign(GTK_LABEL(folder), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(folder), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(folder), "meta-info");

    style_action_button(btn_refresh, FALSE);
    style_action_button(btn_select_all, FALSE);
    style_action_button(btn_clear_selection, FALSE);
    style_action_button(btn_toggle_shell, FALSE);
    style_action_button(btn_import, FALSE);
    style_action_button(btn_open, FALSE);
    style_action_button(btn_copy_paths, FALSE);
    style_action_button(btn_copy_prompt, TRUE);
    style_action_button(btn_capture, FALSE);
    style_action_button(btn_edit_selected, TRUE);
    style_action_button(btn_undo, FALSE);
    style_action_button(btn_clear, FALSE);
    style_action_button(btn_save, TRUE);
    style_action_button(btn_save_svg, FALSE);
    style_action_button(btn_dock_toggle, FALSE);
    style_action_button(btn_step_reset, FALSE);
    style_action_button(btn_apply_text, FALSE);

    gtk_box_pack_start(GTK_BOX(toolbar), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_select_all, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_clear_selection, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_toggle_shell, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_import, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_open, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions), btn_copy_paths, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_copy_prompt, FALSE, FALSE, 0);

    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_xalign(GTK_LABEL(selected), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_status), "status-pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(step_label), "meta-info");

    gtk_text_view_set_editable(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(paths_view), GTK_WRAP_CHAR);
    gtk_container_add(GTK_CONTAINER(paths_scroller), paths_view);
    gtk_widget_set_size_request(paths_scroller, -1, 90);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(paths_scroller),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_style_context_add_class(gtk_widget_get_style_context(paths_scroller), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(paths_view), "surface");

    state->shots_store = gtk_list_store_new(N_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING);
    icon_view = gtk_icon_view_new_with_model(GTK_TREE_MODEL(state->shots_store));
    gtk_icon_view_set_pixbuf_column(GTK_ICON_VIEW(icon_view), COL_PIXBUF);
    gtk_icon_view_set_text_column(GTK_ICON_VIEW(icon_view), COL_NAME);
    gtk_icon_view_set_item_width(GTK_ICON_VIEW(icon_view), 180);
    gtk_icon_view_set_margin(GTK_ICON_VIEW(icon_view), 8);
    gtk_icon_view_set_spacing(GTK_ICON_VIEW(icon_view), 8);
    gtk_icon_view_set_columns(GTK_ICON_VIEW(icon_view), 8);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_MULTIPLE);
    gtk_container_add(GTK_CONTAINER(thumbs_scroller), icon_view);
    gtk_widget_set_size_request(thumbs_scroller, -1, 180);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(thumbs_scroller),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_style_context_add_class(gtk_widget_get_style_context(thumbs_scroller), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(icon_view), "surface");

    gtk_label_set_text(GTK_LABEL(editor_header), "Quick Editor Workspace");
    gtk_label_set_xalign(GTK_LABEL(editor_header), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_header), "hero-sub");
    gtk_label_set_xalign(GTK_LABEL(editor_path), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_path), "meta-info");
    gtk_label_set_xalign(GTK_LABEL(editor_status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(editor_status), TRUE);

    gtk_container_set_border_width(GTK_CONTAINER(editor_shell), 10);
    gtk_container_set_border_width(GTK_CONTAINER(editor_canvas_panel), 10);
    gtk_container_set_border_width(GTK_CONTAINER(editor_sidebar), 8);

    gtk_label_set_xalign(GTK_LABEL(tool_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(stamp_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(step_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(text_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(color_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_width_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_head_len_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_head_angle_label), 0.0);

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "select", "Select/Edit");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "arrow", "Arrow");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "rect", "Callout Box");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "stamp", "Step/Stamp");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "text", "Text");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(tool_combo), "arrow");

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "1", "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "2", "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "3", "3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "!", "!");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "?", "?");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "OK", "OK");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(stamp_combo), "1");

    gtk_entry_set_placeholder_text(GTK_ENTRY(text_entry), "Enter label text...");
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_btn), &default_color);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_step_check), TRUE);
    gtk_range_set_value(GTK_RANGE(arrow_width_scale), 6.0);
    gtk_range_set_value(GTK_RANGE(arrow_head_len_scale), 20.0);
    gtk_range_set_value(GTK_RANGE(arrow_head_angle_scale), 25.0);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_width_scale), FALSE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_head_len_scale), FALSE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_head_angle_scale), FALSE);

    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_capture, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_edit_selected, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_undo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_clear, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_save_svg, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_dock_toggle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), tool_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), tool_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), stamp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), stamp_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), auto_step_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), step_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_step_reset, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), text_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), text_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_apply_text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), color_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), color_btn, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_width_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_width_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_head_len_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_head_len_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_head_angle_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), arrow_head_angle_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), gtk_label_new(""), TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(editor_sidebar_scroller), editor_sidebar);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor_sidebar_scroller),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(editor_sidebar_scroller, 280, -1);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_sidebar_scroller), "surface");

    gtk_widget_set_hexpand(editor_canvas, TRUE);
    gtk_widget_set_vexpand(editor_canvas, TRUE);
    gtk_widget_set_size_request(editor_canvas, -1, 260);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_canvas), "surface");
    gtk_widget_add_events(editor_canvas,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK |
                          GDK_POINTER_MOTION_MASK);

    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_path, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_canvas, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_status, FALSE, FALSE, 0);

    gtk_widget_set_hexpand(editor_main_paned, TRUE);
    gtk_widget_set_vexpand(editor_main_paned, TRUE);
    gtk_box_pack_start(GTK_BOX(editor_shell), editor_header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_shell), editor_main_paned, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(browser_info), status, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(browser_info), selected, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(browser_info), paths_scroller, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(browser_info), actions, FALSE, FALSE, 0);
    gtk_widget_set_size_request(browser_info, -1, 120);
    gtk_container_add(GTK_CONTAINER(browser_info_scroller), browser_info);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(browser_info_scroller),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(browser_info_scroller, -1, 130);
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_info_scroller), "surface");
    gtk_revealer_set_transition_type(GTK_REVEALER(browser_info_revealer),
                                     GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_transition_duration(GTK_REVEALER(browser_info_revealer), 160);
    gtk_container_add(GTK_CONTAINER(browser_info_revealer), browser_info_scroller);
    gtk_widget_set_hexpand(browser_info_revealer, TRUE);

    g_object_set(G_OBJECT(browser_split), "wide-handle", TRUE, NULL);
    gtk_widget_set_hexpand(browser_split, TRUE);
    gtk_widget_set_vexpand(browser_split, TRUE);
    gtk_paned_pack1(GTK_PANED(browser_split), browser_info_revealer, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(browser_split), thumbs_scroller, TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(browser_split), 120);

    gtk_box_pack_start(GTK_BOX(browser_shell), browser_split, TRUE, TRUE, 0);
    gtk_widget_set_vexpand(browser_shell, FALSE);
    gtk_widget_set_size_request(browser_shell, -1, 230);
    gtk_widget_set_margin_bottom(browser_shell, 10);

    g_object_set(G_OBJECT(workspace_split), "wide-handle", TRUE, NULL);
    gtk_widget_set_hexpand(workspace_split, TRUE);
    gtk_widget_set_vexpand(workspace_split, TRUE);
    gtk_widget_set_margin_bottom(workspace_split, 8);
    gtk_paned_pack1(GTK_PANED(workspace_split), editor_shell, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(workspace_split), browser_shell, FALSE, FALSE);
    gtk_paned_set_position(GTK_PANED(workspace_split), 430);

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), folder, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), workspace_split, TRUE, TRUE, 0);

    state->shots_icon_view = icon_view;
    state->shots_status = status;
    state->shots_selected = selected;
    state->shots_paths_view = paths_view;
    state->shots_folder_label = folder;
    state->shots_browser_info_revealer = browser_info_revealer;
    state->shots_browser_toggle_btn = btn_toggle_shell;
    state->shots_browser_split_paned = browser_split;
    state->shots_editor_canvas = editor_canvas;
    state->shots_editor_status = editor_status;
    state->shots_editor_tool_combo = tool_combo;
    state->shots_editor_stamp_combo = stamp_combo;
    state->shots_editor_text_entry = text_entry;
    state->shots_editor_color_btn = color_btn;
    state->shots_editor_arrow_width_scale = arrow_width_scale;
    state->shots_editor_arrow_head_len_scale = arrow_head_len_scale;
    state->shots_editor_arrow_head_angle_scale = arrow_head_angle_scale;
    state->shots_editor_auto_step_check = auto_step_check;
    state->shots_editor_step_label = step_label;
    state->shots_editor_path_label = editor_path;
    state->shots_editor_main_paned = editor_main_paned;
    state->shots_editor_canvas_panel = editor_canvas_panel;
    state->shots_editor_sidebar = editor_sidebar;
    state->shots_editor_sidebar_scroller = editor_sidebar_scroller;
    state->shots_editor_dock_toggle_btn = btn_dock_toggle;
    state->shots_editor_tools_docked_right = TRUE;
    state->shots_editor_next_step = 1;
    shots_editor_ensure_marks(state);
    shots_editor_update_step_label(state);
    shots_editor_apply_dock_layout(state);

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(on_shots_refresh_clicked), state);
    g_signal_connect(btn_select_all, "clicked", G_CALLBACK(on_shots_select_all_clicked), state);
    g_signal_connect(btn_clear_selection, "clicked", G_CALLBACK(on_shots_clear_selection_clicked), state);
    g_signal_connect(btn_toggle_shell, "clicked", G_CALLBACK(on_shots_toggle_shell_clicked), state);
    g_signal_connect(btn_open, "clicked", G_CALLBACK(on_shots_open_folder_clicked), state);
    g_signal_connect(btn_import, "clicked", G_CALLBACK(on_shots_import_clicked), state);
    g_signal_connect(btn_copy_paths, "clicked", G_CALLBACK(on_shots_copy_paths_clicked), state);
    g_signal_connect(btn_copy_prompt, "clicked", G_CALLBACK(on_shots_copy_prompt_clicked), state);
    g_signal_connect(icon_view, "selection-changed", G_CALLBACK(on_shots_selection_changed), state);
    g_signal_connect(icon_view, "item-activated", G_CALLBACK(on_shots_item_activated), state);
    g_signal_connect(btn_capture, "clicked", G_CALLBACK(on_shots_editor_capture_clicked), state);
    g_signal_connect(btn_edit_selected, "clicked", G_CALLBACK(on_shots_editor_edit_selected_clicked), state);
    g_signal_connect(btn_undo, "clicked", G_CALLBACK(on_shots_editor_undo_clicked), state);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_shots_editor_clear_clicked), state);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_shots_editor_save_clicked), state);
    g_signal_connect(btn_save_svg, "clicked", G_CALLBACK(on_shots_editor_save_svg_clicked), state);
    g_signal_connect(btn_dock_toggle, "clicked", G_CALLBACK(on_shots_editor_toggle_dock_clicked), state);
    g_signal_connect(btn_step_reset, "clicked", G_CALLBACK(on_shots_editor_step_reset_clicked), state);
    g_signal_connect(btn_apply_text, "clicked", G_CALLBACK(on_shots_editor_apply_text_clicked), state);
    g_signal_connect(tool_combo, "changed", G_CALLBACK(on_shots_editor_tool_changed), state);
    g_signal_connect(color_btn, "color-set", G_CALLBACK(on_shots_editor_color_set), state);
    g_signal_connect(arrow_width_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_head_len_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_head_angle_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(editor_canvas, "draw", G_CALLBACK(on_shots_editor_draw), state);
    g_signal_connect(editor_canvas, "button-press-event", G_CALLBACK(on_shots_editor_button_press), state);
    g_signal_connect(editor_canvas, "button-release-event", G_CALLBACK(on_shots_editor_button_release), state);
    g_signal_connect(editor_canvas, "motion-notify-event", G_CALLBACK(on_shots_editor_motion), state);

    shots_set_browser_info_visible(state, FALSE);
    shots_reload(state);
    return root;
}

static void app_state_free(AppState *state) {
    if (!state) {
        return;
    }
    if (state->css_provider) {
        g_object_unref(state->css_provider);
    }
    if (state->shots_editor_pixbuf) {
        g_object_unref(state->shots_editor_pixbuf);
    }
    if (state->shots_editor_marks) {
        g_ptr_array_free(state->shots_editor_marks, TRUE);
    }
    g_free(state->repo_root);
    g_free(state->launch_dir);
    g_free(state->redshift_script);
    g_free(state->import_script);
    g_free(state->shots_dir);
    g_free(state->shots_editor_image_path);
    g_free(state);
}

static void on_app_activate(GtkApplication *app, gpointer user_data) {
    AppState *state = user_data;
    GtkWidget *header = NULL;
    GtkWidget *title = NULL;
    GtkWidget *theme_box = NULL;
    GtkWidget *theme_label = NULL;
    GtkWidget *theme_combo = NULL;
    GtkWidget *root = NULL;
    GtkWidget *notebook = NULL;
    GtkWidget *night_tab = NULL;
    GtkWidget *audio_tab = NULL;
    GtkWidget *utilities_tab = NULL;
    GtkWidget *shots_tab = NULL;

    state->window = gtk_application_window_new(app);
    gtk_style_context_add_class(gtk_widget_get_style_context(state->window), "app-window");
    apply_css_theme(state, TRUE);

    gtk_window_set_title(GTK_WINDOW(state->window), "Linux Utilities Control Center");
    gtk_window_set_default_size(GTK_WINDOW(state->window), 1180, 760);

    header = gtk_header_bar_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "app-header");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);
    title = gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(title), "Linux Utilities Control Center");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");
    gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header), title);

    theme_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    theme_label = gtk_label_new("Theme");
    theme_combo = gtk_combo_box_text_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(theme_box), "theme-strip");
    gtk_style_context_add_class(gtk_widget_get_style_context(theme_label), "theme-strip-label");
    gtk_style_context_add_class(gtk_widget_get_style_context(theme_combo), "theme-strip-combo");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(theme_combo), "dark", "Dark");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(theme_combo), "light", "Light");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(theme_combo), "dark");
    gtk_box_pack_start(GTK_BOX(theme_box), theme_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(theme_box), theme_combo, FALSE, FALSE, 0);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header), theme_box);
    g_signal_connect(theme_combo, "changed", G_CALLBACK(on_theme_combo_changed), state);

    gtk_window_set_titlebar(GTK_WINDOW(state->window), header);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(root), "app-root");
    notebook = gtk_notebook_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(notebook), "app-notebook");
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);

    night_tab = build_night_tab(state);
    audio_tab = build_audio_tab(state);
    utilities_tab = build_utilities_tab(state);
    shots_tab = build_screenshots_tab(state);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), night_tab, gtk_label_new("Night Light"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), audio_tab, gtk_label_new("Audio"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), utilities_tab, gtk_label_new("Utilities"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), shots_tab, gtk_label_new("Screenshots"));

    gtk_box_pack_start(GTK_BOX(root), notebook, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(state->window), root);
    gtk_widget_show_all(state->window);
}

int main(int argc, char **argv) {
    AppState *state = g_new0(AppState, 1);
    gint status = 0;
    gchar *exe_path = NULL;
    gchar *target_dir = NULL;
    gchar *target_basename = NULL;
    gchar *candidate_shots = NULL;
    gchar *argv0_only[] = { NULL, NULL };

    state->shots_editor_selected_mark_index = -1;
    state->shots_editor_next_step = 1;

    if (argv[0] && *argv[0]) {
        if (g_path_is_absolute(argv[0]) || strchr(argv[0], '/')) {
            exe_path = g_canonicalize_filename(argv[0], NULL);
        } else {
            exe_path = g_find_program_in_path(argv[0]);
        }
    }

    if (exe_path) {
        state->repo_root = g_path_get_dirname(exe_path);
    } else {
        state->repo_root = g_get_current_dir();
    }
    state->redshift_script = g_build_filename(state->repo_root, "redshift.sh", NULL);
    state->import_script = g_build_filename(state->repo_root, "import_screenshots.sh", NULL);

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
    } else if ((target_basename && g_ascii_strcasecmp(target_basename, "Screenshots") == 0) ||
               dir_contains_images(target_dir)) {
        state->shots_dir = g_strdup(target_dir);
    } else {
        state->shots_dir = g_strdup(candidate_shots);
    }

    state->app = gtk_application_new("com.antshiv.linuxutils.controlcenter", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(state->app, "activate", G_CALLBACK(on_app_activate), state);

    /* We consume optional args ourselves. Avoid GtkApplication treating them
     * as files to open. */
    argv0_only[0] = argv[0];
    status = g_application_run(G_APPLICATION(state->app), 1, argv0_only);

    g_object_unref(state->app);
    g_free(exe_path);
    g_free(target_dir);
    g_free(target_basename);
    g_free(candidate_shots);
    app_state_free(state);
    return status;
}
