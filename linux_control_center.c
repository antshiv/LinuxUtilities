#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <cairo-svg.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

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

typedef enum {
    SHOT_TEXT_STYLE_NORMAL = 0,
    SHOT_TEXT_STYLE_BOLD,
    SHOT_TEXT_STYLE_ITALIC,
    SHOT_TEXT_STYLE_BOLD_ITALIC
} ShotTextStyle;

typedef enum {
    SHOT_ARROW_THEME_CLASSIC = 0,
    SHOT_ARROW_THEME_BOLD,
    SHOT_ARROW_THEME_POINTER,
    SHOT_ARROW_THEME_DASHED,
    SHOT_ARROW_THEME_DOTTED,
    SHOT_ARROW_THEME_DOUBLE
} ShotArrowTheme;

typedef enum {
    SHOT_ARROW_HEAD_ALIGN_INSIDE = 0,
    SHOT_ARROW_HEAD_ALIGN_CENTER,
    SHOT_ARROW_HEAD_ALIGN_OUTSIDE
} ShotArrowHeadAlign;

typedef enum {
    SHOT_RECT_STYLE_BOX = 0,
    SHOT_RECT_STYLE_CALLOUT_LEFT,
    SHOT_RECT_STYLE_CALLOUT_RIGHT,
    SHOT_RECT_STYLE_HIGHLIGHT,
    SHOT_RECT_STYLE_BOX_SHADOW
} ShotRectStyle;

enum {
    SHOT_HANDLE_NONE = 0,
    SHOT_HANDLE_ARROW_START = 1,
    SHOT_HANDLE_ARROW_END = 2,
    SHOT_HANDLE_MOVE = 3,
    SHOT_HANDLE_RECT_START = 4,
    SHOT_HANDLE_RECT_END = 5,
    SHOT_HANDLE_ARROW_CTRL1 = 6,
    SHOT_HANDLE_ARROW_CTRL2 = 7
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
    gdouble arrow_dash_len;
    gdouble arrow_dash_gap;
    guint arrow_theme;
    guint arrow_head_align;
    gdouble arrow_shadow_offset;
    gdouble arrow_curve_bend;
    gdouble arrow_c1x;
    gdouble arrow_c1y;
    gdouble arrow_c2x;
    gdouble arrow_c2y;
    gdouble text_size;
    gdouble text_fill_opacity;
    gdouble rect_fill_opacity;
    gdouble text_stroke_width;
    gchar *text;
    GdkRGBA color;
    guint text_style;
    guint rect_style;
    gboolean text_fill_enabled;
    gboolean text_stroke_enabled;
    gboolean text_shadow_enabled;
    gboolean arrow_shadow_enabled;
    gboolean arrow_curved;
    gboolean rect_fill_enabled;
    gboolean rect_stroke_enabled;
    gboolean rect_shadow_enabled;
    gboolean step_link_enabled;
    gint step_link_from_index;
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
    GtkWidget *shots_top_actions_revealer;
    GtkWidget *shots_top_actions_toggle_btn;
    GtkWidget *shots_top_compact_toggle_btn;
    GtkWidget *shots_quick_start_box;
    gboolean shots_top_compact;
    GtkWidget *shots_search_entry;
    GtkWidget *shots_sort_newest_btn;
    GtkWidget *shots_sort_name_btn;
    GtkWidget *shots_sort_type_btn;
    GtkWidget *shots_editor_canvas;
    GtkWidget *shots_editor_canvas_overlay;
    GtkWidget *shots_editor_status;
    GtkWidget *shots_editor_tool_combo;
    GtkWidget *shots_tool_btn_select;
    GtkWidget *shots_tool_btn_arrow;
    GtkWidget *shots_tool_btn_rect;
    GtkWidget *shots_tool_btn_text;
    GtkWidget *shots_tool_btn_stamp;
    GtkWidget *shots_editor_stamp_combo;
    GtkWidget *shots_editor_text_entry;
    GtkWidget *shots_editor_color_btn;
    GtkWidget *shots_editor_font_style_combo;
    GtkWidget *shots_editor_font_size_scale;
    GtkWidget *shots_editor_text_stroke_width_scale;
    GtkWidget *shots_editor_rect_fill_opacity_scale;
    GtkWidget *shots_editor_text_fill_check;
    GtkWidget *shots_editor_text_stroke_check;
    GtkWidget *shots_editor_text_shadow_check;
    GtkWidget *shots_editor_rect_style_combo;
    GtkWidget *shots_editor_rect_fill_check;
    GtkWidget *shots_editor_rect_stroke_check;
    GtkWidget *shots_editor_rect_shadow_check;
    GtkWidget *shots_editor_arrow_width_scale;
    GtkWidget *shots_editor_arrow_head_len_scale;
    GtkWidget *shots_editor_arrow_head_angle_scale;
    GtkWidget *shots_editor_arrow_theme_combo;
    GtkWidget *shots_editor_arrow_dash_len_scale;
    GtkWidget *shots_editor_arrow_dash_gap_scale;
    GtkWidget *shots_editor_arrow_head_align_combo;
    GtkWidget *shots_editor_arrow_shadow_check;
    GtkWidget *shots_editor_arrow_shadow_offset_scale;
    GtkWidget *shots_editor_arrow_curve_check;
    GtkWidget *shots_editor_arrow_curve_bend_scale;
    GtkWidget *shots_editor_auto_step_check;
    GtkWidget *shots_editor_step_link_check;
    GtkWidget *shots_editor_step_label;
    GtkWidget *shots_editor_props_hint;
    GtkWidget *shots_editor_group_stamp;
    GtkWidget *shots_editor_group_text;
    GtkWidget *shots_editor_group_callout;
    GtkWidget *shots_editor_group_arrow;
    GtkWidget *shots_editor_group_common;
    GtkWidget *shots_editor_quick_styles_scroller;
    GtkWidget *shots_editor_quick_styles_row;
    GtkWidget *shots_editor_styles_toggle_btn;
    gboolean shots_editor_quick_styles_visible;
    GtkWidget *shots_browser_info_revealer;
    GtkWidget *shots_browser_toggle_btn;
    GtkWidget *shots_browser_split_paned;
    GtkWidget *shots_browser_shell;
    GtkWidget *shots_browser_shell_toggle_btn;
    gboolean shots_browser_shell_visible;
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
    gdouble shots_editor_edit_origin_c1x;
    gdouble shots_editor_edit_origin_c1y;
    gdouble shots_editor_edit_origin_c2x;
    gdouble shots_editor_edit_origin_c2y;
    gboolean shots_editor_dragging;
    ShotMarkType shots_editor_drag_mark_type;
    gdouble shots_editor_drag_start_x;
    gdouble shots_editor_drag_start_y;
    gdouble shots_editor_drag_cur_x;
    gdouble shots_editor_drag_cur_y;
    gdouble shots_editor_draw_scale;
    gdouble shots_editor_draw_offset_x;
    gdouble shots_editor_draw_offset_y;
    gdouble shots_editor_zoom_factor;
    gint shots_editor_next_step;
    gboolean shots_editor_style_syncing;
    gboolean shots_editor_tool_syncing;
    gboolean shots_editor_tools_docked_right;
    GtkWidget *shots_editor_inline_entry;
    gint shots_editor_inline_mark_index;
    gboolean shots_editor_inline_active;

    GtkWidget *utils_status;
    GtkWidget *shortcuts_status;
    GtkWidget *main_notebook;

    gchar *repo_root;
    gchar *launch_dir;
    gchar *redshift_script;
    gchar *import_script;
    gchar *shots_dir;
} AppState;

static void shots_reload(AppState *state);
static void shots_editor_show_color_palette_menu(AppState *state, GdkEventButton *event);
static void shots_editor_apply_text_value_to_mark(AppState *state, ShotMark *mark, const gchar *raw_text);
static void shots_editor_inline_begin(AppState *state, gint mark_index);
static void shots_editor_inline_hide(AppState *state, gboolean apply_changes);
static void shots_editor_inline_reposition(AppState *state);
static void shots_editor_sync_tool_buttons(AppState *state, const gchar *tool_id);
static void shots_editor_update_properties_visibility(AppState *state);
static void shots_editor_apply_quick_style(AppState *state, const gchar *style_id);
static void shots_editor_update_quick_styles_visibility(AppState *state);
static void shots_set_top_compact(AppState *state, gboolean compact);
static void shots_editor_arrow_initialize_curve(ShotMark *mark, gdouble bend);
static void shots_editor_arrow_ensure_curve_points(ShotMark *mark);

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

static const gchar *image_entry_ext(const ImageEntry *entry) {
    const gchar *dot = NULL;
    if (!entry || !entry->name) {
        return "";
    }
    dot = strrchr(entry->name, '.');
    return dot ? dot + 1 : "";
}

static gint compare_entries_name_asc(gconstpointer a, gconstpointer b) {
    const ImageEntry *ea = *(ImageEntry * const *)a;
    const ImageEntry *eb = *(ImageEntry * const *)b;
    gint cmp = g_ascii_strcasecmp(ea ? ea->name : "", eb ? eb->name : "");
    if (cmp != 0) {
        return cmp;
    }
    if (ea && eb && ea->mtime != eb->mtime) {
        return (ea->mtime < eb->mtime) ? 1 : -1;
    }
    return 0;
}

static gint compare_entries_type_then_name(gconstpointer a, gconstpointer b) {
    const ImageEntry *ea = *(ImageEntry * const *)a;
    const ImageEntry *eb = *(ImageEntry * const *)b;
    gint ext_cmp = g_ascii_strcasecmp(image_entry_ext(ea), image_entry_ext(eb));
    if (ext_cmp != 0) {
        return ext_cmp;
    }
    return compare_entries_name_asc(a, b);
}

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#endif
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
        ".header-compact {"
        "  background: #121b29;"
        "  border: 1px solid #2a3a52;"
        "  border-radius: 10px;"
        "  padding: 8px 10px;"
        "}"
        ".header-compact image {"
        "  opacity: 0.92;"
        "}"
        ".top-primary-row button {"
        "  margin: 0 8px 0 0;"
        "  min-height: 30px;"
        "  padding: 4px 10px;"
        "}"
        ".top-secondary-row button {"
        "  margin: 0 6px 0 0;"
        "  min-height: 24px;"
        "  padding: 2px 8px;"
        "}"
        ".thumb-filter-row {"
        "  background: #151f2d;"
        "  border: 1px solid #2b3b53;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "}"
        ".sort-chip {"
        "  min-width: 62px;"
        "  margin: 0 6px 0 0;"
        "}"
        ".sort-chip:checked {"
        "  background: #0e639c;"
        "  color: #ffffff;"
        "  border-color: #2f78b7;"
        "  box-shadow: 0 0 0 1px rgba(95, 160, 235, 0.36), 0 0 8px rgba(48, 120, 195, 0.28);"
        "}"
        ".quick-styles-row {"
        "  background: #131d2b;"
        "  border: 1px solid #2a3b53;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "}"
        ".quick-style-btn {"
        "  min-width: 86px;"
        "  min-height: 72px;"
        "  margin: 0 6px 0 0;"
        "  padding: 4px;"
        "  border-radius: 8px;"
        "}"
        ".quick-style-btn label {"
        "  font-size: 10px;"
        "  font-weight: 600;"
        "}"
        ".tool-ribbon {"
        "  background: #151d29;"
        "  border: 1px solid #2f3d52;"
        "  border-radius: 8px;"
        "  padding: 4px;"
        "}"
        ".tool-btn {"
        "  min-width: 84px;"
        "  min-height: 30px;"
        "  padding: 4px 8px;"
        "  border-radius: 6px;"
        "  border: 1px solid transparent;"
        "  background: transparent;"
        "  box-shadow: none;"
        "}"
        ".tool-btn label {"
        "  font-weight: 600;"
        "}"
        ".tool-btn:hover {"
        "  background: #253246;"
        "  border-color: #3f5675;"
        "}"
        ".tool-btn:checked {"
        "  background: #0e639c;"
        "  border-color: #4d7fb8;"
        "  box-shadow: 0 0 0 1px rgba(95, 160, 235, 0.45), 0 0 10px rgba(48, 120, 195, 0.35);"
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
        "}"
        ".shortcut-scroll > viewport {"
        "  background: transparent;"
        "}"
        ".shortcut-card {"
        "  background: #172231;"
        "  border: 1px solid #35506f;"
        "  border-radius: 12px;"
        "}"
        ".shortcut-flow {"
        "  background: #12263c;"
        "  border: 1px solid #2f6ea7;"
        "  border-radius: 10px;"
        "  padding: 10px 12px;"
        "}"
        ".shortcut-flow label {"
        "  color: #d8ebff;"
        "}"
        ".shortcut-card-title {"
        "  color: #f1f6ff;"
        "  font-size: 12px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-row {"
        "  background: rgba(26, 44, 66, 0.62);"
        "  border: 1px solid #28415e;"
        "  border-radius: 8px;"
        "  padding: 6px 8px;"
        "}"
        ".shortcut-keychip {"
        "  background: #0f3559;"
        "  color: #eef7ff;"
        "  border: 1px solid #4588c7;"
        "  border-radius: 7px;"
        "  padding: 4px 8px;"
        "  font-family: 'Ubuntu Mono', monospace;"
        "  font-size: 10px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-action {"
        "  color: #ecf3ff;"
        "  font-size: 11px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-hint {"
        "  color: #acc0d8;"
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
        ".header-compact {"
        "  background: #eef4fc;"
        "  border: 1px solid #c6d8ed;"
        "  border-radius: 10px;"
        "  padding: 8px 10px;"
        "}"
        ".header-compact image {"
        "  opacity: 0.92;"
        "}"
        ".top-primary-row button {"
        "  margin: 0 8px 0 0;"
        "  min-height: 30px;"
        "  padding: 4px 10px;"
        "}"
        ".top-secondary-row button {"
        "  margin: 0 6px 0 0;"
        "  min-height: 24px;"
        "  padding: 2px 8px;"
        "}"
        ".thumb-filter-row {"
        "  background: #edf4fc;"
        "  border: 1px solid #c6d8ed;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "}"
        ".sort-chip {"
        "  min-width: 62px;"
        "  margin: 0 6px 0 0;"
        "}"
        ".sort-chip:checked {"
        "  background: #3e88cc;"
        "  color: #ffffff;"
        "  border-color: #2f6fae;"
        "  box-shadow: 0 0 0 1px rgba(94, 150, 220, 0.36), 0 0 8px rgba(84, 140, 210, 0.24);"
        "}"
        ".quick-styles-row {"
        "  background: #edf4fc;"
        "  border: 1px solid #c6d8ed;"
        "  border-radius: 8px;"
        "  padding: 6px;"
        "}"
        ".quick-style-btn {"
        "  min-width: 86px;"
        "  min-height: 72px;"
        "  margin: 0 6px 0 0;"
        "  padding: 4px;"
        "  border-radius: 8px;"
        "}"
        ".quick-style-btn label {"
        "  font-size: 10px;"
        "  font-weight: 600;"
        "}"
        ".tool-ribbon {"
        "  background: #e9f1fb;"
        "  border: 1px solid #bfd2e9;"
        "  border-radius: 8px;"
        "  padding: 4px;"
        "}"
        ".tool-btn {"
        "  min-width: 84px;"
        "  min-height: 30px;"
        "  padding: 4px 8px;"
        "  border-radius: 6px;"
        "  border: 1px solid transparent;"
        "  background: transparent;"
        "  box-shadow: none;"
        "}"
        ".tool-btn label {"
        "  font-weight: 600;"
        "}"
        ".tool-btn:hover {"
        "  background: #d9e8f9;"
        "  border-color: #a8c3e3;"
        "}"
        ".tool-btn:checked {"
        "  background: #3e88cc;"
        "  border-color: #2f6fae;"
        "  color: #ffffff;"
        "  box-shadow: 0 0 0 1px rgba(94, 150, 220, 0.45), 0 0 10px rgba(84, 140, 210, 0.28);"
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
        "}"
        ".shortcut-scroll > viewport {"
        "  background: transparent;"
        "}"
        ".shortcut-card {"
        "  background: #fbfdff;"
        "  border: 1px solid #aac6e3;"
        "  border-radius: 12px;"
        "}"
        ".shortcut-flow {"
        "  background: #eaf5ff;"
        "  border: 1px solid #7aaddb;"
        "  border-radius: 10px;"
        "  padding: 10px 12px;"
        "}"
        ".shortcut-flow label {"
        "  color: #1d3f5d;"
        "}"
        ".shortcut-card-title {"
        "  color: #1f3043;"
        "  font-size: 12px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-row {"
        "  background: #f3f8ff;"
        "  border: 1px solid #c5d9ef;"
        "  border-radius: 8px;"
        "  padding: 6px 8px;"
        "}"
        ".shortcut-keychip {"
        "  background: #e3f0ff;"
        "  color: #143754;"
        "  border: 1px solid #84add2;"
        "  border-radius: 7px;"
        "  padding: 4px 8px;"
        "  font-family: 'Ubuntu Mono', monospace;"
        "  font-size: 10px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-action {"
        "  color: #1e2f40;"
        "  font-size: 11px;"
        "  font-weight: 700;"
        "}"
        ".shortcut-hint {"
        "  color: #486885;"
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
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

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

static void style_sort_chip_button(GtkWidget *button) {
    GtkStyleContext *ctx = gtk_widget_get_style_context(button);
    style_action_button(button, FALSE);
    gtk_style_context_add_class(ctx, "sort-chip");
    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), FALSE);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
}

static GtkWidget *create_quick_style_button(AppState *state,
                                            const gchar *style_id,
                                            const gchar *label,
                                            const gchar *relative_svg_path,
                                            const gchar *category) {
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *image = NULL;
    GtkWidget *text = gtk_label_new(label ? label : "Style");
    GtkStyleContext *ctx = gtk_widget_get_style_context(button);
    gchar *fullpath = NULL;
    GError *error = NULL;

    style_action_button(button, FALSE);
    gtk_style_context_add_class(ctx, "quick-style-btn");
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_widget_set_halign(text, GTK_ALIGN_CENTER);

    if (relative_svg_path && *relative_svg_path) {
        fullpath = g_build_filename(state && state->repo_root ? state->repo_root : ".",
                                    relative_svg_path,
                                    NULL);
        if (g_file_test(fullpath, G_FILE_TEST_EXISTS)) {
            GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(fullpath, 64, 38, TRUE, &error);
            if (pix) {
                image = gtk_image_new_from_pixbuf(pix);
                g_object_unref(pix);
            } else {
                g_clear_error(&error);
            }
        }
    }
    if (!image) {
        image = gtk_image_new_from_icon_name("applications-graphics-symbolic", GTK_ICON_SIZE_BUTTON);
    }
    gtk_box_pack_start(GTK_BOX(box), image, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), text, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(button), box);
    g_object_set_data_full(G_OBJECT(button), "quick-style-id", g_strdup(style_id), g_free);
    g_object_set_data_full(G_OBJECT(button), "quick-style-category", g_strdup(category ? category : "all"), g_free);
    gtk_widget_set_tooltip_text(button, label ? label : "Quick Style");
    g_free(fullpath);
    return button;
}

static void set_button_icon_if_available(GtkWidget *button, const gchar *icon_name, GtkIconSize size) {
    GtkIconTheme *theme = gtk_icon_theme_get_default();

    if (!button || !icon_name || !*icon_name || !theme || !gtk_icon_theme_has_icon(theme, icon_name)) {
        return;
    }

    gtk_button_set_image(GTK_BUTTON(button), gtk_image_new_from_icon_name(icon_name, size));
    gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);
}

static GtkWidget *create_tool_ribbon_button(GtkWidget *group_source,
                                            const gchar *tool_id,
                                            const gchar *icon_name,
                                            const gchar *fallback_label,
                                            const gchar *tooltip) {
    GtkWidget *button = NULL;

    if (group_source) {
        button = gtk_radio_button_new_from_widget(GTK_RADIO_BUTTON(group_source));
    } else {
        button = gtk_radio_button_new(NULL);
    }

    gtk_toggle_button_set_mode(GTK_TOGGLE_BUTTON(button), FALSE);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_style_context_add_class(gtk_widget_get_style_context(button), "tool-btn");
    gtk_button_set_image_position(GTK_BUTTON(button), GTK_POS_LEFT);
    set_button_icon_if_available(button, icon_name, GTK_ICON_SIZE_MENU);
    if (fallback_label && *fallback_label) {
        gtk_button_set_label(GTK_BUTTON(button), fallback_label);
    }

    if (tooltip && *tooltip) {
        gtk_widget_set_tooltip_text(button, tooltip);
    }
    g_object_set_data(G_OBJECT(button), "tool-id", (gpointer)tool_id);
    return button;
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

static gdouble shots_editor_get_arrow_dash_len_control(AppState *state) {
    if (state && state->shots_editor_arrow_dash_len_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_dash_len_scale));
    }
    return 10.0;
}

static gdouble shots_editor_get_arrow_dash_gap_control(AppState *state) {
    if (state && state->shots_editor_arrow_dash_gap_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_dash_gap_scale));
    }
    return 7.0;
}

static gdouble shots_editor_get_arrow_shadow_offset_control(AppState *state) {
    if (state && state->shots_editor_arrow_shadow_offset_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_shadow_offset_scale));
    }
    return 0.0;
}

static gdouble shots_editor_get_arrow_curve_bend_control(AppState *state) {
    if (state && state->shots_editor_arrow_curve_bend_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_arrow_curve_bend_scale));
    }
    return 0.35;
}

static gboolean shots_editor_get_arrow_curved_control(AppState *state) {
    if (!state || !state->shots_editor_arrow_curve_check) {
        return FALSE;
    }
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shots_editor_arrow_curve_check));
}

static void shots_editor_update_arrow_curve_controls_sensitivity(AppState *state) {
    gboolean curved = FALSE;

    if (!state) {
        return;
    }
    curved = shots_editor_get_arrow_curved_control(state);
    if (state->shots_editor_arrow_curve_bend_scale) {
        gtk_widget_set_sensitive(state->shots_editor_arrow_curve_bend_scale, curved);
    }
}

static guint shots_editor_get_arrow_head_align_control(AppState *state) {
    const gchar *id = NULL;

    if (!state || !state->shots_editor_arrow_head_align_combo) {
        return SHOT_ARROW_HEAD_ALIGN_INSIDE;
    }
    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_head_align_combo));
    if (!id || g_strcmp0(id, "inside") == 0) {
        return SHOT_ARROW_HEAD_ALIGN_INSIDE;
    }
    if (g_strcmp0(id, "center") == 0) {
        return SHOT_ARROW_HEAD_ALIGN_CENTER;
    }
    if (g_strcmp0(id, "outside") == 0) {
        return SHOT_ARROW_HEAD_ALIGN_OUTSIDE;
    }
    return SHOT_ARROW_HEAD_ALIGN_INSIDE;
}

static const gchar *shots_editor_arrow_head_align_to_id(guint align) {
    switch (align) {
        case SHOT_ARROW_HEAD_ALIGN_CENTER:
            return "center";
        case SHOT_ARROW_HEAD_ALIGN_OUTSIDE:
            return "outside";
        case SHOT_ARROW_HEAD_ALIGN_INSIDE:
        default:
            return "inside";
    }
}

static guint shots_editor_get_arrow_theme_control(AppState *state) {
    const gchar *id = NULL;

    if (!state || !state->shots_editor_arrow_theme_combo) {
        return SHOT_ARROW_THEME_CLASSIC;
    }
    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_theme_combo));
    if (!id || g_strcmp0(id, "classic") == 0) {
        return SHOT_ARROW_THEME_CLASSIC;
    }
    if (g_strcmp0(id, "bold") == 0) {
        return SHOT_ARROW_THEME_BOLD;
    }
    if (g_strcmp0(id, "pointer") == 0) {
        return SHOT_ARROW_THEME_POINTER;
    }
    if (g_strcmp0(id, "dashed") == 0) {
        return SHOT_ARROW_THEME_DASHED;
    }
    if (g_strcmp0(id, "dotted") == 0) {
        return SHOT_ARROW_THEME_DOTTED;
    }
    if (g_strcmp0(id, "double") == 0) {
        return SHOT_ARROW_THEME_DOUBLE;
    }
    return SHOT_ARROW_THEME_CLASSIC;
}

static const gchar *shots_editor_arrow_theme_to_id(guint theme) {
    switch (theme) {
        case SHOT_ARROW_THEME_BOLD:
            return "bold";
        case SHOT_ARROW_THEME_POINTER:
            return "pointer";
        case SHOT_ARROW_THEME_DASHED:
            return "dashed";
        case SHOT_ARROW_THEME_DOTTED:
            return "dotted";
        case SHOT_ARROW_THEME_DOUBLE:
            return "double";
        case SHOT_ARROW_THEME_CLASSIC:
        default:
            return "classic";
    }
}

static guint shots_editor_get_rect_style_control(AppState *state) {
    const gchar *id = NULL;

    if (!state || !state->shots_editor_rect_style_combo) {
        return SHOT_RECT_STYLE_BOX;
    }
    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(state->shots_editor_rect_style_combo));
    if (!id || g_strcmp0(id, "box") == 0) {
        return SHOT_RECT_STYLE_BOX;
    }
    if (g_strcmp0(id, "callout_left") == 0) {
        return SHOT_RECT_STYLE_CALLOUT_LEFT;
    }
    if (g_strcmp0(id, "callout_right") == 0) {
        return SHOT_RECT_STYLE_CALLOUT_RIGHT;
    }
    if (g_strcmp0(id, "highlight") == 0) {
        return SHOT_RECT_STYLE_HIGHLIGHT;
    }
    if (g_strcmp0(id, "box_shadow") == 0) {
        return SHOT_RECT_STYLE_BOX_SHADOW;
    }
    return SHOT_RECT_STYLE_BOX;
}

static const gchar *shots_editor_rect_style_to_id(guint style) {
    switch (style) {
        case SHOT_RECT_STYLE_CALLOUT_LEFT:
            return "callout_left";
        case SHOT_RECT_STYLE_CALLOUT_RIGHT:
            return "callout_right";
        case SHOT_RECT_STYLE_HIGHLIGHT:
            return "highlight";
        case SHOT_RECT_STYLE_BOX_SHADOW:
            return "box_shadow";
        case SHOT_RECT_STYLE_BOX:
        default:
            return "box";
    }
}

static gdouble shots_editor_get_text_size_control(AppState *state) {
    if (state && state->shots_editor_font_size_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_font_size_scale));
    }
    return 24.0;
}

static gdouble shots_editor_get_text_stroke_width_control(AppState *state) {
    if (state && state->shots_editor_text_stroke_width_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_text_stroke_width_scale));
    }
    return 2.0;
}

static gdouble shots_editor_get_rect_fill_opacity_control(AppState *state) {
    if (state && state->shots_editor_rect_fill_opacity_scale) {
        return gtk_range_get_value(GTK_RANGE(state->shots_editor_rect_fill_opacity_scale));
    }
    return 0.18;
}

static gboolean shots_editor_check_is_active(GtkWidget *check, gboolean fallback) {
    if (!check) {
        return fallback;
    }
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
}

static guint shots_editor_get_text_style_control(AppState *state) {
    const gchar *id = NULL;

    if (!state || !state->shots_editor_font_style_combo) {
        return SHOT_TEXT_STYLE_BOLD;
    }

    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(state->shots_editor_font_style_combo));
    if (!id || g_strcmp0(id, "bold") == 0) {
        return SHOT_TEXT_STYLE_BOLD;
    }
    if (g_strcmp0(id, "normal") == 0) {
        return SHOT_TEXT_STYLE_NORMAL;
    }
    if (g_strcmp0(id, "italic") == 0) {
        return SHOT_TEXT_STYLE_ITALIC;
    }
    if (g_strcmp0(id, "bolditalic") == 0) {
        return SHOT_TEXT_STYLE_BOLD_ITALIC;
    }
    return SHOT_TEXT_STYLE_BOLD;
}

static void shots_editor_apply_text_style_to_mark(AppState *state, ShotMark *mark) {
    if (!mark || (mark->type != SHOT_MARK_TEXT && mark->type != SHOT_MARK_RECT)) {
        return;
    }

    mark->text_size = shots_editor_get_text_size_control(state);
    mark->text_stroke_width = shots_editor_get_text_stroke_width_control(state);
    mark->text_style = shots_editor_get_text_style_control(state);
    mark->text_fill_enabled = shots_editor_check_is_active(state ? state->shots_editor_text_fill_check : NULL, TRUE);
    mark->text_stroke_enabled = shots_editor_check_is_active(state ? state->shots_editor_text_stroke_check : NULL, FALSE);
    mark->text_shadow_enabled = shots_editor_check_is_active(state ? state->shots_editor_text_shadow_check : NULL, TRUE);
    mark->text_fill_opacity = 1.0;

    if (!mark->text_fill_enabled && !mark->text_stroke_enabled) {
        mark->text_fill_enabled = TRUE;
    }
    if (mark->text_stroke_width <= 0.0) {
        mark->text_stroke_width = 2.0;
    }
    if (mark->text_size <= 0.0) {
        mark->text_size = 24.0;
    }
}

static void shots_editor_apply_rect_style_to_mark(AppState *state, ShotMark *mark) {
    if (!mark || mark->type != SHOT_MARK_RECT) {
        return;
    }

    mark->rect_style = shots_editor_get_rect_style_control(state);
    mark->rect_fill_enabled = shots_editor_check_is_active(state ? state->shots_editor_rect_fill_check : NULL, TRUE);
    mark->rect_stroke_enabled = shots_editor_check_is_active(state ? state->shots_editor_rect_stroke_check : NULL, TRUE);
    mark->rect_shadow_enabled = shots_editor_check_is_active(state ? state->shots_editor_rect_shadow_check : NULL, TRUE);
    mark->rect_fill_opacity = shots_editor_get_rect_fill_opacity_control(state);

    if (!mark->rect_fill_enabled && !mark->rect_stroke_enabled) {
        mark->rect_stroke_enabled = TRUE;
    }
    if (mark->rect_fill_opacity < 0.0) {
        mark->rect_fill_opacity = 0.0;
    }
    if (mark->rect_fill_opacity > 1.0) {
        mark->rect_fill_opacity = 1.0;
    }
}

static void shots_editor_apply_arrow_style_to_mark(AppState *state, ShotMark *mark) {
    gboolean curved = FALSE;
    gdouble bend = 0.35;
    gboolean curve_changed = FALSE;

    if (!mark || mark->type != SHOT_MARK_ARROW) {
        return;
    }
    mark->stroke_width = shots_editor_get_arrow_width_control(state);
    mark->arrow_head_len = shots_editor_get_arrow_head_len_control(state);
    mark->arrow_head_spread = shots_editor_get_arrow_head_spread_control(state);
    mark->arrow_dash_len = shots_editor_get_arrow_dash_len_control(state);
    mark->arrow_dash_gap = shots_editor_get_arrow_dash_gap_control(state);
    mark->arrow_theme = shots_editor_get_arrow_theme_control(state);
    mark->arrow_head_align = shots_editor_get_arrow_head_align_control(state);
    mark->arrow_shadow_enabled = shots_editor_check_is_active(state ? state->shots_editor_arrow_shadow_check : NULL,
                                                              FALSE);
    mark->arrow_shadow_offset = shots_editor_get_arrow_shadow_offset_control(state);
    curved = shots_editor_get_arrow_curved_control(state);
    bend = shots_editor_get_arrow_curve_bend_control(state);
    curve_changed = (mark->arrow_curved != curved) || (fabs(mark->arrow_curve_bend - bend) > 1e-6);
    mark->arrow_curved = curved;
    mark->arrow_curve_bend = bend;
    if (mark->arrow_curved) {
        if (curve_changed) {
            shots_editor_arrow_initialize_curve(mark, bend);
        } else {
            shots_editor_arrow_ensure_curve_points(mark);
        }
    }
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
    gdouble dash_len = 10.0;
    gdouble dash_gap = 7.0;
    gdouble shadow_offset = 3.0;
    gdouble curve_bend = 0.35;
    gboolean shadow_enabled = FALSE;
    gboolean curved = FALSE;
    const gchar *head_align_id = "inside";
    const gchar *theme_id = "classic";

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
    if (mark->arrow_dash_len > 0.0) {
        dash_len = mark->arrow_dash_len;
    }
    if (mark->arrow_dash_gap > 0.0) {
        dash_gap = mark->arrow_dash_gap;
    }
    shadow_enabled = mark->arrow_shadow_enabled;
    shadow_offset = mark->arrow_shadow_offset >= 0.0 ? mark->arrow_shadow_offset : 3.0;
    curved = mark->arrow_curved;
    curve_bend = mark->arrow_curve_bend;
    if (!curved && fabs(curve_bend) <= 1e-6) {
        curve_bend = 0.35;
    }
    theme_id = shots_editor_arrow_theme_to_id(mark->arrow_theme);
    head_align_id = shots_editor_arrow_head_align_to_id(mark->arrow_head_align);

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
    if (state->shots_editor_arrow_theme_combo) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_theme_combo), theme_id);
    }
    if (state->shots_editor_arrow_head_align_combo) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_head_align_combo), head_align_id);
    }
    if (state->shots_editor_arrow_dash_len_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_dash_len_scale), dash_len);
    }
    if (state->shots_editor_arrow_dash_gap_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_dash_gap_scale), dash_gap);
    }
    if (state->shots_editor_arrow_shadow_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_arrow_shadow_check), shadow_enabled);
    }
    if (state->shots_editor_arrow_shadow_offset_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_shadow_offset_scale), shadow_offset);
    }
    if (state->shots_editor_arrow_curve_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_arrow_curve_check), curved);
    }
    if (state->shots_editor_arrow_curve_bend_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_curve_bend_scale), curve_bend);
    }
    state->shots_editor_style_syncing = FALSE;
    shots_editor_update_arrow_curve_controls_sensitivity(state);
}

static void shots_editor_sync_controls_from_selected_mark(AppState *state) {
    ShotMark *mark = shots_editor_get_selected_mark(state);
    const gchar *style_id = "bold";
    gdouble text_size = 24.0;
    gdouble text_stroke_width = 2.0;
    gdouble rect_fill_opacity = 0.18;

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
    if (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_RECT) {
        gboolean text_fill = mark->text_fill_enabled || !mark->text_stroke_enabled;
        gboolean text_stroke = mark->text_stroke_enabled;
        gboolean text_shadow = mark->text_shadow_enabled || (mark->text_size <= 0.0 && mark->text_style == 0);
        text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;
        text_stroke_width = mark->text_stroke_width > 0.0 ? mark->text_stroke_width : 2.0;
        if (state->shots_editor_font_size_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_font_size_scale), text_size);
        }
        if (state->shots_editor_text_stroke_width_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_text_stroke_width_scale), text_stroke_width);
        }

        switch (mark->text_style) {
            case SHOT_TEXT_STYLE_NORMAL:
                style_id = "normal";
                break;
            case SHOT_TEXT_STYLE_ITALIC:
                style_id = "italic";
                break;
            case SHOT_TEXT_STYLE_BOLD_ITALIC:
                style_id = "bolditalic";
                break;
            case SHOT_TEXT_STYLE_BOLD:
            default:
                style_id = "bold";
                break;
        }
        if (state->shots_editor_font_style_combo) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_font_style_combo), style_id);
        }
        if (state->shots_editor_text_fill_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_fill_check),
                                         text_fill);
        }
        if (state->shots_editor_text_stroke_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_stroke_check),
                                         text_stroke);
        }
        if (state->shots_editor_text_shadow_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_shadow_check),
                                         text_shadow);
        }
    }
    if (mark->type == SHOT_MARK_RECT) {
        gboolean rect_fill = mark->rect_fill_enabled || !mark->rect_stroke_enabled;
        gboolean rect_stroke = mark->rect_stroke_enabled || !mark->rect_fill_enabled;
        gboolean rect_shadow = mark->rect_shadow_enabled || (mark->rect_fill_opacity <= 0.0);
        const gchar *rect_style_id = shots_editor_rect_style_to_id(mark->rect_style);
        rect_fill_opacity = mark->rect_fill_opacity > 0.0 ? mark->rect_fill_opacity : 0.18;
        if (state->shots_editor_rect_style_combo) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_rect_style_combo), rect_style_id);
        }
        if (state->shots_editor_rect_fill_opacity_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_rect_fill_opacity_scale), rect_fill_opacity);
        }
        if (state->shots_editor_rect_fill_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_fill_check),
                                         rect_fill);
        }
        if (state->shots_editor_rect_stroke_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_stroke_check),
                                         rect_stroke);
        }
        if (state->shots_editor_rect_shadow_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_shadow_check),
                                         rect_shadow);
        }
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
    shots_editor_update_properties_visibility(state);
    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
}

static gboolean shots_editor_mark_supports_inline_text(const ShotMark *mark) {
    return mark && (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_RECT);
}

static gdouble shots_editor_rect_tail_width_for_style(guint rect_style, gdouble width) {
    gdouble tail = 0.0;

    if (!(rect_style == SHOT_RECT_STYLE_CALLOUT_LEFT || rect_style == SHOT_RECT_STYLE_CALLOUT_RIGHT)) {
        return 0.0;
    }

    tail = CLAMP(width * 0.18, 12.0, 48.0);
    if (tail > width - 24.0) {
        tail = MAX(0.0, width - 24.0);
    }
    return tail;
}

static gdouble shots_editor_rect_text_inset_x(const ShotMark *mark, gdouble width) {
    gdouble inset = 10.0;

    if (mark && mark->type == SHOT_MARK_RECT && mark->rect_style == SHOT_RECT_STYLE_CALLOUT_LEFT) {
        inset += shots_editor_rect_tail_width_for_style(mark->rect_style, width);
    }
    return inset;
}

static void shots_editor_autosize_rect_for_text(ShotMark *mark) {
    const gchar *text = NULL;
    gchar **lines = NULL;
    gsize max_chars = 0;
    gsize line_count = 0;
    gdouble text_size = 24.0;
    gdouble need_w = 0.0;
    gdouble need_h = 0.0;
    gdouble left = 0.0;
    gdouble right = 0.0;
    gdouble top = 0.0;
    gdouble bottom = 0.0;
    gdouble width = 0.0;
    gdouble height = 0.0;

    if (!mark || mark->type != SHOT_MARK_RECT) {
        return;
    }

    text = (mark->text && *mark->text) ? mark->text : "Callout";
    text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;

    lines = g_strsplit(text, "\n", -1);
    for (gsize i = 0; lines && lines[i]; ++i) {
        gsize chars = (gsize)g_utf8_strlen(lines[i], -1);
        if (chars > max_chars) {
            max_chars = chars;
        }
        line_count++;
    }
    if (line_count == 0) {
        line_count = 1;
    }
    if (max_chars == 0) {
        max_chars = 4;
    }

    need_w = (gdouble)max_chars * (text_size * 0.62) + 24.0;
    if (mark->rect_style == SHOT_RECT_STYLE_CALLOUT_LEFT || mark->rect_style == SHOT_RECT_STYLE_CALLOUT_RIGHT) {
        need_w += shots_editor_rect_tail_width_for_style(mark->rect_style, need_w + 24.0);
    }
    need_h = (gdouble)line_count * (text_size + 4.0) + 20.0;

    left = MIN(mark->x1, mark->x2);
    right = MAX(mark->x1, mark->x2);
    top = MIN(mark->y1, mark->y2);
    bottom = MAX(mark->y1, mark->y2);
    width = right - left;
    height = bottom - top;

    if (width < need_w) {
        right = left + need_w;
    }
    if (height < need_h) {
        bottom = top + need_h;
    }

    mark->x1 = left;
    mark->y1 = top;
    mark->x2 = right;
    mark->y2 = bottom;
    g_strfreev(lines);
}

static gboolean shots_editor_get_mark_text_anchor_widget_xy(AppState *state,
                                                            const ShotMark *mark,
                                                            gint *x_out,
                                                            gint *y_out) {
    gdouble image_x = 0.0;
    gdouble image_y = 0.0;
    gdouble widget_x = 0.0;
    gdouble widget_y = 0.0;

    if (!state || !mark || !state->shots_editor_pixbuf || state->shots_editor_draw_scale <= 0.0) {
        return FALSE;
    }

    if (mark->type == SHOT_MARK_TEXT) {
        gdouble text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;
        image_x = mark->x1;
        image_y = mark->y1 - text_size - 4.0;
    } else if (mark->type == SHOT_MARK_RECT) {
        gdouble left = MIN(mark->x1, mark->x2);
        gdouble right = MAX(mark->x1, mark->x2);
        image_x = left + shots_editor_rect_text_inset_x(mark, right - left);
        image_y = MIN(mark->y1, mark->y2) + 6.0;
    } else {
        return FALSE;
    }

    widget_x = state->shots_editor_draw_offset_x + image_x * state->shots_editor_draw_scale;
    widget_y = state->shots_editor_draw_offset_y + image_y * state->shots_editor_draw_scale;

    if (x_out) {
        *x_out = (gint)MAX(0.0, floor(widget_x));
    }
    if (y_out) {
        *y_out = (gint)MAX(0.0, floor(widget_y));
    }
    return TRUE;
}

static void shots_editor_apply_text_value_to_mark(AppState *state, ShotMark *mark, const gchar *raw_text) {
    gchar *text = g_strdup(raw_text ? raw_text : "");

    if (!state || !mark) {
        g_free(text);
        return;
    }
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

    if (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_RECT) {
        shots_editor_apply_text_style_to_mark(state, mark);
    }
    if (mark->type == SHOT_MARK_RECT) {
        shots_editor_apply_rect_style_to_mark(state, mark);
        shots_editor_autosize_rect_for_text(mark);
    }
}

static void shots_editor_inline_reposition(AppState *state) {
    ShotMark *mark = NULL;
    gint x = 0;
    gint y = 0;
    gint width = 280;

    if (!state || !state->shots_editor_inline_active || !state->shots_editor_inline_entry) {
        return;
    }

    shots_editor_ensure_marks(state);
    if (state->shots_editor_inline_mark_index < 0 ||
        (guint)state->shots_editor_inline_mark_index >= state->shots_editor_marks->len) {
        shots_editor_inline_hide(state, FALSE);
        return;
    }

    mark = g_ptr_array_index(state->shots_editor_marks, (guint)state->shots_editor_inline_mark_index);
    if (!shots_editor_mark_supports_inline_text(mark)) {
        shots_editor_inline_hide(state, FALSE);
        return;
    }

    if (!shots_editor_get_mark_text_anchor_widget_xy(state, mark, &x, &y)) {
        return;
    }

    if (state->shots_editor_canvas) {
        GtkAllocation alloc = {0};
        gtk_widget_get_allocation(state->shots_editor_canvas, &alloc);
        if (x + width > alloc.width - 8) {
            width = MAX(140, alloc.width - x - 8);
        }
    }

    gtk_widget_set_halign(state->shots_editor_inline_entry, GTK_ALIGN_START);
    gtk_widget_set_valign(state->shots_editor_inline_entry, GTK_ALIGN_START);
    gtk_widget_set_margin_start(state->shots_editor_inline_entry, x);
    gtk_widget_set_margin_top(state->shots_editor_inline_entry, y);
    gtk_widget_set_size_request(state->shots_editor_inline_entry, width, -1);
}

static void shots_editor_inline_begin(AppState *state, gint mark_index) {
    ShotMark *mark = NULL;

    if (!state || !state->shots_editor_inline_entry) {
        return;
    }

    shots_editor_ensure_marks(state);
    if (mark_index < 0 || (guint)mark_index >= state->shots_editor_marks->len) {
        shots_editor_inline_hide(state, FALSE);
        return;
    }

    mark = g_ptr_array_index(state->shots_editor_marks, (guint)mark_index);
    if (!shots_editor_mark_supports_inline_text(mark)) {
        shots_editor_inline_hide(state, FALSE);
        return;
    }

    state->shots_editor_inline_mark_index = mark_index;
    state->shots_editor_inline_active = TRUE;
    gtk_entry_set_text(GTK_ENTRY(state->shots_editor_inline_entry), mark->text ? mark->text : "");
    gtk_widget_show(state->shots_editor_inline_entry);
    shots_editor_inline_reposition(state);
    gtk_widget_grab_focus(state->shots_editor_inline_entry);
    gtk_editable_set_position(GTK_EDITABLE(state->shots_editor_inline_entry), -1);
}

static void shots_editor_inline_hide(AppState *state, gboolean apply_changes) {
    ShotMark *mark = NULL;

    if (!state || !state->shots_editor_inline_entry || !state->shots_editor_inline_active) {
        return;
    }

    if (apply_changes) {
        shots_editor_ensure_marks(state);
        if (state->shots_editor_inline_mark_index >= 0 &&
            (guint)state->shots_editor_inline_mark_index < state->shots_editor_marks->len) {
            mark = g_ptr_array_index(state->shots_editor_marks, (guint)state->shots_editor_inline_mark_index);
            if (mark) {
                const gchar *text = gtk_entry_get_text(GTK_ENTRY(state->shots_editor_inline_entry));
                shots_editor_apply_text_value_to_mark(state, mark, text);
                if (state->shots_editor_text_entry) {
                    gtk_entry_set_text(GTK_ENTRY(state->shots_editor_text_entry), mark->text ? mark->text : "");
                }
            }
        }
    }

    state->shots_editor_inline_active = FALSE;
    state->shots_editor_inline_mark_index = -1;
    gtk_widget_hide(state->shots_editor_inline_entry);
    if (state->shots_editor_canvas) {
        gtk_widget_grab_focus(state->shots_editor_canvas);
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
}

static gint shots_editor_find_last_stamp_index(AppState *state) {
    if (!state) {
        return -1;
    }

    shots_editor_ensure_marks(state);
    for (gint i = (gint)state->shots_editor_marks->len - 1; i >= 0; --i) {
        ShotMark *mark = g_ptr_array_index(state->shots_editor_marks, (guint)i);
        if (mark && mark->type == SHOT_MARK_STAMP) {
            return i;
        }
    }
    return -1;
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
    shots_editor_inline_hide(state, FALSE);
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

static const gchar *shots_editor_get_quick_style_category(AppState *state) {
    const gchar *tool = NULL;
    ShotMark *mark = NULL;

    if (!state) {
        return NULL;
    }

    tool = shots_editor_get_active_tool(state);
    if (tool && g_strcmp0(tool, "select") != 0) {
        if (g_strcmp0(tool, "arrow") == 0) {
            return "arrow";
        }
        if (g_strcmp0(tool, "rect") == 0) {
            return "callout";
        }
        if (g_strcmp0(tool, "stamp") == 0) {
            return "step";
        }
        if (g_strcmp0(tool, "text") == 0) {
            return "text";
        }
        return NULL;
    }

    mark = shots_editor_get_selected_mark(state);
    if (!mark) {
        return NULL;
    }
    if (mark->type == SHOT_MARK_ARROW) {
        return "arrow";
    }
    if (mark->type == SHOT_MARK_RECT) {
        return "callout";
    }
    if (mark->type == SHOT_MARK_STAMP) {
        return "step";
    }
    if (mark->type == SHOT_MARK_TEXT) {
        return "text";
    }
    return NULL;
}

static void shots_editor_update_quick_styles_visibility(AppState *state) {
    const gchar *category = NULL;
    GList *children = NULL;
    gboolean any_visible = FALSE;

    if (!state || !state->shots_editor_quick_styles_row) {
        return;
    }

    category = shots_editor_get_quick_style_category(state);
    children = gtk_container_get_children(GTK_CONTAINER(state->shots_editor_quick_styles_row));
    for (GList *it = children; it; it = it->next) {
        GtkWidget *child = GTK_WIDGET(it->data);
        const gchar *button_category = g_object_get_data(G_OBJECT(child), "quick-style-category");
        gboolean visible = FALSE;

        if (!category || !button_category || g_strcmp0(button_category, "all") == 0) {
            visible = TRUE;
        } else {
            visible = (g_strcmp0(button_category, category) == 0);
        }
        gtk_widget_set_visible(child, visible);
        if (visible) {
            any_visible = TRUE;
        }
    }
    g_list_free(children);

    if (state->shots_editor_quick_styles_scroller) {
        gtk_widget_set_visible(state->shots_editor_quick_styles_scroller,
                               state->shots_editor_quick_styles_visible && any_visible);
    }
    if (state->shots_editor_styles_toggle_btn) {
        gtk_button_set_label(GTK_BUTTON(state->shots_editor_styles_toggle_btn),
                             state->shots_editor_quick_styles_visible ? "Hide Styles" : "Show Styles");
    }
}

static void shots_editor_update_properties_visibility(AppState *state) {
    ShotMark *mark = NULL;
    const gchar *tool = NULL;
    const gchar *hint = "Choose a tool to start annotating.";
    gboolean show_stamp = FALSE;
    gboolean show_text = FALSE;
    gboolean show_callout = FALSE;
    gboolean show_arrow = FALSE;
    gboolean show_common = FALSE;

    if (!state) {
        return;
    }

    mark = shots_editor_get_selected_mark(state);
    tool = shots_editor_get_active_tool(state);

    if (tool && g_strcmp0(tool, "select") != 0) {
        if (g_strcmp0(tool, "stamp") == 0) {
            show_stamp = TRUE;
            hint = "Step properties";
        } else if (g_strcmp0(tool, "text") == 0) {
            show_text = TRUE;
            hint = "Text properties";
        } else if (g_strcmp0(tool, "rect") == 0) {
            show_callout = TRUE;
            hint = "Callout properties";
        } else if (g_strcmp0(tool, "arrow") == 0) {
            show_arrow = TRUE;
            hint = "Arrow properties";
        }
    } else if (mark) {
        switch (mark->type) {
            case SHOT_MARK_STAMP:
                show_stamp = TRUE;
                hint = "Step properties";
                break;
            case SHOT_MARK_TEXT:
                show_text = TRUE;
                hint = "Text properties";
                break;
            case SHOT_MARK_RECT:
                show_callout = TRUE;
                hint = "Callout properties";
                break;
            case SHOT_MARK_ARROW:
                show_arrow = TRUE;
                hint = "Arrow properties";
                break;
            default:
                break;
        }
    } else {
        hint = "Select an annotation to view contextual properties.";
    }

    if (state->shots_editor_group_stamp) {
        gtk_widget_set_visible(state->shots_editor_group_stamp, show_stamp);
    }
    if (state->shots_editor_group_text) {
        gtk_widget_set_visible(state->shots_editor_group_text, show_text);
    }
    if (state->shots_editor_group_callout) {
        gtk_widget_set_visible(state->shots_editor_group_callout, show_callout);
    }
    if (state->shots_editor_group_arrow) {
        gtk_widget_set_visible(state->shots_editor_group_arrow, show_arrow);
    }
    show_common = show_stamp || show_text || show_callout || show_arrow;
    if (state->shots_editor_group_common) {
        gtk_widget_set_visible(state->shots_editor_group_common, show_common);
    }
    if (state->shots_editor_props_hint) {
        gtk_label_set_text(GTK_LABEL(state->shots_editor_props_hint), hint);
    }
    shots_editor_update_arrow_curve_controls_sensitivity(state);
    shots_editor_update_quick_styles_visibility(state);
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

static void shots_editor_cubic_bezier_point(gdouble t,
                                            gdouble x0,
                                            gdouble y0,
                                            gdouble x1,
                                            gdouble y1,
                                            gdouble x2,
                                            gdouble y2,
                                            gdouble x3,
                                            gdouble y3,
                                            gdouble *out_x,
                                            gdouble *out_y) {
    gdouble it = 1.0 - t;
    gdouble b0 = it * it * it;
    gdouble b1 = 3.0 * it * it * t;
    gdouble b2 = 3.0 * it * t * t;
    gdouble b3 = t * t * t;

    if (out_x) {
        *out_x = (b0 * x0) + (b1 * x1) + (b2 * x2) + (b3 * x3);
    }
    if (out_y) {
        *out_y = (b0 * y0) + (b1 * y1) + (b2 * y2) + (b3 * y3);
    }
}

static void shots_editor_cubic_bezier_tangent(gdouble t,
                                              gdouble x0,
                                              gdouble y0,
                                              gdouble x1,
                                              gdouble y1,
                                              gdouble x2,
                                              gdouble y2,
                                              gdouble x3,
                                              gdouble y3,
                                              gdouble *out_dx,
                                              gdouble *out_dy) {
    gdouble it = 1.0 - t;
    gdouble dx = 0.0;
    gdouble dy = 0.0;

    dx = 3.0 * it * it * (x1 - x0) +
         6.0 * it * t * (x2 - x1) +
         3.0 * t * t * (x3 - x2);
    dy = 3.0 * it * it * (y1 - y0) +
         6.0 * it * t * (y2 - y1) +
         3.0 * t * t * (y3 - y2);
    if (out_dx) {
        *out_dx = dx;
    }
    if (out_dy) {
        *out_dy = dy;
    }
}

static void shots_editor_arrow_initialize_curve(ShotMark *mark, gdouble bend) {
    gdouble dx = 0.0;
    gdouble dy = 0.0;
    gdouble dist = 0.0;
    gdouble ux = 0.0;
    gdouble uy = 0.0;
    gdouble nx = 0.0;
    gdouble ny = 0.0;
    gdouble offset = 0.0;

    if (!mark || mark->type != SHOT_MARK_ARROW) {
        return;
    }

    dx = mark->x2 - mark->x1;
    dy = mark->y2 - mark->y1;
    dist = sqrt(dx * dx + dy * dy);
    mark->arrow_curve_bend = bend;

    if (dist <= 1e-6) {
        mark->arrow_c1x = mark->x1;
        mark->arrow_c1y = mark->y1;
        mark->arrow_c2x = mark->x2;
        mark->arrow_c2y = mark->y2;
        return;
    }

    ux = dx / dist;
    uy = dy / dist;
    nx = -uy;
    ny = ux;
    offset = bend * dist * 0.35;

    mark->arrow_c1x = mark->x1 + (dx / 3.0) + (nx * offset);
    mark->arrow_c1y = mark->y1 + (dy / 3.0) + (ny * offset);
    mark->arrow_c2x = mark->x1 + (2.0 * dx / 3.0) + (nx * offset);
    mark->arrow_c2y = mark->y1 + (2.0 * dy / 3.0) + (ny * offset);
}

static void shots_editor_arrow_ensure_curve_points(ShotMark *mark) {
    gdouble coords_sum = 0.0;

    if (!mark || mark->type != SHOT_MARK_ARROW || !mark->arrow_curved) {
        return;
    }

    coords_sum = fabs(mark->arrow_c1x) +
                 fabs(mark->arrow_c1y) +
                 fabs(mark->arrow_c2x) +
                 fabs(mark->arrow_c2y);
    if (coords_sum <= 1e-6) {
        gdouble bend = (fabs(mark->arrow_curve_bend) > 1e-6) ? mark->arrow_curve_bend : 0.35;
        shots_editor_arrow_initialize_curve(mark, bend);
    }
}

static gdouble shots_editor_point_cubic_bezier_distance(gdouble px,
                                                        gdouble py,
                                                        gdouble x0,
                                                        gdouble y0,
                                                        gdouble x1,
                                                        gdouble y1,
                                                        gdouble x2,
                                                        gdouble y2,
                                                        gdouble x3,
                                                        gdouble y3) {
    const gint samples = 28;
    gdouble min_dist = G_MAXDOUBLE;
    gdouble prev_x = x0;
    gdouble prev_y = y0;

    for (gint i = 1; i <= samples; ++i) {
        gdouble t = (gdouble)i / (gdouble)samples;
        gdouble cur_x = 0.0;
        gdouble cur_y = 0.0;
        gdouble d = 0.0;

        shots_editor_cubic_bezier_point(t, x0, y0, x1, y1, x2, y2, x3, y3, &cur_x, &cur_y);
        d = shots_editor_point_segment_distance(px, py, prev_x, prev_y, cur_x, cur_y);
        if (d < min_dist) {
            min_dist = d;
        }
        prev_x = cur_x;
        prev_y = cur_y;
    }
    return min_dist;
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
                    *arrow_handle_out = SHOT_HANDLE_ARROW_END;
                }
                return i;
            }
            if (sqrt(shots_editor_dist_sq(x, y, mark->x1, mark->y1)) <= endpoint_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_ARROW_START;
                }
                return i;
            }
            if (mark->arrow_curved) {
                shots_editor_arrow_ensure_curve_points(mark);
                if (sqrt(shots_editor_dist_sq(x, y, mark->arrow_c1x, mark->arrow_c1y)) <= endpoint_threshold) {
                    if (arrow_handle_out) {
                        *arrow_handle_out = SHOT_HANDLE_ARROW_CTRL1;
                    }
                    return i;
                }
                if (sqrt(shots_editor_dist_sq(x, y, mark->arrow_c2x, mark->arrow_c2y)) <= endpoint_threshold) {
                    if (arrow_handle_out) {
                        *arrow_handle_out = SHOT_HANDLE_ARROW_CTRL2;
                    }
                    return i;
                }
                if (shots_editor_point_cubic_bezier_distance(x,
                                                             y,
                                                             mark->x1,
                                                             mark->y1,
                                                             mark->arrow_c1x,
                                                             mark->arrow_c1y,
                                                             mark->arrow_c2x,
                                                             mark->arrow_c2y,
                                                             mark->x2,
                                                             mark->y2) <= line_threshold) {
                    if (arrow_handle_out) {
                        *arrow_handle_out = SHOT_HANDLE_MOVE;
                    }
                    return i;
                }
            } else if (shots_editor_point_segment_distance(x,
                                                           y,
                                                           mark->x1,
                                                           mark->y1,
                                                           mark->x2,
                                                           mark->y2) <= line_threshold) {
                if (arrow_handle_out) {
                    *arrow_handle_out = SHOT_HANDLE_MOVE;
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
            gdouble text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;
            gdouble text_w = (text_size * 0.62) * (gdouble)MAX(1, (gint)strlen(mark->text ? mark->text : "Note"));
            gdouble text_h = text_size + 8.0;
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

static gdouble shots_editor_arrow_head_align_shift(guint align, gdouble head_len) {
    if (align == SHOT_ARROW_HEAD_ALIGN_OUTSIDE) {
        return head_len;
    }
    if (align == SHOT_ARROW_HEAD_ALIGN_CENTER) {
        return head_len * 0.5;
    }
    return 0.0;
}

static void shots_editor_set_arrow_dash(cairo_t *cr,
                                        guint theme,
                                        gdouble dash_len,
                                        gdouble dash_gap) {
    if (!cr) {
        return;
    }
    if (theme == SHOT_ARROW_THEME_DASHED) {
        gdouble dashes[] = {
            MAX(1.0, dash_len > 0.0 ? dash_len : 10.0),
            MAX(1.0, dash_gap > 0.0 ? dash_gap : 7.0)
        };
        cairo_set_dash(cr, dashes, 2, 0.0);
    } else if (theme == SHOT_ARROW_THEME_DOTTED) {
        gdouble dashes[] = {
            MAX(1.0, dash_len > 0.0 ? dash_len : 1.5),
            MAX(1.0, dash_gap > 0.0 ? dash_gap : 7.0)
        };
        cairo_set_dash(cr, dashes, 2, 0.0);
    } else {
        cairo_set_dash(cr, NULL, 0, 0.0);
    }
}

static void shots_editor_draw_arrow_pass(cairo_t *cr,
                                         gdouble x1,
                                         gdouble y1,
                                         gdouble x2,
                                         gdouble y2,
                                         gboolean curved,
                                         gdouble c1x,
                                         gdouble c1y,
                                         gdouble c2x,
                                         gdouble c2y,
                                         gdouble width,
                                         gdouble head,
                                         gdouble spread,
                                         guint theme,
                                         guint head_align,
                                         gdouble dash_len,
                                         gdouble dash_gap) {
    gdouble end_dx = x2 - x1;
    gdouble end_dy = y2 - y1;
    gdouble start_dx = end_dx;
    gdouble start_dy = end_dy;
    gdouble end_dist = 0.0;
    gdouble start_dist = 0.0;
    gdouble end_angle = 0.0;
    gdouble start_angle = 0.0;
    gdouble end_ux = 0.0;
    gdouble end_uy = 0.0;
    gdouble start_ux = 0.0;
    gdouble start_uy = 0.0;
    gdouble shift = 0.0;
    gdouble tip_end_x = 0.0;
    gdouble tip_end_y = 0.0;
    gdouble tip_start_x = 0.0;
    gdouble tip_start_y = 0.0;

    if (!cr) {
        return;
    }

    if (curved) {
        shots_editor_cubic_bezier_tangent(1.0,
                                          x1,
                                          y1,
                                          c1x,
                                          c1y,
                                          c2x,
                                          c2y,
                                          x2,
                                          y2,
                                          &end_dx,
                                          &end_dy);
        shots_editor_cubic_bezier_tangent(0.0,
                                          x1,
                                          y1,
                                          c1x,
                                          c1y,
                                          c2x,
                                          c2y,
                                          x2,
                                          y2,
                                          &start_dx,
                                          &start_dy);
    }

    end_dist = sqrt(end_dx * end_dx + end_dy * end_dy);
    start_dist = sqrt(start_dx * start_dx + start_dy * start_dy);
    if (end_dist <= 1e-6) {
        end_dx = x2 - x1;
        end_dy = y2 - y1;
        end_dist = sqrt(end_dx * end_dx + end_dy * end_dy);
    }
    if (start_dist <= 1e-6) {
        start_dx = x2 - x1;
        start_dy = y2 - y1;
        start_dist = sqrt(start_dx * start_dx + start_dy * start_dy);
    }
    if (end_dist <= 1e-6 || start_dist <= 1e-6) {
        return;
    }

    end_ux = end_dx / end_dist;
    end_uy = end_dy / end_dist;
    start_ux = start_dx / start_dist;
    start_uy = start_dy / start_dist;
    end_angle = atan2(end_dy, end_dx);
    start_angle = atan2(start_dy, start_dx);
    shift = shots_editor_arrow_head_align_shift(head_align, head);
    tip_end_x = x2 + end_ux * shift;
    tip_end_y = y2 + end_uy * shift;
    tip_start_x = x1 - start_ux * shift;
    tip_start_y = y1 - start_uy * shift;

    cairo_set_line_width(cr, width);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    shots_editor_set_arrow_dash(cr, theme, dash_len, dash_gap);
    cairo_move_to(cr, x1, y1);
    if (curved) {
        cairo_curve_to(cr, c1x, c1y, c2x, c2y, x2, y2);
    } else {
        cairo_line_to(cr, x2, y2);
    }
    cairo_stroke(cr);
    cairo_set_dash(cr, NULL, 0, 0.0);

    cairo_move_to(cr, tip_end_x, tip_end_y);
    cairo_line_to(cr,
                  tip_end_x - head * cos(end_angle - spread),
                  tip_end_y - head * sin(end_angle - spread));
    cairo_line_to(cr,
                  tip_end_x - head * cos(end_angle + spread),
                  tip_end_y - head * sin(end_angle + spread));
    cairo_close_path(cr);
    cairo_fill(cr);

    if (theme == SHOT_ARROW_THEME_DOUBLE) {
        cairo_move_to(cr, tip_start_x, tip_start_y);
        cairo_line_to(cr,
                      tip_start_x + head * cos(start_angle - spread),
                      tip_start_y + head * sin(start_angle - spread));
        cairo_line_to(cr,
                      tip_start_x + head * cos(start_angle + spread),
                      tip_start_y + head * sin(start_angle + spread));
        cairo_close_path(cr);
        cairo_fill(cr);
    }
}

static void shots_editor_draw_arrow(cairo_t *cr, const ShotMark *mark) {
    gdouble head = 20.0;
    gdouble spread = G_PI / 7.0;
    gdouble width = 6.0;
    gdouble dash_len = 10.0;
    gdouble dash_gap = 7.0;
    gdouble shadow_offset = 0.0;
    gdouble c1x = 0.0;
    gdouble c1y = 0.0;
    gdouble c2x = 0.0;
    gdouble c2y = 0.0;
    gboolean shadow_enabled = FALSE;
    gboolean curved = FALSE;
    guint theme = SHOT_ARROW_THEME_CLASSIC;
    guint head_align = SHOT_ARROW_HEAD_ALIGN_INSIDE;
    ShotMark curve_mark = {0};

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
    if (mark->arrow_dash_len > 0.0) {
        dash_len = mark->arrow_dash_len;
    }
    if (mark->arrow_dash_gap > 0.0) {
        dash_gap = mark->arrow_dash_gap;
    }
    theme = mark->arrow_theme;
    head_align = mark->arrow_head_align;
    shadow_enabled = mark->arrow_shadow_enabled;
    shadow_offset = mark->arrow_shadow_offset;
    curved = mark->arrow_curved;
    c1x = mark->arrow_c1x;
    c1y = mark->arrow_c1y;
    c2x = mark->arrow_c2x;
    c2y = mark->arrow_c2y;
    if (curved) {
        curve_mark = *mark;
        shots_editor_arrow_ensure_curve_points(&curve_mark);
        c1x = curve_mark.arrow_c1x;
        c1y = curve_mark.arrow_c1y;
        c2x = curve_mark.arrow_c2x;
        c2y = curve_mark.arrow_c2y;
    }

    if (theme == SHOT_ARROW_THEME_BOLD) {
        width *= 1.45;
        head *= 1.30;
        spread *= 1.08;
    } else if (theme == SHOT_ARROW_THEME_POINTER) {
        width *= 0.78;
        head *= 1.22;
        spread *= 0.86;
    } else if (theme == SHOT_ARROW_THEME_DOUBLE) {
        width *= 0.92;
        head *= 1.08;
        spread *= 0.94;
    }

    if (shadow_enabled && shadow_offset > 0.0) {
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.34);
        shots_editor_draw_arrow_pass(cr,
                                     mark->x1 + shadow_offset,
                                     mark->y1 + shadow_offset,
                                     mark->x2 + shadow_offset,
                                     mark->y2 + shadow_offset,
                                     curved,
                                     c1x + shadow_offset,
                                     c1y + shadow_offset,
                                     c2x + shadow_offset,
                                     c2y + shadow_offset,
                                     width,
                                     head,
                                     spread,
                                     theme,
                                     head_align,
                                     dash_len,
                                     dash_gap);
    }

    cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 1.0);
    shots_editor_draw_arrow_pass(cr,
                                 mark->x1,
                                 mark->y1,
                                 mark->x2,
                                 mark->y2,
                                 curved,
                                 c1x,
                                 c1y,
                                 c2x,
                                 c2y,
                                 width,
                                 head,
                                 spread,
                                 theme,
                                 head_align,
                                 dash_len,
                                 dash_gap);
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
    cairo_fill(cr);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20.0);
    cairo_text_extents(cr, text, &extents);
    cairo_move_to(cr,
                  mark->x1 - (extents.width / 2.0 + extents.x_bearing),
                  mark->y1 - (extents.height / 2.0 + extents.y_bearing));
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_show_text(cr, text);
}

static void shots_editor_draw_step_link(cairo_t *cr,
                                        const ShotMark *from_mark,
                                        const ShotMark *to_mark,
                                        const GdkRGBA *color) {
    gdouble dx = 0.0;
    gdouble dy = 0.0;
    gdouble dist = 0.0;
    gdouble ux = 0.0;
    gdouble uy = 0.0;
    gdouble pad = 20.0;
    gdouble start_x = 0.0;
    gdouble start_y = 0.0;
    gdouble end_x = 0.0;
    gdouble end_y = 0.0;
    gdouble angle = 0.0;
    gdouble head_len = 12.0;
    gdouble head_spread = G_PI / 7.0;
    GdkRGBA fallback = {0.95, 0.24, 0.24, 1.0};
    const GdkRGBA *line_color = color ? color : &fallback;

    if (!cr || !from_mark || !to_mark) {
        return;
    }

    dx = to_mark->x1 - from_mark->x1;
    dy = to_mark->y1 - from_mark->y1;
    dist = sqrt(dx * dx + dy * dy);
    if (dist < 2.0) {
        return;
    }

    ux = dx / dist;
    uy = dy / dist;
    pad = CLAMP(dist * 0.24, 8.0, 24.0);
    start_x = from_mark->x1 + ux * pad;
    start_y = from_mark->y1 + uy * pad;
    end_x = to_mark->x1 - ux * pad;
    end_y = to_mark->y1 - uy * pad;
    angle = atan2(end_y - start_y, end_x - start_x);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, 4.0);
    cairo_set_source_rgba(cr, line_color->red, line_color->green, line_color->blue, 0.82);
    cairo_move_to(cr, start_x, start_y);
    cairo_line_to(cr, end_x, end_y);
    cairo_stroke(cr);

    cairo_move_to(cr, end_x, end_y);
    cairo_line_to(cr,
                  end_x - head_len * cos(angle - head_spread),
                  end_y - head_len * sin(angle - head_spread));
    cairo_line_to(cr,
                  end_x - head_len * cos(angle + head_spread),
                  end_y - head_len * sin(angle + head_spread));
    cairo_close_path(cr);
    cairo_set_source_rgba(cr, line_color->red, line_color->green, line_color->blue, 0.92);
    cairo_fill(cr);
}

static void shots_editor_text_style_to_cairo(guint text_style,
                                             cairo_font_slant_t *slant_out,
                                             cairo_font_weight_t *weight_out) {
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_BOLD;

    switch (text_style) {
        case SHOT_TEXT_STYLE_NORMAL:
            slant = CAIRO_FONT_SLANT_NORMAL;
            weight = CAIRO_FONT_WEIGHT_NORMAL;
            break;
        case SHOT_TEXT_STYLE_ITALIC:
            slant = CAIRO_FONT_SLANT_ITALIC;
            weight = CAIRO_FONT_WEIGHT_NORMAL;
            break;
        case SHOT_TEXT_STYLE_BOLD_ITALIC:
            slant = CAIRO_FONT_SLANT_ITALIC;
            weight = CAIRO_FONT_WEIGHT_BOLD;
            break;
        case SHOT_TEXT_STYLE_BOLD:
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

static void shots_editor_draw_styled_text(cairo_t *cr,
                                          const ShotMark *mark,
                                          const gchar *text,
                                          gdouble x,
                                          gdouble y) {
    cairo_font_slant_t slant = CAIRO_FONT_SLANT_NORMAL;
    cairo_font_weight_t weight = CAIRO_FONT_WEIGHT_BOLD;
    gdouble text_size = 24.0;
    gdouble stroke_width = 2.0;
    gdouble fill_alpha = 1.0;
    gboolean fill_enabled = TRUE;
    gboolean stroke_enabled = FALSE;
    gboolean shadow_enabled = TRUE;

    if (!cr || !mark || !text) {
        return;
    }

    shots_editor_text_style_to_cairo(mark->text_style, &slant, &weight);
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

static void shots_editor_draw_text(cairo_t *cr, const ShotMark *mark) {
    const gchar *text = (mark && mark->text && *mark->text) ? mark->text : "Note";

    if (!cr || !mark) {
        return;
    }
    shots_editor_draw_styled_text(cr, mark, text, mark->x1, mark->y1);
}

static void shots_editor_append_rounded_rect_path(cairo_t *cr,
                                                   gdouble left,
                                                   gdouble top,
                                                   gdouble width,
                                                   gdouble height,
                                                   gdouble radius) {
    gdouble r = MIN(radius, MIN(width, height) * 0.5);

    cairo_new_path(cr);
    cairo_move_to(cr, left + r, top);
    cairo_line_to(cr, left + width - r, top);
    cairo_arc(cr, left + width - r, top + r, r, -G_PI / 2.0, 0.0);
    cairo_line_to(cr, left + width, top + height - r);
    cairo_arc(cr, left + width - r, top + height - r, r, 0.0, G_PI / 2.0);
    cairo_line_to(cr, left + r, top + height);
    cairo_arc(cr, left + r, top + height - r, r, G_PI / 2.0, G_PI);
    cairo_line_to(cr, left, top + r);
    cairo_arc(cr, left + r, top + r, r, G_PI, 3.0 * G_PI / 2.0);
    cairo_close_path(cr);
}

static void shots_editor_append_callout_left_path(cairo_t *cr,
                                                  gdouble left,
                                                  gdouble top,
                                                  gdouble width,
                                                  gdouble height) {
    gdouble tail = shots_editor_rect_tail_width_for_style(SHOT_RECT_STYLE_CALLOUT_LEFT, width);
    gdouble body_left = left + tail;
    gdouble body_width = width - tail;
    gdouble radius = CLAMP(MIN(body_width, height) * 0.12, 6.0, 22.0);
    gdouble tail_top = top + height * 0.60;
    gdouble tail_bottom = top + height * 0.88;
    gdouble tail_tip = 0.0;

    if (body_width < 24.0 || tail < 6.0) {
        shots_editor_append_rounded_rect_path(cr, left, top, width, height, 14.0);
        return;
    }

    tail_top = CLAMP(tail_top, top + radius + 4.0, top + height - radius - 12.0);
    tail_bottom = CLAMP(tail_bottom, tail_top + 8.0, top + height - radius - 4.0);
    tail_tip = (tail_top + tail_bottom) * 0.5 + 2.0;

    cairo_new_path(cr);
    cairo_move_to(cr, body_left + radius, top);
    cairo_line_to(cr, body_left + body_width - radius, top);
    cairo_arc(cr, body_left + body_width - radius, top + radius, radius, -G_PI / 2.0, 0.0);
    cairo_line_to(cr, body_left + body_width, top + height - radius);
    cairo_arc(cr, body_left + body_width - radius, top + height - radius, radius, 0.0, G_PI / 2.0);
    cairo_line_to(cr, body_left + radius, top + height);
    cairo_arc(cr, body_left + radius, top + height - radius, radius, G_PI / 2.0, G_PI);
    cairo_line_to(cr, body_left, tail_bottom);
    cairo_line_to(cr, left, tail_tip);
    cairo_line_to(cr, body_left, tail_top);
    cairo_line_to(cr, body_left, top + radius);
    cairo_arc(cr, body_left + radius, top + radius, radius, G_PI, 3.0 * G_PI / 2.0);
    cairo_close_path(cr);
}

static void shots_editor_append_callout_right_path(cairo_t *cr,
                                                   gdouble left,
                                                   gdouble top,
                                                   gdouble width,
                                                   gdouble height) {
    gdouble tail = shots_editor_rect_tail_width_for_style(SHOT_RECT_STYLE_CALLOUT_RIGHT, width);
    gdouble body_width = width - tail;
    gdouble body_right = left + body_width;
    gdouble radius = CLAMP(MIN(body_width, height) * 0.12, 6.0, 22.0);
    gdouble tail_top = top + height * 0.60;
    gdouble tail_bottom = top + height * 0.88;
    gdouble tail_tip = 0.0;

    if (body_width < 24.0 || tail < 6.0) {
        shots_editor_append_rounded_rect_path(cr, left, top, width, height, 14.0);
        return;
    }

    tail_top = CLAMP(tail_top, top + radius + 4.0, top + height - radius - 12.0);
    tail_bottom = CLAMP(tail_bottom, tail_top + 8.0, top + height - radius - 4.0);
    tail_tip = (tail_top + tail_bottom) * 0.5 + 2.0;

    cairo_new_path(cr);
    cairo_move_to(cr, left + radius, top);
    cairo_line_to(cr, body_right - radius, top);
    cairo_arc(cr, body_right - radius, top + radius, radius, -G_PI / 2.0, 0.0);
    cairo_line_to(cr, body_right, tail_top);
    cairo_line_to(cr, left + width, tail_tip);
    cairo_line_to(cr, body_right, tail_bottom);
    cairo_line_to(cr, body_right, top + height - radius);
    cairo_arc(cr, body_right - radius, top + height - radius, radius, 0.0, G_PI / 2.0);
    cairo_line_to(cr, left + radius, top + height);
    cairo_arc(cr, left + radius, top + height - radius, radius, G_PI / 2.0, G_PI);
    cairo_line_to(cr, left, top + radius);
    cairo_arc(cr, left + radius, top + radius, radius, G_PI, 3.0 * G_PI / 2.0);
    cairo_close_path(cr);
}

static void shots_editor_append_rect_style_path(cairo_t *cr,
                                                const ShotMark *mark,
                                                gdouble left,
                                                gdouble top,
                                                gdouble width,
                                                gdouble height) {
    guint rect_style = mark ? mark->rect_style : SHOT_RECT_STYLE_BOX;

    if (rect_style == SHOT_RECT_STYLE_CALLOUT_LEFT) {
        shots_editor_append_callout_left_path(cr, left, top, width, height);
        return;
    }
    if (rect_style == SHOT_RECT_STYLE_CALLOUT_RIGHT) {
        shots_editor_append_callout_right_path(cr, left, top, width, height);
        return;
    }
    if (rect_style == SHOT_RECT_STYLE_HIGHLIGHT) {
        shots_editor_append_rounded_rect_path(cr, left, top, width, height, height * 0.48);
        return;
    }
    if (rect_style == SHOT_RECT_STYLE_BOX_SHADOW) {
        shots_editor_append_rounded_rect_path(cr, left, top, width, height, 16.0);
        return;
    }
    shots_editor_append_rounded_rect_path(cr, left, top, width, height, 14.0);
}

static void shots_editor_draw_rect(cairo_t *cr, const ShotMark *mark) {
    gdouble left = 0.0;
    gdouble right = 0.0;
    gdouble top = 0.0;
    gdouble bottom = 0.0;
    gdouble width = 0.0;
    gdouble height = 0.0;
    gdouble text_x = 0.0;
    gdouble shadow_alpha = 0.28;
    gdouble shadow_offset = 4.0;
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

    if (mark->rect_style == SHOT_RECT_STYLE_BOX_SHADOW) {
        shadow_alpha = 0.36;
        shadow_offset = 6.0;
    }

    if (mark->rect_shadow_enabled) {
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, shadow_alpha);
        shots_editor_append_rect_style_path(cr, mark, left + shadow_offset, top + shadow_offset, width, height);
        cairo_fill(cr);
    }

    shots_editor_append_rect_style_path(cr, mark, left, top, width, height);
    if (mark->rect_fill_enabled) {
        gdouble fill_alpha = mark->rect_fill_opacity > 0.0 ? mark->rect_fill_opacity : 0.18;
        cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, fill_alpha);
        if (mark->rect_stroke_enabled) {
            cairo_fill_preserve(cr);
        } else {
            cairo_fill(cr);
        }
    }

    if (mark->rect_stroke_enabled) {
        cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, 1.0);
        cairo_set_line_width(cr, mark->stroke_width > 0.0 ? mark->stroke_width : 3.0);
        cairo_stroke(cr);
    }

    text = (mark->text && *mark->text) ? mark->text : "Callout";
    text_x = left + shots_editor_rect_text_inset_x(mark, width);
    shots_editor_draw_styled_text(cr, mark, text, text_x, top + 28.0);
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
        if (mark &&
            mark->type == SHOT_MARK_STAMP &&
            mark->step_link_enabled &&
            mark->step_link_from_index >= 0 &&
            (guint)mark->step_link_from_index < state->shots_editor_marks->len) {
            ShotMark *from_mark = g_ptr_array_index(state->shots_editor_marks, (guint)mark->step_link_from_index);
            if (from_mark && from_mark->type == SHOT_MARK_STAMP && from_mark != mark) {
                shots_editor_draw_step_link(cr, from_mark, mark, &mark->color);
            }
        }
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
        preview.arrow_dash_len = shots_editor_get_arrow_dash_len_control(state);
        preview.arrow_dash_gap = shots_editor_get_arrow_dash_gap_control(state);
        preview.arrow_theme = shots_editor_get_arrow_theme_control(state);
        preview.arrow_head_align = shots_editor_get_arrow_head_align_control(state);
        preview.arrow_shadow_enabled = shots_editor_check_is_active(state ? state->shots_editor_arrow_shadow_check : NULL,
                                                                    FALSE);
        preview.arrow_shadow_offset = shots_editor_get_arrow_shadow_offset_control(state);
        preview.arrow_curved = shots_editor_get_arrow_curved_control(state);
        preview.arrow_curve_bend = shots_editor_get_arrow_curve_bend_control(state);
        if (preview.type == SHOT_MARK_ARROW && preview.arrow_curved) {
            shots_editor_arrow_initialize_curve(&preview, preview.arrow_curve_bend);
        }
        preview.color = color;
        if (preview.type == SHOT_MARK_RECT && state->shots_editor_text_entry) {
            text = g_strdup(gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));
            if (text) {
                g_strstrip(text);
            }
            preview.text = (text && *text) ? text : "Callout";
            shots_editor_apply_text_style_to_mark(state, &preview);
            shots_editor_apply_rect_style_to_mark(state, &preview);
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
        gdouble c1x = mark->arrow_c1x;
        gdouble c1y = mark->arrow_c1y;
        gdouble c2x = mark->arrow_c2x;
        gdouble c2y = mark->arrow_c2y;
        gboolean curved = mark->arrow_curved;

        if (curved) {
            shots_editor_arrow_ensure_curve_points(mark);
            c1x = mark->arrow_c1x;
            c1y = mark->arrow_c1y;
            c2x = mark->arrow_c2x;
            c2y = mark->arrow_c2y;
        }

        cairo_set_line_width(cr, 2.0);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.75);
        cairo_move_to(cr, mark->x1, mark->y1);
        if (curved) {
            cairo_curve_to(cr, c1x, c1y, c2x, c2y, mark->x2, mark->y2);
        } else {
            cairo_line_to(cr, mark->x2, mark->y2);
        }
        cairo_stroke(cr);

        cairo_set_source_rgba(cr, 0.08, 0.55, 0.98, 0.95);
        cairo_arc(cr, mark->x1, mark->y1, 6.0, 0, 2 * G_PI);
        cairo_fill(cr);
        cairo_arc(cr, mark->x2, mark->y2, 6.0, 0, 2 * G_PI);
        cairo_fill(cr);
        if (curved) {
            gdouble helper_dash[] = {4.0, 4.0};
            cairo_set_line_width(cr, 1.2);
            cairo_set_dash(cr, helper_dash, 2, 0.0);
            cairo_set_source_rgba(cr, 0.74, 0.84, 0.98, 0.75);
            cairo_move_to(cr, mark->x1, mark->y1);
            cairo_line_to(cr, c1x, c1y);
            cairo_move_to(cr, mark->x2, mark->y2);
            cairo_line_to(cr, c2x, c2y);
            cairo_stroke(cr);
            cairo_set_dash(cr, NULL, 0, 0.0);

            cairo_set_source_rgba(cr, 0.98, 0.78, 0.22, 0.95);
            cairo_arc(cr, c1x, c1y, 5.0, 0, 2 * G_PI);
            cairo_fill(cr);
            cairo_arc(cr, c2x, c2y, 5.0, 0, 2 * G_PI);
            cairo_fill(cr);
        }
    } else if (mark->type == SHOT_MARK_STAMP) {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.9);
        cairo_set_line_width(cr, 2.0);
        cairo_arc(cr, mark->x1, mark->y1, 26.0, 0, 2 * G_PI);
        cairo_stroke(cr);
    } else if (mark->type == SHOT_MARK_TEXT) {
        gdouble text_size = mark->text_size > 0.0 ? mark->text_size : 24.0;
        gdouble w = (text_size * 0.62) * (gdouble)MAX(1, (gint)strlen(mark->text ? mark->text : "Note"));
        gdouble h = text_size + 8.0;
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
        shots_editor_append_rect_style_path(cr,
                                            mark,
                                            left - 2.0,
                                            top - 2.0,
                                            (right - left) + 4.0,
                                            (bottom - top) + 4.0);
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
    gdouble fit_scale = 1.0;
    gdouble zoom = 1.0;
    gdouble scale = 1.0;
    gdouble draw_w = 0.0;
    gdouble draw_h = 0.0;

    gtk_widget_get_allocation(widget, &alloc);

    cairo_set_source_rgb(cr, 0.11, 0.14, 0.18);
    cairo_paint(cr);

    if (!state || !state->shots_editor_pixbuf) {
        gdouble mid_y = (gdouble)alloc.height / 2.0;
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 20.0);
        cairo_set_source_rgba(cr, 0.88, 0.92, 0.98, 0.95);
        cairo_move_to(cr, 18.0, mid_y - 26.0);
        cairo_show_text(cr, "Start editing a screenshot");
        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, 14.0);
        cairo_set_source_rgba(cr, 0.78, 0.83, 0.90, 0.88);
        cairo_move_to(cr, 18.0, mid_y + 2.0);
        cairo_show_text(cr, "1) Capture or import  2) Select one thumbnail  3) Annotate and save");
        return FALSE;
    }

    img_w = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
    img_h = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
    fit_scale = MIN((gdouble)alloc.width / (gdouble)img_w, (gdouble)alloc.height / (gdouble)img_h);
    if (fit_scale <= 0.0) {
        fit_scale = 1.0;
    }
    zoom = (state->shots_editor_zoom_factor > 0.0) ? state->shots_editor_zoom_factor : 1.0;
    zoom = CLAMP(zoom, 0.10, 8.0);
    scale = fit_scale * zoom;
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

    if (state->shots_editor_inline_active) {
        shots_editor_inline_reposition(state);
    }

    return FALSE;
}

static gboolean on_shots_editor_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data) {
    AppState *state = user_data;
    gboolean ctrl = FALSE;
    gdouble step = 0.0;
    gdouble zoom = 1.0;
    gchar *msg = NULL;

    (void)widget;
    if (!state || !state->shots_editor_pixbuf) {
        return FALSE;
    }

    ctrl = (event->state & GDK_CONTROL_MASK) != 0;
    if (!ctrl) {
        return FALSE;
    }

    if (event->direction == GDK_SCROLL_UP) {
        step = 1.0;
    } else if (event->direction == GDK_SCROLL_DOWN) {
        step = -1.0;
    } else if (event->direction == GDK_SCROLL_SMOOTH) {
        step = (event->delta_y < 0.0) ? 1.0 : -1.0;
    } else {
        return FALSE;
    }

    zoom = state->shots_editor_zoom_factor > 0.0 ? state->shots_editor_zoom_factor : 1.0;
    if (step > 0.0) {
        zoom *= 1.10;
    } else {
        zoom /= 1.10;
    }
    zoom = CLAMP(zoom, 0.10, 8.0);
    state->shots_editor_zoom_factor = zoom;

    msg = g_strdup_printf("Canvas zoom: %.0f%% (Ctrl+Scroll, right-click canvas for color menu).", zoom * 100.0);
    shots_editor_set_status(state, msg);
    g_free(msg);

    gtk_widget_queue_draw(state->shots_editor_canvas);
    return TRUE;
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

    if (state->shots_editor_inline_active) {
        shots_editor_inline_hide(state, TRUE);
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
            if (mark &&
                event->type == GDK_2BUTTON_PRESS &&
                (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_RECT || mark->type == SHOT_MARK_STAMP)) {
                if (shots_editor_mark_supports_inline_text(mark)) {
                    shots_editor_inline_begin(state, hit_index);
                    shots_editor_set_status(state, "Inline edit active. Press Enter to apply or Escape to cancel.");
                    return TRUE;
                }
                if (mark->type == SHOT_MARK_STAMP && state->shots_editor_text_entry) {
                    gtk_entry_set_text(GTK_ENTRY(state->shots_editor_text_entry), mark->text ? mark->text : "");
                    gtk_widget_grab_focus(state->shots_editor_text_entry);
                    shots_editor_set_status(state, "Stamp text loaded. Edit it and click Apply Text.");
                    return TRUE;
                }
            }
            if (mark && arrow_handle > SHOT_HANDLE_NONE) {
                if (mark->type == SHOT_MARK_ARROW && mark->arrow_curved) {
                    shots_editor_arrow_ensure_curve_points(mark);
                }
                state->shots_editor_selected_arrow_handle = arrow_handle;
                state->shots_editor_edit_dragging = TRUE;
                state->shots_editor_edit_anchor_x = ix;
                state->shots_editor_edit_anchor_y = iy;
                state->shots_editor_edit_origin_x1 = mark->x1;
                state->shots_editor_edit_origin_y1 = mark->y1;
                state->shots_editor_edit_origin_x2 = mark->x2;
                state->shots_editor_edit_origin_y2 = mark->y2;
                state->shots_editor_edit_origin_c1x = mark->arrow_c1x;
                state->shots_editor_edit_origin_c1y = mark->arrow_c1y;
                state->shots_editor_edit_origin_c2x = mark->arrow_c2x;
                state->shots_editor_edit_origin_c2y = mark->arrow_c2y;

                if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_START) {
                    shots_editor_set_status(state, "Editing arrow start. Drag the blue handle.");
                } else if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_END) {
                    shots_editor_set_status(state, "Editing arrow head. Drag the tip handle.");
                } else if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_CTRL1) {
                    shots_editor_set_status(state, "Editing curve control #1.");
                } else if (mark->type == SHOT_MARK_ARROW && arrow_handle == SHOT_HANDLE_ARROW_CTRL2) {
                    shots_editor_set_status(state, "Editing curve control #2.");
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
        gboolean link_steps = FALSE;
        gint prev_stamp_index = -1;

        shots_editor_set_selected_mark(state, -1);
        link_steps = state->shots_editor_step_link_check &&
                     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shots_editor_step_link_check));
        if (link_steps) {
            prev_stamp_index = shots_editor_find_last_stamp_index(state);
        }
        mark->type = SHOT_MARK_STAMP;
        mark->x1 = ix;
        mark->y1 = iy;
        mark->x2 = ix;
        mark->y2 = iy;
        mark->color = color;
        mark->step_link_enabled = FALSE;
        mark->step_link_from_index = -1;
        if (link_steps && prev_stamp_index >= 0) {
            mark->step_link_enabled = TRUE;
            mark->step_link_from_index = prev_stamp_index;
        }
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
        if (mark->step_link_enabled) {
            shots_editor_set_status(state, auto_step ? "Linked auto step added." : "Linked stamp added.");
        } else {
            shots_editor_set_status(state, auto_step ? "Auto step stamp added." : "Stamp added.");
        }
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
        shots_editor_apply_text_style_to_mark(state, mark);
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
                gdouble dx = ix - state->shots_editor_edit_anchor_x;
                gdouble dy = iy - state->shots_editor_edit_anchor_y;
                mark->x1 = state->shots_editor_edit_origin_x1 + dx;
                mark->y1 = state->shots_editor_edit_origin_y1 + dy;
                if (mark->arrow_curved) {
                    mark->arrow_c1x = state->shots_editor_edit_origin_c1x + dx;
                    mark->arrow_c1y = state->shots_editor_edit_origin_c1y + dy;
                }
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_ARROW_END) {
                gdouble dx = ix - state->shots_editor_edit_anchor_x;
                gdouble dy = iy - state->shots_editor_edit_anchor_y;
                mark->x2 = state->shots_editor_edit_origin_x2 + dx;
                mark->y2 = state->shots_editor_edit_origin_y2 + dy;
                if (mark->arrow_curved) {
                    mark->arrow_c2x = state->shots_editor_edit_origin_c2x + dx;
                    mark->arrow_c2y = state->shots_editor_edit_origin_c2y + dy;
                }
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_ARROW_CTRL1) {
                mark->arrow_curved = TRUE;
                mark->arrow_c1x = ix;
                mark->arrow_c1y = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_ARROW_CTRL2) {
                mark->arrow_curved = TRUE;
                mark->arrow_c2x = ix;
                mark->arrow_c2y = iy;
            } else if (state->shots_editor_selected_arrow_handle == SHOT_HANDLE_MOVE) {
                gint width = gdk_pixbuf_get_width(state->shots_editor_pixbuf);
                gint height = gdk_pixbuf_get_height(state->shots_editor_pixbuf);
                gdouble dx = ix - state->shots_editor_edit_anchor_x;
                gdouble dy = iy - state->shots_editor_edit_anchor_y;
                gdouble min_x = MIN(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble max_x = MAX(state->shots_editor_edit_origin_x1, state->shots_editor_edit_origin_x2);
                gdouble min_y = MIN(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);
                gdouble max_y = MAX(state->shots_editor_edit_origin_y1, state->shots_editor_edit_origin_y2);
                gdouble min_dx = 0.0;
                gdouble max_dx = 0.0;
                gdouble min_dy = 0.0;
                gdouble max_dy = 0.0;

                if (mark->arrow_curved) {
                    min_x = MIN(min_x, MIN(state->shots_editor_edit_origin_c1x, state->shots_editor_edit_origin_c2x));
                    max_x = MAX(max_x, MAX(state->shots_editor_edit_origin_c1x, state->shots_editor_edit_origin_c2x));
                    min_y = MIN(min_y, MIN(state->shots_editor_edit_origin_c1y, state->shots_editor_edit_origin_c2y));
                    max_y = MAX(max_y, MAX(state->shots_editor_edit_origin_c1y, state->shots_editor_edit_origin_c2y));
                }
                min_dx = -min_x;
                max_dx = (gdouble)(width - 1) - max_x;
                min_dy = -min_y;
                max_dy = (gdouble)(height - 1) - max_y;

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
                if (mark->arrow_curved) {
                    mark->arrow_c1x = state->shots_editor_edit_origin_c1x + dx;
                    mark->arrow_c1y = state->shots_editor_edit_origin_c1y + dy;
                    mark->arrow_c2x = state->shots_editor_edit_origin_c2x + dx;
                    mark->arrow_c2y = state->shots_editor_edit_origin_c2y + dy;
                }
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
        shots_editor_apply_text_style_to_mark(state, mark);
        shots_editor_apply_rect_style_to_mark(state, mark);

        shots_editor_ensure_marks(state);
        g_ptr_array_add(state->shots_editor_marks, mark);
        shots_editor_set_selected_mark(state, (gint)state->shots_editor_marks->len - 1);
        shots_editor_set_status(state, "Callout added.");
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
    state->shots_editor_zoom_factor = 1.0;
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
    if (state->shots_editor_step_link_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_step_link_check), FALSE);
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

static void shots_editor_sync_tool_buttons(AppState *state, const gchar *tool_id) {
    const gchar *id = tool_id ? tool_id : "arrow";

    if (!state) {
        return;
    }

    state->shots_editor_tool_syncing = TRUE;
    if (state->shots_tool_btn_select) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_tool_btn_select),
                                     g_strcmp0(id, "select") == 0);
    }
    if (state->shots_tool_btn_arrow) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_tool_btn_arrow),
                                     g_strcmp0(id, "arrow") == 0);
    }
    if (state->shots_tool_btn_rect) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_tool_btn_rect),
                                     g_strcmp0(id, "rect") == 0);
    }
    if (state->shots_tool_btn_text) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_tool_btn_text),
                                     g_strcmp0(id, "text") == 0);
    }
    if (state->shots_tool_btn_stamp) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_tool_btn_stamp),
                                     g_strcmp0(id, "stamp") == 0);
    }
    state->shots_editor_tool_syncing = FALSE;
}

static void on_shots_editor_tool_button_toggled(GtkToggleButton *button, gpointer user_data) {
    AppState *state = user_data;
    const gchar *tool_id = NULL;

    if (!state || state->shots_editor_tool_syncing || !gtk_toggle_button_get_active(button)) {
        return;
    }
    if (!state->shots_editor_tool_combo) {
        return;
    }

    tool_id = g_object_get_data(G_OBJECT(button), "tool-id");
    if (!tool_id || *tool_id == '\0') {
        return;
    }

    state->shots_editor_tool_syncing = TRUE;
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_tool_combo), tool_id);
    state->shots_editor_tool_syncing = FALSE;
}

static void on_shots_editor_tool_changed(GtkComboBox *combo, gpointer user_data) {
    AppState *state = user_data;
    const gchar *id = gtk_combo_box_get_active_id(combo);

    if (!state) {
        return;
    }
    if (state->shots_editor_inline_active) {
        shots_editor_inline_hide(state, TRUE);
    }
    shots_editor_sync_tool_buttons(state, id);
    shots_editor_update_properties_visibility(state);
    if (id && g_strcmp0(id, "select") == 0) {
        shots_editor_set_status(state, "Select mode: drag annotations to move/edit. Ctrl+Scroll zooms canvas.");
    } else if (id && g_strcmp0(id, "rect") == 0) {
        shots_editor_set_status(state, "Callout mode: drag to create a styled callout.");
    } else if (id && g_strcmp0(id, "arrow") == 0) {
        shots_editor_set_status(state,
                                "Arrow mode: drag to draw. Use sidebar for width, head, dash/dot, curve, and shadow.");
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

static void on_shots_editor_text_style_control_changed(GtkWidget *widget, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    (void)widget;

    if (!state || state->shots_editor_style_syncing || !mark) {
        return;
    }

    if (mark->type == SHOT_MARK_TEXT || mark->type == SHOT_MARK_RECT) {
        shots_editor_apply_text_style_to_mark(state, mark);
    }
    if (mark->type == SHOT_MARK_RECT) {
        shots_editor_apply_rect_style_to_mark(state, mark);
        shots_editor_autosize_rect_for_text(mark);
    } else if (mark->type != SHOT_MARK_TEXT) {
        return;
    }

    gtk_widget_queue_draw(state->shots_editor_canvas);
    shots_editor_set_status(state, "Selected text/callout style updated.");
}

static void shots_editor_scale_selected_rect(AppState *state, gdouble scale_factor) {
    ShotMark *mark = shots_editor_get_selected_mark(state);
    gboolean flip_x = FALSE;
    gboolean flip_y = FALSE;
    gdouble left = 0.0;
    gdouble right = 0.0;
    gdouble top = 0.0;
    gdouble bottom = 0.0;
    gdouble center_x = 0.0;
    gdouble center_y = 0.0;
    gdouble half_w = 0.0;
    gdouble half_h = 0.0;
    gdouble max_x = 0.0;
    gdouble max_y = 0.0;

    if (!state || scale_factor <= 0.0 || !mark || mark->type != SHOT_MARK_RECT) {
        return;
    }

    flip_x = mark->x2 < mark->x1;
    flip_y = mark->y2 < mark->y1;
    left = MIN(mark->x1, mark->x2);
    right = MAX(mark->x1, mark->x2);
    top = MIN(mark->y1, mark->y2);
    bottom = MAX(mark->y1, mark->y2);
    center_x = (left + right) * 0.5;
    center_y = (top + bottom) * 0.5;
    half_w = MAX(4.0, (right - left) * 0.5 * scale_factor);
    half_h = MAX(4.0, (bottom - top) * 0.5 * scale_factor);

    left = center_x - half_w;
    right = center_x + half_w;
    top = center_y - half_h;
    bottom = center_y + half_h;

    if (state->shots_editor_pixbuf) {
        max_x = (gdouble)(gdk_pixbuf_get_width(state->shots_editor_pixbuf) - 1);
        max_y = (gdouble)(gdk_pixbuf_get_height(state->shots_editor_pixbuf) - 1);

        if (left < 0.0) {
            right -= left;
            left = 0.0;
        }
        if (right > max_x) {
            left -= (right - max_x);
            right = max_x;
        }
        if (top < 0.0) {
            bottom -= top;
            top = 0.0;
        }
        if (bottom > max_y) {
            top -= (bottom - max_y);
            bottom = max_y;
        }
        left = CLAMP(left, 0.0, max_x);
        right = CLAMP(right, 0.0, max_x);
        top = CLAMP(top, 0.0, max_y);
        bottom = CLAMP(bottom, 0.0, max_y);
    }

    if (flip_x) {
        mark->x1 = right;
        mark->x2 = left;
    } else {
        mark->x1 = left;
        mark->x2 = right;
    }
    if (flip_y) {
        mark->y1 = bottom;
        mark->y2 = top;
    } else {
        mark->y1 = top;
        mark->y2 = bottom;
    }
}

static void on_shots_editor_rect_scale_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    const gchar *factor_text = g_object_get_data(G_OBJECT(button), "scale-factor");
    gdouble factor = factor_text ? g_ascii_strtod(factor_text, NULL) : 1.0;

    if (!state) {
        return;
    }
    if (!mark || mark->type != SHOT_MARK_RECT) {
        shots_editor_set_status(state, "Select a callout first, then use scale controls.");
        return;
    }
    if (factor <= 0.0 || fabs(factor - 1.0) < 1e-5) {
        return;
    }

    shots_editor_scale_selected_rect(state, factor);
    gtk_widget_queue_draw(state->shots_editor_canvas);
    shots_editor_set_status(state, factor > 1.0 ? "Callout scaled up." : "Callout scaled down.");
}

static void on_shots_editor_inline_activate(GtkEntry *entry, gpointer user_data) {
    AppState *state = user_data;
    (void)entry;
    shots_editor_inline_hide(state, TRUE);
    shots_editor_set_status(state, "Inline text applied.");
}

static gboolean on_shots_editor_inline_focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer user_data) {
    AppState *state = user_data;
    (void)widget;
    (void)event;
    shots_editor_inline_hide(state, TRUE);
    return FALSE;
}

static gboolean on_shots_editor_inline_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    AppState *state = user_data;
    (void)widget;
    if (!state || !state->shots_editor_inline_active) {
        return FALSE;
    }
    if (event->keyval == GDK_KEY_Escape) {
        shots_editor_inline_hide(state, FALSE);
        shots_editor_set_status(state, "Inline edit canceled.");
        return TRUE;
    }
    return FALSE;
}

static void on_shots_editor_apply_text_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);

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

    shots_editor_apply_text_value_to_mark(state, mark, gtk_entry_get_text(GTK_ENTRY(state->shots_editor_text_entry)));

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

static void on_shots_editor_arrow_style_changed(GtkWidget *widget, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    (void)widget;

    if (!state || state->shots_editor_style_syncing) {
        return;
    }
    shots_editor_update_arrow_curve_controls_sensitivity(state);
    if (!mark) {
        if (state->shots_editor_canvas) {
            gtk_widget_queue_draw(state->shots_editor_canvas);
        }
        shots_editor_set_status(state, "Arrow preset updated for new arrows.");
        return;
    }
    if (mark->type != SHOT_MARK_ARROW && mark->type != SHOT_MARK_RECT) {
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

static void on_shots_editor_arrow_curve_reset_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    ShotMark *mark = shots_editor_get_selected_mark(state);
    const gdouble default_bend = 0.35;
    (void)button;

    if (!state) {
        return;
    }

    state->shots_editor_style_syncing = TRUE;
    if (state->shots_editor_arrow_curve_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_arrow_curve_check), TRUE);
    }
    if (state->shots_editor_arrow_curve_bend_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_curve_bend_scale), default_bend);
    }
    state->shots_editor_style_syncing = FALSE;
    shots_editor_update_arrow_curve_controls_sensitivity(state);

    if (mark && mark->type == SHOT_MARK_ARROW) {
        mark->arrow_curved = TRUE;
        mark->arrow_curve_bend = default_bend;
        shots_editor_arrow_initialize_curve(mark, default_bend);
        gtk_widget_queue_draw(state->shots_editor_canvas);
        shots_editor_set_status(state, "Arrow curve reset.");
    } else {
        shots_editor_set_status(state, "Curve preset reset for new arrows.");
    }
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

static void on_shots_editor_step_link_toggled(GtkToggleButton *button, gpointer user_data) {
    AppState *state = user_data;
    gboolean enabled = FALSE;

    if (!state) {
        return;
    }
    enabled = gtk_toggle_button_get_active(button);
    shots_editor_set_status(state,
                            enabled
                                ? "Step linking enabled. New steps will connect to the previous step."
                                : "Step linking disabled. New steps are independent.");
}

static void shots_editor_apply_quick_style(AppState *state, const gchar *style_id) {
    const gchar *tool_id = NULL;
    const gchar *status = "Quick style applied.";
    const gchar *color_hex = "#ef4444";
    const gchar *arrow_theme_id = NULL;
    const gchar *arrow_head_align_id = NULL;
    const gchar *text_style_id = NULL;
    gdouble arrow_width = -1.0;
    gdouble arrow_head_len = -1.0;
    gdouble arrow_head_angle = -1.0;
    gdouble arrow_dash_len = -1.0;
    gdouble arrow_dash_gap = -1.0;
    gdouble arrow_shadow_offset = -1.0;
    gdouble text_size = -1.0;
    gdouble text_stroke_width = -1.0;
    gdouble rect_fill_opacity = -1.0;
    const gchar *rect_style_id = NULL;
    gboolean set_text_style = FALSE;
    gboolean text_fill = TRUE;
    gboolean text_stroke = FALSE;
    gboolean text_shadow = TRUE;
    gboolean rect_fill = TRUE;
    gboolean rect_stroke = TRUE;
    gboolean rect_shadow = TRUE;
    gboolean set_rect_style = FALSE;
    gboolean set_auto_step = FALSE;
    gboolean set_link_step = FALSE;
    gboolean auto_step_value = TRUE;
    gboolean link_step_value = FALSE;
    gboolean set_arrow_shadow = FALSE;
    gboolean arrow_shadow_enabled = FALSE;
    GdkRGBA color = {0.95, 0.24, 0.24, 1.0};
    ShotMark *mark = NULL;
    gboolean have_color = FALSE;

    if (!state || !style_id || !*style_id) {
        return;
    }

    if (g_strcmp0(style_id, "arrow_classic") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Classic";
        color_hex = "#ef4444";
        arrow_theme_id = "classic";
        arrow_width = 6.0;
        arrow_head_len = 20.0;
        arrow_head_angle = 25.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_bold") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Bold";
        color_hex = "#f97316";
        arrow_theme_id = "bold";
        arrow_width = 9.0;
        arrow_head_len = 28.0;
        arrow_head_angle = 30.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_pointer") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Pointer";
        color_hex = "#8b5cf6";
        arrow_theme_id = "pointer";
        arrow_width = 5.0;
        arrow_head_len = 24.0;
        arrow_head_angle = 17.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_dashed") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Dashed";
        color_hex = "#06b6d4";
        arrow_theme_id = "dashed";
        arrow_width = 6.0;
        arrow_head_len = 20.0;
        arrow_head_angle = 24.0;
        arrow_dash_len = 10.0;
        arrow_dash_gap = 7.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_dotted") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Dotted";
        color_hex = "#ef4444";
        arrow_theme_id = "dotted";
        arrow_width = 6.0;
        arrow_head_len = 18.0;
        arrow_head_angle = 23.0;
        arrow_dash_len = 1.5;
        arrow_dash_gap = 7.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_double") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Double";
        color_hex = "#f97316";
        arrow_theme_id = "double";
        arrow_width = 6.0;
        arrow_head_len = 20.0;
        arrow_head_angle = 24.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "arrow_shadow") == 0) {
        tool_id = "arrow";
        status = "Quick style: Arrow Shadow";
        color_hex = "#ef4444";
        arrow_theme_id = "classic";
        arrow_width = 6.0;
        arrow_head_len = 20.0;
        arrow_head_angle = 25.0;
        set_arrow_shadow = TRUE;
        arrow_shadow_enabled = TRUE;
        arrow_shadow_offset = 5.0;
        arrow_head_align_id = "inside";
    } else if (g_strcmp0(style_id, "callout_red") == 0) {
        tool_id = "rect";
        status = "Quick style: Callout Red";
        color_hex = "#ef4444";
        set_rect_style = TRUE;
        rect_style_id = "box";
        rect_fill = TRUE;
        rect_stroke = TRUE;
        rect_shadow = TRUE;
        rect_fill_opacity = 0.16;
    } else if (g_strcmp0(style_id, "callout_green") == 0) {
        tool_id = "rect";
        status = "Quick style: Callout Green";
        color_hex = "#22c55e";
        set_rect_style = TRUE;
        rect_style_id = "callout_left";
        rect_fill = TRUE;
        rect_stroke = TRUE;
        rect_shadow = TRUE;
        rect_fill_opacity = 0.14;
    } else if (g_strcmp0(style_id, "callout_right") == 0) {
        tool_id = "rect";
        status = "Quick style: Callout Right";
        color_hex = "#14b8a6";
        set_rect_style = TRUE;
        rect_style_id = "callout_right";
        rect_fill = TRUE;
        rect_stroke = TRUE;
        rect_shadow = TRUE;
        rect_fill_opacity = 0.14;
    } else if (g_strcmp0(style_id, "callout_highlight") == 0) {
        tool_id = "rect";
        status = "Quick style: Highlight";
        color_hex = "#eab308";
        set_rect_style = TRUE;
        rect_style_id = "highlight";
        rect_fill = TRUE;
        rect_stroke = FALSE;
        rect_shadow = FALSE;
        rect_fill_opacity = 0.30;
    } else if (g_strcmp0(style_id, "callout_shadow") == 0) {
        tool_id = "rect";
        status = "Quick style: Callout Shadow";
        color_hex = "#3b82f6";
        set_rect_style = TRUE;
        rect_style_id = "box_shadow";
        rect_fill = TRUE;
        rect_stroke = TRUE;
        rect_shadow = TRUE;
        rect_fill_opacity = 0.18;
    } else if (g_strcmp0(style_id, "step_badge") == 0) {
        tool_id = "stamp";
        status = "Quick style: Step Badge";
        color_hex = "#ef4444";
        set_auto_step = TRUE;
        set_link_step = TRUE;
        auto_step_value = TRUE;
        link_step_value = FALSE;
    } else if (g_strcmp0(style_id, "step_badge_blue") == 0) {
        tool_id = "stamp";
        status = "Quick style: Step Blue";
        color_hex = "#2563eb";
        set_auto_step = TRUE;
        set_link_step = TRUE;
        auto_step_value = TRUE;
        link_step_value = FALSE;
    } else if (g_strcmp0(style_id, "step_badge_linked") == 0) {
        tool_id = "stamp";
        status = "Quick style: Step Linked";
        color_hex = "#f97316";
        set_auto_step = TRUE;
        set_link_step = TRUE;
        auto_step_value = TRUE;
        link_step_value = TRUE;
    } else if (g_strcmp0(style_id, "text_bold") == 0) {
        tool_id = "text";
        status = "Quick style: Text Bold";
        color_hex = "#ef4444";
        set_text_style = TRUE;
        text_style_id = "bold";
        text_size = 24.0;
        text_stroke_width = 2.0;
        text_fill = TRUE;
        text_stroke = FALSE;
        text_shadow = TRUE;
    } else if (g_strcmp0(style_id, "text_outline") == 0) {
        tool_id = "text";
        status = "Quick style: Text Outline";
        color_hex = "#ffffff";
        set_text_style = TRUE;
        text_style_id = "bold";
        text_size = 24.0;
        text_stroke_width = 2.5;
        text_fill = FALSE;
        text_stroke = TRUE;
        text_shadow = FALSE;
    } else {
        return;
    }

    have_color = gdk_rgba_parse(&color, color_hex);
    state->shots_editor_style_syncing = TRUE;
    if (tool_id && state->shots_editor_tool_combo) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_tool_combo), tool_id);
    }
    if (have_color && state->shots_editor_color_btn) {
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(state->shots_editor_color_btn), &color);
    }
    if (arrow_width > 0.0 && state->shots_editor_arrow_width_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_width_scale), arrow_width);
    }
    if (arrow_head_len > 0.0 && state->shots_editor_arrow_head_len_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_head_len_scale), arrow_head_len);
    }
    if (arrow_head_angle > 0.0 && state->shots_editor_arrow_head_angle_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_head_angle_scale), arrow_head_angle);
    }
    if (arrow_theme_id && state->shots_editor_arrow_theme_combo) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_theme_combo), arrow_theme_id);
    }
    if (arrow_head_align_id && state->shots_editor_arrow_head_align_combo) {
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_arrow_head_align_combo), arrow_head_align_id);
    }
    if (arrow_dash_len > 0.0 && state->shots_editor_arrow_dash_len_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_dash_len_scale), arrow_dash_len);
    }
    if (arrow_dash_gap > 0.0 && state->shots_editor_arrow_dash_gap_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_dash_gap_scale), arrow_dash_gap);
    }
    if (set_arrow_shadow && state->shots_editor_arrow_shadow_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_arrow_shadow_check), arrow_shadow_enabled);
    }
    if (arrow_shadow_offset >= 0.0 && state->shots_editor_arrow_shadow_offset_scale) {
        gtk_range_set_value(GTK_RANGE(state->shots_editor_arrow_shadow_offset_scale), arrow_shadow_offset);
    }
    if (set_text_style) {
        if (text_style_id && state->shots_editor_font_style_combo) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_font_style_combo), text_style_id);
        }
        if (text_size > 0.0 && state->shots_editor_font_size_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_font_size_scale), text_size);
        }
        if (text_stroke_width > 0.0 && state->shots_editor_text_stroke_width_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_text_stroke_width_scale), text_stroke_width);
        }
        if (state->shots_editor_text_fill_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_fill_check), text_fill);
        }
        if (state->shots_editor_text_stroke_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_stroke_check), text_stroke);
        }
        if (state->shots_editor_text_shadow_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_text_shadow_check), text_shadow);
        }
    }
    if (set_rect_style) {
        if (rect_style_id && state->shots_editor_rect_style_combo) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(state->shots_editor_rect_style_combo), rect_style_id);
        }
        if (state->shots_editor_rect_fill_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_fill_check), rect_fill);
        }
        if (state->shots_editor_rect_stroke_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_stroke_check), rect_stroke);
        }
        if (state->shots_editor_rect_shadow_check) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_rect_shadow_check), rect_shadow);
        }
        if (rect_fill_opacity >= 0.0 && state->shots_editor_rect_fill_opacity_scale) {
            gtk_range_set_value(GTK_RANGE(state->shots_editor_rect_fill_opacity_scale), rect_fill_opacity);
        }
    }
    if (set_auto_step && state->shots_editor_auto_step_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_auto_step_check), auto_step_value);
    }
    if (set_link_step && state->shots_editor_step_link_check) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shots_editor_step_link_check), link_step_value);
    }
    state->shots_editor_style_syncing = FALSE;

    mark = shots_editor_get_selected_mark(state);
    if (mark) {
        if (have_color) {
            mark->color = color;
        }
        if (mark->type == SHOT_MARK_ARROW) {
            shots_editor_apply_arrow_style_to_mark(state, mark);
        } else if (mark->type == SHOT_MARK_RECT) {
            shots_editor_apply_text_style_to_mark(state, mark);
            shots_editor_apply_rect_style_to_mark(state, mark);
            mark->stroke_width = MAX(2.0, shots_editor_get_arrow_width_control(state) * 0.5);
            shots_editor_autosize_rect_for_text(mark);
        } else if (mark->type == SHOT_MARK_TEXT) {
            shots_editor_apply_text_style_to_mark(state, mark);
        }
        if (state->shots_editor_canvas) {
            gtk_widget_queue_draw(state->shots_editor_canvas);
        }
    }

    shots_editor_update_properties_visibility(state);
    shots_editor_update_quick_styles_visibility(state);
    shots_editor_set_status(state, status);
}

static void on_shots_quick_style_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    const gchar *style_id = g_object_get_data(G_OBJECT(button), "quick-style-id");
    shots_editor_apply_quick_style(state, style_id);
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

static void on_shots_editor_toggle_styles_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!state) {
        return;
    }
    state->shots_editor_quick_styles_visible = !state->shots_editor_quick_styles_visible;
    shots_editor_update_quick_styles_visibility(state);
}

static void on_shots_editor_zoom_reset_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!state) {
        return;
    }
    state->shots_editor_zoom_factor = 1.0;
    if (state->shots_editor_canvas) {
        gtk_widget_queue_draw(state->shots_editor_canvas);
    }
    shots_editor_set_status(state, "Canvas zoom reset to Fit (100%).");
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
        gtk_label_set_text(GTK_LABEL(state->shots_selected),
                           "No images selected. Tip: Ctrl/Shift-click to multi-select.");
    } else {
        selection_text = g_strdup_printf("%u image(s) selected. Press Delete to remove.", paths->len);
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

static gchar *shots_pretty_path(const gchar *path) {
    const gchar *home = g_get_home_dir();
    gchar *with_home = NULL;
    gchar **parts = NULL;
    GPtrArray *tokens = NULL;
    GString *out = NULL;

    if (!path || !*path) {
        return g_strdup("unknown");
    }

    if (home && *home && g_str_has_prefix(path, home)) {
        const gchar *suffix = path + strlen(home);
        if (!suffix || *suffix == '\0') {
            with_home = g_strdup("~");
        } else if (*suffix == G_DIR_SEPARATOR) {
            with_home = g_strdup_printf("~%s", suffix);
        } else {
            with_home = g_strdup(path);
        }
    } else {
        with_home = g_strdup(path);
    }

    parts = g_strsplit(with_home, G_DIR_SEPARATOR_S, -1);
    tokens = g_ptr_array_new();
    for (guint i = 0; parts && parts[i]; ++i) {
        if (*parts[i] == '\0') {
            continue;
        }
        g_ptr_array_add(tokens, parts[i]);
    }

    out = g_string_new(NULL);
    if (tokens->len <= 4) {
        g_string_append(out, with_home);
    } else {
        guint start = tokens->len - 3;
        if (with_home[0] == '~') {
            g_string_append(out, "~");
        }
        g_string_append(out, " > ...");
        for (guint i = start; i < tokens->len; ++i) {
            g_string_append(out, " > ");
            g_string_append(out, (const gchar *)g_ptr_array_index(tokens, i));
        }
    }

    g_ptr_array_free(tokens, TRUE);
    g_strfreev(parts);
    g_free(with_home);
    return g_string_free(out, FALSE);
}

static gboolean shots_name_matches_query(const gchar *name, const gchar *query) {
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
    if (!q || !*q) {
        match = TRUE;
    } else {
        match = (n && strstr(n, q) != NULL);
    }
    g_free(q);
    g_free(n);
    return match;
}

static const gchar *shots_get_sort_mode(AppState *state) {
    if (!state) {
        return "newest";
    }
    if (state->shots_sort_name_btn &&
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shots_sort_name_btn))) {
        return "name";
    }
    if (state->shots_sort_type_btn &&
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shots_sort_type_btn))) {
        return "type";
    }
    return "newest";
}

static void shots_reload(AppState *state) {
    GDir *dir = NULL;
    const gchar *name = NULL;
    const gchar *query = NULL;
    const gchar *sort_mode = NULL;
    gboolean has_query = FALSE;
    GPtrArray *entries = g_ptr_array_new_with_free_func(free_image_entry);
    guint total_images = 0;

    if (state->shots_folder_label) {
        gchar *pretty = shots_pretty_path(state->shots_dir);
        gchar *folder_msg = g_strdup_printf("Folder: %s", pretty ? pretty : state->shots_dir);
        gchar *tooltip = g_strdup_printf("Launch folder: %s\nActive folder: %s",
                                         state->launch_dir ? state->launch_dir : "unknown",
                                         state->shots_dir ? state->shots_dir : "unknown");
        gtk_label_set_text(GTK_LABEL(state->shots_folder_label), folder_msg);
        gtk_widget_set_tooltip_text(state->shots_folder_label, tooltip);
        g_free(tooltip);
        g_free(pretty);
        g_free(folder_msg);
    }

    if (state->shots_search_entry) {
        query = gtk_entry_get_text(GTK_ENTRY(state->shots_search_entry));
    }
    has_query = (query && *query);
    sort_mode = shots_get_sort_mode(state);

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
        total_images++;
        if (!shots_name_matches_query(name, query)) {
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
    if (g_strcmp0(sort_mode, "name") == 0) {
        g_ptr_array_sort(entries, compare_entries_name_asc);
    } else if (g_strcmp0(sort_mode, "type") == 0) {
        g_ptr_array_sort(entries, compare_entries_type_then_name);
    } else {
        g_ptr_array_sort(entries, compare_entries_desc);
    }

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
        gchar *msg = has_query
                         ? g_strdup_printf("No images match '%s' in %s", query, state->shots_dir)
                         : g_strdup_printf("No images found in: %s", state->shots_dir);
        gtk_label_set_text(GTK_LABEL(state->shots_status), msg);
        g_free(msg);
    } else {
        gchar *msg = has_query
                         ? g_strdup_printf("%u/%u image(s) shown from %s",
                                           entries->len, total_images, state->shots_dir)
                         : g_strdup_printf("%u image(s) loaded from %s", entries->len, state->shots_dir);
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

static void shots_set_top_compact(AppState *state, gboolean compact) {
    if (!state) {
        return;
    }
    state->shots_top_compact = compact;
    if (state->shots_quick_start_box) {
        gtk_widget_set_visible(state->shots_quick_start_box, !compact);
    }
    if (state->shots_top_compact_toggle_btn) {
        gtk_button_set_label(GTK_BUTTON(state->shots_top_compact_toggle_btn),
                             compact ? "Expand Top" : "Compact Top");
    }
}

static void shots_set_browser_shell_visible(AppState *state, gboolean visible) {
    if (!state) {
        return;
    }
    state->shots_browser_shell_visible = visible;
    if (state->shots_browser_shell) {
        gtk_widget_set_visible(state->shots_browser_shell, visible);
    }
    if (state->shots_browser_shell_toggle_btn) {
        gtk_button_set_label(GTK_BUTTON(state->shots_browser_shell_toggle_btn),
                             visible ? "Hide Thumbs" : "Show Thumbs");
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

static void on_shots_toggle_browser_shell_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!state) {
        return;
    }
    shots_set_browser_shell_visible(state, !state->shots_browser_shell_visible);
}

static void on_shots_toggle_more_actions_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    gboolean visible = FALSE;

    (void)button;
    if (!state || !state->shots_top_actions_revealer || !state->shots_top_actions_toggle_btn) {
        return;
    }

    visible = gtk_revealer_get_reveal_child(GTK_REVEALER(state->shots_top_actions_revealer));
    gtk_revealer_set_reveal_child(GTK_REVEALER(state->shots_top_actions_revealer), !visible);
    gtk_button_set_label(GTK_BUTTON(state->shots_top_actions_toggle_btn),
                         visible ? "More Actions" : "Hide Actions");
}

static void on_shots_toggle_top_compact_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    (void)button;

    if (!state) {
        return;
    }
    shots_set_top_compact(state, !state->shots_top_compact);
}

static void on_shots_search_changed(GtkEntry *entry, gpointer user_data) {
    (void)entry;
    shots_reload(user_data);
}

static void on_shots_sort_toggled(GtkToggleButton *button, gpointer user_data) {
    if (!gtk_toggle_button_get_active(button)) {
        return;
    }
    shots_reload(user_data);
}

static void on_shots_search_icon_press(GtkEntry *entry,
                                       GtkEntryIconPosition icon_pos,
                                       GdkEvent *event,
                                       gpointer user_data) {
    (void)event;
    (void)user_data;
    if (icon_pos == GTK_ENTRY_ICON_SECONDARY) {
        gtk_entry_set_text(entry, "");
    }
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

static void on_shots_delete_selected_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    GPtrArray *paths = NULL;
    GtkWidget *dialog = NULL;
    gint response = GTK_RESPONSE_CANCEL;
    guint removed = 0;
    guint failed = 0;
    gchar *msg = NULL;
    (void)button;

    if (!state) {
        return;
    }

    paths = shots_get_selected_paths(state);
    if (!paths || paths->len == 0) {
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Select one or more images to delete.");
        if (paths) {
            g_ptr_array_free(paths, TRUE);
        }
        return;
    }

    msg = g_strdup_printf("Delete %u selected image(s)? This cannot be undone.", paths->len);
    dialog = gtk_message_dialog_new(GTK_WINDOW(state->window),
                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_WARNING,
                                    GTK_BUTTONS_NONE,
                                    "%s",
                                    msg);
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           "Cancel", GTK_RESPONSE_CANCEL,
                           "Delete", GTK_RESPONSE_ACCEPT,
                           NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    g_free(msg);

    if (response != GTK_RESPONSE_ACCEPT) {
        g_ptr_array_free(paths, TRUE);
        gtk_label_set_text(GTK_LABEL(state->shots_status), "Delete canceled.");
        return;
    }

    for (guint i = 0; i < paths->len; ++i) {
        const gchar *path = g_ptr_array_index(paths, i);
        if (path && g_remove(path) == 0) {
            removed++;
        } else {
            failed++;
        }
    }
    g_ptr_array_free(paths, TRUE);

    shots_reload(state);
    shots_editor_load_from_selection(state, TRUE);

    msg = g_strdup_printf("Deleted %u image(s)%s.",
                          removed,
                          failed > 0 ? " (some files failed)" : "");
    gtk_label_set_text(GTK_LABEL(state->shots_status), msg);
    g_free(msg);
}

static gboolean on_shots_icon_view_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    AppState *state = user_data;
    (void)widget;

    if (!state || !event) {
        return FALSE;
    }

    if ((event->state & GDK_CONTROL_MASK) &&
        (event->keyval == GDK_KEY_a || event->keyval == GDK_KEY_A)) {
        gtk_icon_view_select_all(GTK_ICON_VIEW(state->shots_icon_view));
        shots_update_selection_ui(state);
        return TRUE;
    }

    if (event->keyval == GDK_KEY_Escape) {
        gtk_icon_view_unselect_all(GTK_ICON_VIEW(state->shots_icon_view));
        shots_update_selection_ui(state);
        return TRUE;
    }

    if (event->keyval == GDK_KEY_Delete || event->keyval == GDK_KEY_KP_Delete) {
        on_shots_delete_selected_clicked(NULL, state);
        return TRUE;
    }

    return FALSE;
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

static void shortcuts_set_status(AppState *state, const gchar *text) {
    if (!state || !state->shortcuts_status) {
        return;
    }
    set_label_text_trimmed(state->shortcuts_status, text, "Shortcut helper ready.");
}

static GtkWidget *create_shortcut_row(const gchar *keys, const gchar *action, const gchar *hint) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *keys_label = gtk_label_new(keys ? keys : "");
    GtkWidget *text_col = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *action_label = gtk_label_new(action ? action : "");
    GtkWidget *hint_label = gtk_label_new(hint ? hint : "");

    gtk_style_context_add_class(gtk_widget_get_style_context(row), "shortcut-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(keys_label), "shortcut-keychip");
    gtk_style_context_add_class(gtk_widget_get_style_context(action_label), "shortcut-action");
    gtk_style_context_add_class(gtk_widget_get_style_context(hint_label), "shortcut-hint");

    gtk_label_set_xalign(GTK_LABEL(keys_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(action_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(hint_label), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(action_label), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(hint_label), TRUE);
    gtk_widget_set_size_request(keys_label, 240, -1);
    gtk_widget_set_hexpand(text_col, TRUE);

    gtk_box_pack_start(GTK_BOX(text_col), action_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(text_col), hint_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row), keys_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(row), text_col, TRUE, TRUE, 0);
    return row;
}

static GtkWidget *create_shortcut_card(const gchar *title, const gchar *subtitle, GtkWidget **body_out) {
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *title_label = gtk_label_new(title ? title : "Shortcuts");
    GtkWidget *subtitle_label = gtk_label_new(subtitle ? subtitle : "");
    GtkWidget *body = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    gtk_style_context_add_class(gtk_widget_get_style_context(card), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "shortcut-card");
    gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "shortcut-card-title");
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle_label), "meta-info");
    gtk_container_set_border_width(GTK_CONTAINER(card), 12);

    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(subtitle_label), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(subtitle_label), TRUE);
    gtk_widget_set_hexpand(body, TRUE);

    gtk_box_pack_start(GTK_BOX(card), title_label, FALSE, FALSE, 0);
    if (subtitle && *subtitle) {
        gtk_box_pack_start(GTK_BOX(card), subtitle_label, FALSE, FALSE, 0);
    }
    gtk_box_pack_start(GTK_BOX(card), body, FALSE, FALSE, 0);

    if (body_out) {
        *body_out = body;
    }
    return card;
}

static void on_shortcuts_copy_presenter_flow_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    const gchar *flow =
        "Presenter flow quick sequence:\n"
        "1) Alt+F11  -> set anchor\n"
        "2) F11      -> dashed segment\n"
        "3) Shift+F11 -> dotted segment\n"
        "4) Mod4+F11 -> arrow segment\n"
        "5) F6       -> free draw toggle\n"
        "6) Hold Shift/Ctrl/Alt while dragging for marker/arrow/red pen tools\n"
        "7) Shift+F6 -> clear strokes\n"
        "8) Button3 drag -> eraser, Button2 drag -> fine pen\n"
        "9) F7       -> cursor spotlight";

    (void)button;
    if (!state || !state->window) {
        return;
    }
    set_clipboard_text(state->window, flow);
    shortcuts_set_status(state, "Copied presenter flow to clipboard.");
}

static void on_shortcuts_copy_install_cmd_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    const gchar *cmd =
        "sudo apt install gromit-mpx xdotool flameshot pavucontrol\n"
        "./install_gromit_profile.sh\n"
        "./build_linux_control_center.sh";

    (void)button;
    if (!state || !state->window) {
        return;
    }
    set_clipboard_text(state->window, cmd);
    shortcuts_set_status(state, "Copied install/build command block.");
}

static void on_shortcuts_open_sheet_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;
    gchar *sheet_path = NULL;
    gchar *quoted = NULL;
    gchar *cmd = NULL;
    gchar *found = NULL;

    (void)button;
    if (!state || !state->repo_root) {
        return;
    }

    sheet_path = g_build_filename(state->repo_root, "SHORTCUTS_CHEATSHEET.md", NULL);
    if (!g_file_test(sheet_path, G_FILE_TEST_EXISTS)) {
        shortcuts_set_status(state, "SHORTCUTS_CHEATSHEET.md is missing in repo root.");
        g_free(sheet_path);
        return;
    }

    found = g_find_program_in_path("xdg-open");
    if (!found) {
        shortcuts_set_status(state, "xdg-open is not available.");
        g_free(sheet_path);
        return;
    }

    quoted = g_shell_quote(sheet_path);
    cmd = g_strdup_printf("xdg-open %s", quoted);
    launch_in_background(cmd);
    shortcuts_set_status(state, "Opened SHORTCUTS_CHEATSHEET.md.");

    g_free(found);
    g_free(cmd);
    g_free(quoted);
    g_free(sheet_path);
}

static void on_shortcuts_jump_utilities_clicked(GtkButton *button, gpointer user_data) {
    AppState *state = user_data;

    (void)button;
    if (!state || !state->main_notebook || !GTK_IS_NOTEBOOK(state->main_notebook)) {
        return;
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(state->main_notebook), 2);
    shortcuts_set_status(state, "Switched to Utilities tab.");
}

static GtkWidget *build_shortcuts_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *title = gtk_label_new("Shortcut Playbook");
    GtkWidget *subtitle = gtk_label_new("Live-demo controls grouped by workflow so you can present without pausing.");
    GtkWidget *flow_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    GtkWidget *flow_line1 = gtk_label_new("Demo sequence: F7 spotlight -> Alt+F11 anchor -> F11/Shift+F11/Mod4+F11 path -> F6 free draw.");
    GtkWidget *flow_line2 = gtk_label_new("Draw mode (compat profile): Shift marker, Ctrl arrow pen, Alt red pen, Button2 fine pen, Button3 eraser. Reset: Shift+F6 clear.");
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_copy_flow = gtk_button_new_with_label("Copy Presenter Flow");
    GtkWidget *btn_copy_install = gtk_button_new_with_label("Copy Install/Build");
    GtkWidget *btn_open_sheet = gtk_button_new_with_label("Open Markdown Sheet");
    GtkWidget *btn_jump_utils = gtk_button_new_with_label("Jump To Utilities");
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *status = gtk_label_new("Shortcut helper ready.");
    GtkWidget *card = NULL;
    GtkWidget *body = NULL;

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "hero-sub");
    gtk_style_context_add_class(gtk_widget_get_style_context(flow_box), "shortcut-flow");
    gtk_style_context_add_class(gtk_widget_get_style_context(scroller), "shortcut-scroll");
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");
    gtk_style_context_add_class(gtk_widget_get_style_context(actions), "action-row");

    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_label_set_xalign(GTK_LABEL(flow_line1), 0.0);
    gtk_label_set_xalign(GTK_LABEL(flow_line2), 0.0);
    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(subtitle), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(flow_line1), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(flow_line2), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(status), TRUE);

    style_action_button(btn_copy_flow, TRUE);
    style_action_button(btn_copy_install, FALSE);
    style_action_button(btn_open_sheet, FALSE);
    style_action_button(btn_jump_utils, FALSE);

    gtk_box_pack_start(GTK_BOX(actions), btn_copy_flow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_copy_install, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_open_sheet, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_jump_utils, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(flow_box), flow_line1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(flow_box), flow_line2, FALSE, FALSE, 0);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroller), content);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_widget_set_hexpand(scroller, TRUE);

    card = create_shortcut_card("Presenter Overlay", "Fast keys for drawing and animated callouts during coding videos.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("F6", "Toggle draw mode", "When pointer feels locked in drawing mode, press F6 again."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Shift+F6", "Clear all strokes", "Use between topics to reset the canvas."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Ctrl+F6 / Ctrl+Shift+F6", "Undo / Redo stroke", "Quick correction without leaving presentation flow."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Alt+F11, F11, Shift+F11, Mod4+F11", "Anchor + dashed/dotted/arrow segments", "Creates bytebytego-style flow emphasis in real time."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Shift / Ctrl / Alt + drag", "Marker / arrow pen / red pen (compat profile)", "Install profile once via ./install_gromit_profile.sh"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Button2 / Button3 + drag", "Fine pen / Eraser", "Advanced shape tools need newer gromit + GROMIT_PROFILE_MODE=advanced"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Ctrl+Alt+F11", "Reset presenter anchor", "Resync path start before a new section."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Spotlight + Capture", "Keep focus on cursor while capturing clean visuals.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("F7", "Toggle cursor spotlight", "Best used while narrating terminal lines."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("F9 / F10", "Adjust dim amount", "Decrease or increase background dimming."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Shift+F9 / Shift+F10", "Adjust spotlight radius", "Small radius for code, larger for diagrams."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("F8 / Print", "Launch Flameshot capture", "Fast callout screenshot workflow."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Presenter Canvas", "Browser whiteboard for live diagrams, icons, and JSON scene files.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Utility Button: Presenter Canvas", "Launch full-screen drawing canvas", "Use when you want a dedicated black board instead of drawing over existing apps."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("1..9 / 0", "Select tool (select, pen, line, arrow, rect, ellipse, text, icon, eraser, pan)", "Fast tool switching while recording."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Ctrl+S / Ctrl+O / Ctrl+Z / Ctrl+Y", "Save JSON / Load JSON / Undo / Redo", "Keep reusable episode scenes and restore quickly."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Ctrl+Shift+S", "Export DSL starter from current canvas", "Send scene directly into Storyboard DSL Player timeline workflow."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("F / H / G / Delete", "Focus mode / show controls / toggle grid / remove selection", "Cleaner live demos with fewer UI distractions."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Storyboard DSL Player", "Timeline parser/player for bytebytego-style scripted animations.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Utility Button: Storyboard DSL", "Launch DSL timeline player", "Import presenter canvas JSON and generate timeline-ready animation scripts."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("CKE Train Preset / CKE Infer Preset", "Load C-Kernel-Engine animation templates", "Optimized for LLM training and decode walkthrough videos."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Space / R / F / H", "Play-pause / reset / focus / panel", "Operate playback while recording without touching the editor."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Ctrl+Enter / Ctrl+S", "Validate+Play / save DSL JSON", "Fast edit-run loop for scene scripting."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Teleprompter", "Laptop-panel script reader for ATEM workflows without OBS.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Utility Button: Teleprompter", "Launch local prompter window", "Use extended display: keep prompter on laptop panel, HDMI output stays clean."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Space / R / F / M", "Play-Pause / reset / focus / mirror", "Inside teleprompter window."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Up/Down and [ / ]", "Speed and font size adjust", "Tune pacing live while speaking."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Mouse Mapped Controls", "Side-button accelerators from AwesomeWM root/client bindings.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Button 8", "Flameshot capture", "Works from desktop and focused client windows."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Button 9", "Open Linux Control Center", "Client-aware launch keeps context aligned."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4 + Mouse Wheel", "Volume up/down", "Quick audio correction while recording."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("Workspace Shell Navigation", "Jump between repos instantly in terminal sessions.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("ws / ck / ct / ant / lu", "Direct project jump commands", "Loaded by sourcing workspace_shortcuts.sh in your shell rc."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("ws_help / ws_status", "Show command map and load status", "Good onboarding shortcut for new terminals."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("browse", "Interactive workspace browser", "Arrow keys + enter/backspace navigation."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    card = create_shortcut_card("AwesomeWM Essentials", "High-frequency window manager controls you use while presenting.", &body);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4+Return", "Open terminal", "Primary launcher during demos."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4+s", "Show built-in hotkeys popup", "Fast reminder without leaving workflow."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4+j / Mod4+k", "Focus next/previous client", "Clean camera movement across windows."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4+Tab", "Return to previous client", "Useful for back-and-forth explanation."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(body), create_shortcut_row("Mod4+Ctrl+r", "Reload AwesomeWM config", "Apply rc.lua updates immediately."), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), card, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), flow_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), actions, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), scroller, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(root), status, FALSE, FALSE, 0);

    g_signal_connect(btn_copy_flow, "clicked", G_CALLBACK(on_shortcuts_copy_presenter_flow_clicked), state);
    g_signal_connect(btn_copy_install, "clicked", G_CALLBACK(on_shortcuts_copy_install_cmd_clicked), state);
    g_signal_connect(btn_open_sheet, "clicked", G_CALLBACK(on_shortcuts_open_sheet_clicked), state);
    g_signal_connect(btn_jump_utils, "clicked", G_CALLBACK(on_shortcuts_jump_utilities_clicked), state);

    state->shortcuts_status = status;
    return root;
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
    GtkWidget *scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *grid = gtk_grid_new();
    GtkWidget *hotkey_hint = gtk_label_new(NULL);
    GtkWidget *status = gtk_label_new("Click a utility to launch it.");
    const gchar *spotlight_toggle_cmd =
        "bash -lc 'bin=\"$HOME/Workspace/LinuxUtilities/cursor_spotlight\"; "
        "spot_pat=\"(^|/)cursor_spotlight( |$)\"; "
        "if pgrep -u \"$USER\" -f \"$spot_pat\" >/dev/null 2>&1; then "
        "pkill -u \"$USER\" -f \"$spot_pat\" >/dev/null 2>&1 || true; "
        "else [ -x \"$bin\" ] || exit 1; "
        "\"$bin\" --radius 180 --dim 0.68 --fps 50 >/dev/null 2>&1 & fi'";
    const gchar *spotlight_build_cmd =
        "bash -lc 'cd \"$HOME/Workspace/LinuxUtilities\" && ./build_cursor_spotlight.sh'";
    const gchar *gromit_toggle_cmd =
        "bash -lc 'if ! command -v gromit-mpx >/dev/null 2>&1; then exit 1; fi; "
        "if ! pgrep -u \"$USER\" -x gromit-mpx >/dev/null 2>&1; then "
        "gromit-mpx --key F6 --undo-key F5 >/dev/null 2>&1 & sleep 0.25; fi; "
        "gromit-mpx -t >/dev/null 2>&1'";
    const gchar *gromit_clear_cmd =
        "bash -lc 'if pgrep -u \"$USER\" -x gromit-mpx >/dev/null 2>&1; then gromit-mpx -c >/dev/null 2>&1; fi'";
    const gchar *presenter_anchor_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/presenter_dash.sh\"; [ -x \"$script\" ] || exit 1; \"$script\" anchor'";
    const gchar *presenter_dash_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/presenter_dash.sh\"; [ -x \"$script\" ] || exit 1; \"$script\" dash'";
    const gchar *presenter_dot_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/presenter_dash.sh\"; [ -x \"$script\" ] || exit 1; \"$script\" dot'";
    const gchar *presenter_arrow_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/presenter_dash.sh\"; [ -x \"$script\" ] || exit 1; \"$script\" arrow'";
    const gchar *gromit_profile_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/install_gromit_profile.sh\"; [ -x \"$script\" ] || exit 1; \"$script\"'";
    const gchar *shortcut_sheet_cmd =
        "bash -lc 'sheet=\"$HOME/Workspace/LinuxUtilities/SHORTCUTS_CHEATSHEET.md\"; [ -f \"$sheet\" ] || exit 1; xdg-open \"$sheet\" >/dev/null 2>&1'";
    const gchar *presenter_canvas_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/launch_presenter_canvas.sh\"; [ -x \"$script\" ] || exit 1; \"$script\"'";
    const gchar *presenter_storyboard_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/launch_presenter_storyboard.sh\"; [ -x \"$script\" ] || exit 1; \"$script\"'";
    const gchar *teleprompter_cmd =
        "bash -lc 'script=\"$HOME/Workspace/LinuxUtilities/launch_teleprompter.sh\"; [ -x \"$script\" ] || exit 1; \"$script\"'";
    GtkWidget *b1 = create_utility_button("Pavucontrol", "Audio mixer and device routing", "pavucontrol", state);
    GtkWidget *b2 = create_utility_button("CC Switch", "Project/context switch helper", "cc-switch", state);
    GtkWidget *b3 = create_utility_button("Flameshot", "Capture area screenshot", "flameshot gui", state);
    GtkWidget *b4 = create_utility_button("Network", "Connection editor and details", "nm-connection-editor", state);
    GtkWidget *b5 = create_utility_button("Bluetooth", "Devices and pairing", "blueman-manager", state);
    GtkWidget *b6 = create_utility_button("Workspace", "Open LinuxUtilities folder", "xdg-open .", state);
    GtkWidget *b7 = create_utility_button("Screenshots", "Open Screenshots folder", "xdg-open Screenshots", state);
    GtkWidget *b8 = create_utility_button("Terminator", "Open terminal workspace", "terminator", state);
    GtkWidget *b9 = create_utility_button("Cursor Spotlight", "Toggle cursor highlight overlay", spotlight_toggle_cmd, state);
    GtkWidget *b10 = create_utility_button("Build Spotlight", "Compile spotlight binary", spotlight_build_cmd, state);
    GtkWidget *b11 = create_utility_button("Gromit Draw", "Toggle screen drawing mode", gromit_toggle_cmd, state);
    GtkWidget *b12 = create_utility_button("Gromit Clear", "Clear all current strokes", gromit_clear_cmd, state);
    GtkWidget *b13 = create_utility_button("Dash Anchor", "Set animated path anchor", presenter_anchor_cmd, state);
    GtkWidget *b14 = create_utility_button("Dash Segment", "Animated dashed segment to cursor", presenter_dash_cmd, state);
    GtkWidget *b15 = create_utility_button("Dot Segment", "Animated dotted segment to cursor", presenter_dot_cmd, state);
    GtkWidget *b16 = create_utility_button("Arrow Segment", "Animated arrow segment to cursor", presenter_arrow_cmd, state);
    GtkWidget *b17 = create_utility_button("Install Gromit Profile", "Install compatible draw tool profile", gromit_profile_cmd, state);
    GtkWidget *b18 = create_utility_button("Shortcut Cheat Sheet", "Open full key/mouse/shell shortcuts", shortcut_sheet_cmd, state);
    GtkWidget *b19 = create_utility_button("Teleprompter", "Open local script prompter window", teleprompter_cmd, state);
    GtkWidget *b20 = create_utility_button("Presenter Canvas", "Open live whiteboard canvas with shape + JSON tools", presenter_canvas_cmd, state);
    GtkWidget *b21 = create_utility_button("Storyboard DSL", "Open timeline parser/player for scripted animation scenes", presenter_storyboard_cmd, state);

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");

    gtk_label_set_text(GTK_LABEL(title), "Utilities");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_text(GTK_LABEL(subtitle), "Fast launch pad for daily Linux tools and overlays.");
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
    gtk_grid_attach(GTK_GRID(grid), b9, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b10, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b11, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b12, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b13, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b14, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b15, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b16, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b17, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b18, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b19, 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b20, 1, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), b21, 0, 10, 1, 1);
    gtk_widget_set_hexpand(grid, TRUE);
    gtk_widget_set_vexpand(grid, FALSE);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL);

    gtk_label_set_text(GTK_LABEL(hotkey_hint),
                       "F6 draw; compat profile: Shift marker, Ctrl arrow pen, Alt red pen, Button3 eraser; F11 dash path; F7 spotlight; F8/Print capture; Presenter Canvas, Storyboard DSL, and Teleprompter buttons launch recording aids.");
    gtk_label_set_xalign(GTK_LABEL(hotkey_hint), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(hotkey_hint), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(hotkey_hint), "meta-info");

    gtk_label_set_xalign(GTK_LABEL(status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(status), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(status), "status-pill");

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroller), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);
    gtk_box_pack_start(GTK_BOX(content), grid, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), hotkey_hint, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), status, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(scroller), content);

    gtk_box_pack_start(GTK_BOX(root), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), scroller, TRUE, TRUE, 0);

    state->utils_status = status;
    return root;
}

static GtkWidget *build_screenshots_tab(AppState *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *top_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *top_header_text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *top_header_icon = gtk_image_new_from_icon_name("camera-photo-symbolic", GTK_ICON_SIZE_LARGE_TOOLBAR);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *folder = gtk_label_new(NULL);
    GtkWidget *quick_start = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *quick_actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *quick_actions_revealer = gtk_revealer_new();
    GtkWidget *btn_quick_capture = gtk_button_new_with_label("Capture New");
    GtkWidget *btn_quick_edit = gtk_button_new_with_label("Edit Selected");
    GtkWidget *btn_quick_open = gtk_button_new_with_label("Open Folder");
    GtkWidget *btn_quick_compact = gtk_button_new_with_label("Compact Top");
    GtkWidget *btn_quick_more = gtk_button_new_with_label("More Actions");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_select_all = gtk_button_new_with_label("Select All");
    GtkWidget *btn_clear_selection = gtk_button_new_with_label("Clear Selection");
    GtkWidget *btn_toggle_shell = gtk_button_new_with_label("Show Selection Shell");
    GtkWidget *btn_import = gtk_button_new_with_label("Run Import Script");
    GtkWidget *workspace_split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *editor_shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *editor_top_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *editor_top_actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *editor_header = gtk_label_new(NULL);
    GtkWidget *tool_ribbon = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *quick_styles_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *quick_styles_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
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
    GtkWidget *qs_step_badge = NULL;
    GtkWidget *qs_step_badge_blue = NULL;
    GtkWidget *qs_step_badge_linked = NULL;
    GtkWidget *qs_text_bold = NULL;
    GtkWidget *qs_text_outline = NULL;
    GtkWidget *tool_btn_select = NULL;
    GtkWidget *tool_btn_arrow = NULL;
    GtkWidget *tool_btn_rect = NULL;
    GtkWidget *tool_btn_text = NULL;
    GtkWidget *tool_btn_stamp = NULL;
    GtkWidget *editor_main_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    GtkWidget *editor_canvas_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *editor_sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *editor_sidebar_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *editor_canvas_overlay = gtk_overlay_new();
    GtkWidget *editor_canvas = gtk_drawing_area_new();
    GtkWidget *editor_inline_entry = gtk_entry_new();
    GtkWidget *editor_path = gtk_label_new("Editing: (none)");
    GtkWidget *editor_status = gtk_label_new("Select one screenshot or click Edit Selected.");
    GtkWidget *browser_shell = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *browser_split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *browser_info_revealer = gtk_revealer_new();
    GtkWidget *browser_info_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *browser_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *thumb_filter_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *thumb_search = gtk_search_entry_new();
    GtkWidget *thumb_sort_label = gtk_label_new("Sort:");
    GtkWidget *thumb_sort_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *thumb_sort_newest = gtk_radio_button_new_with_label(NULL, "Newest");
    GtkWidget *thumb_sort_name = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(thumb_sort_newest), "Name");
    GtkWidget *thumb_sort_type = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(thumb_sort_newest), "Type");
    GtkWidget *thumbs_scroller = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *icon_view = NULL;
    GtkWidget *actions = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_copy_paths = gtk_button_new_with_label("Copy Path(s)");
    GtkWidget *btn_copy_prompt = gtk_button_new_with_label("Copy Prompt");
    GtkWidget *btn_delete_selected = gtk_button_new_with_label("Delete Selected");
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
    GtkWidget *btn_toggle_styles = gtk_button_new_with_label("Hide Styles");
    GtkWidget *btn_toggle_thumbs = gtk_button_new_with_label("Hide Thumbs");
    GtkWidget *btn_zoom_reset = gtk_button_new_with_label("Fit 100%");
    GtkWidget *props_hint = gtk_label_new("Choose a tool to start annotating.");
    GtkWidget *props_group_stamp = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_group_text = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_group_callout = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_group_arrow = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *props_group_common = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *tool_label = gtk_label_new("Tool");
    GtkWidget *tool_combo = gtk_combo_box_text_new();
    GtkWidget *stamp_label = gtk_label_new("Stamp");
    GtkWidget *stamp_combo = gtk_combo_box_text_new();
    GtkWidget *auto_step_check = gtk_check_button_new_with_label("Auto step numbering");
    GtkWidget *step_link_check = gtk_check_button_new_with_label("Link steps (default: off)");
    GtkWidget *step_label = gtk_label_new("Next step: 1");
    GtkWidget *btn_step_reset = gtk_button_new_with_label("Reset Step");
    GtkWidget *text_label = gtk_label_new("Text");
    GtkWidget *text_entry = gtk_entry_new();
    GtkWidget *btn_apply_text = gtk_button_new_with_label("Apply Text");
    GtkWidget *font_style_label = gtk_label_new("Font style");
    GtkWidget *font_style_combo = gtk_combo_box_text_new();
    GtkWidget *font_size_label = gtk_label_new("Font size");
    GtkWidget *font_size_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 10, 72, 1);
    GtkWidget *text_stroke_width_label = gtk_label_new("Text stroke width");
    GtkWidget *text_stroke_width_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 8.0, 0.5);
    GtkWidget *text_fill_check = gtk_check_button_new_with_label("Text fill");
    GtkWidget *text_stroke_check = gtk_check_button_new_with_label("Text stroke");
    GtkWidget *text_shadow_check = gtk_check_button_new_with_label("Text shadow");
    GtkWidget *rect_style_label = gtk_label_new("Callout style");
    GtkWidget *rect_style_combo = gtk_combo_box_text_new();
    GtkWidget *rect_fill_check = gtk_check_button_new_with_label("Callout fill");
    GtkWidget *rect_stroke_check = gtk_check_button_new_with_label("Callout stroke");
    GtkWidget *rect_shadow_check = gtk_check_button_new_with_label("Callout shadow");
    GtkWidget *rect_fill_opacity_label = gtk_label_new("Callout fill opacity");
    GtkWidget *rect_fill_opacity_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.05, 0.95, 0.01);
    GtkWidget *rect_scale_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *btn_rect_scale_down = gtk_button_new_with_label("Scale -10%");
    GtkWidget *btn_rect_scale_up = gtk_button_new_with_label("Scale +10%");
    GtkWidget *color_label = gtk_label_new("Color");
    GtkWidget *color_btn = gtk_color_button_new();
    GtkWidget *arrow_width_label = gtk_label_new("Arrow width");
    GtkWidget *arrow_width_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.5, 40.0, 0.5);
    GtkWidget *arrow_head_len_label = gtk_label_new("Arrow head size");
    GtkWidget *arrow_head_len_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 2.0, 120.0, 1.0);
    GtkWidget *arrow_head_angle_label = gtk_label_new("Arrow head angle");
    GtkWidget *arrow_head_angle_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 5.0, 85.0, 1.0);
    GtkWidget *arrow_theme_label = gtk_label_new("Arrow theme");
    GtkWidget *arrow_theme_combo = gtk_combo_box_text_new();
    GtkWidget *arrow_head_align_label = gtk_label_new("Arrow head position");
    GtkWidget *arrow_head_align_combo = gtk_combo_box_text_new();
    GtkWidget *arrow_dash_len_label = gtk_label_new("Dash length");
    GtkWidget *arrow_dash_len_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 40.0, 0.5);
    GtkWidget *arrow_dash_gap_label = gtk_label_new("Dash gap");
    GtkWidget *arrow_dash_gap_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 1.0, 40.0, 0.5);
    GtkWidget *arrow_curve_check = gtk_check_button_new_with_label("Curved path (Bezier)");
    GtkWidget *arrow_curve_bend_label = gtk_label_new("Curve bend");
    GtkWidget *arrow_curve_bend_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1.0, 1.0, 0.01);
    GtkWidget *btn_arrow_curve_reset = gtk_button_new_with_label("Reset Curve");
    GtkWidget *arrow_shadow_check = gtk_check_button_new_with_label("Arrow shadow");
    GtkWidget *arrow_shadow_offset_label = gtk_label_new("Arrow shadow distance");
    GtkWidget *arrow_shadow_offset_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 24, 0.5);
    GdkRGBA default_color = {0.95, 0.24, 0.24, 1.0};

    gtk_style_context_add_class(gtk_widget_get_style_context(root), "panel-root");
    gtk_style_context_add_class(gtk_widget_get_style_context(top_header), "header-compact");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_actions), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_actions), "top-primary-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "top-secondary-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(toolbar), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_shell), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_top_row), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_top_actions), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(tool_ribbon), "tool-ribbon");
    gtk_style_context_add_class(gtk_widget_get_style_context(quick_styles_row), "quick-styles-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_info), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_shell), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(thumb_filter_row), "thumb-filter-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(thumb_sort_box), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_canvas_panel), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_sidebar), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(props_group_stamp), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(props_group_text), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(props_group_callout), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(props_group_arrow), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(props_group_common), "card");
    gtk_style_context_add_class(gtk_widget_get_style_context(rect_scale_row), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(actions), "action-row");
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_main_paned), "split-pane");
    gtk_style_context_add_class(gtk_widget_get_style_context(browser_split), "split-pane");
    gtk_style_context_add_class(gtk_widget_get_style_context(workspace_split), "split-pane");

    gtk_label_set_text(GTK_LABEL(title), "Screenshot Studio");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(title), "hero-title");

    gtk_label_set_xalign(GTK_LABEL(folder), 0.0);
    gtk_label_set_single_line_mode(GTK_LABEL(folder), TRUE);
    gtk_label_set_ellipsize(GTK_LABEL(folder), PANGO_ELLIPSIZE_MIDDLE);
    gtk_style_context_add_class(gtk_widget_get_style_context(folder), "meta-info");
    gtk_widget_set_halign(top_header_icon, GTK_ALIGN_START);
    gtk_widget_set_valign(top_header_icon, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(top_header_text), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_header_text), folder, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_header), top_header_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_header), top_header_text, TRUE, TRUE, 0);
    gtk_widget_set_margin_top(thumb_filter_row, 2);
    gtk_label_set_xalign(GTK_LABEL(thumb_sort_label), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(thumb_sort_label), "meta-info");
    gtk_entry_set_placeholder_text(GTK_ENTRY(thumb_search), "Filter thumbnails by filename...");
    gtk_widget_set_hexpand(thumb_search, TRUE);
    gtk_entry_set_icon_from_icon_name(GTK_ENTRY(thumb_search),
                                      GTK_ENTRY_ICON_SECONDARY,
                                      "edit-clear-symbolic");
    gtk_entry_set_icon_tooltip_text(GTK_ENTRY(thumb_search),
                                    GTK_ENTRY_ICON_SECONDARY,
                                    "Clear filter");

    style_action_button(btn_refresh, FALSE);
    style_action_button(btn_select_all, FALSE);
    style_action_button(btn_clear_selection, FALSE);
    style_action_button(btn_toggle_shell, FALSE);
    style_action_button(btn_import, FALSE);
    style_action_button(btn_quick_capture, TRUE);
    style_action_button(btn_quick_edit, TRUE);
    style_action_button(btn_quick_open, FALSE);
    style_action_button(btn_quick_compact, FALSE);
    style_action_button(btn_quick_more, FALSE);
    style_action_button(btn_copy_paths, FALSE);
    style_action_button(btn_copy_prompt, TRUE);
    style_action_button(btn_delete_selected, FALSE);
    style_sort_chip_button(thumb_sort_newest);
    style_sort_chip_button(thumb_sort_name);
    style_sort_chip_button(thumb_sort_type);
    style_action_button(btn_capture, FALSE);
    style_action_button(btn_edit_selected, TRUE);
    style_action_button(btn_undo, FALSE);
    style_action_button(btn_clear, FALSE);
    style_action_button(btn_save, TRUE);
    style_action_button(btn_save_svg, FALSE);
    style_action_button(btn_dock_toggle, FALSE);
    style_action_button(btn_toggle_styles, FALSE);
    style_action_button(btn_toggle_thumbs, FALSE);
    style_action_button(btn_zoom_reset, FALSE);
    style_action_button(btn_step_reset, FALSE);
    style_action_button(btn_apply_text, FALSE);
    style_action_button(btn_rect_scale_down, FALSE);
    style_action_button(btn_rect_scale_up, FALSE);
    style_action_button(btn_arrow_curve_reset, FALSE);

    set_button_icon_if_available(btn_quick_capture, "camera-photo-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_quick_edit, "document-edit-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_quick_open, "document-open-folder-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_quick_compact, "view-restore-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_quick_more, "view-more-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_delete_selected, "edit-delete-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_dock_toggle, "view-dual-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_toggle_styles, "applications-graphics-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_toggle_thumbs, "view-grid-symbolic", GTK_ICON_SIZE_BUTTON);
    set_button_icon_if_available(btn_zoom_reset, "zoom-fit-best-symbolic", GTK_ICON_SIZE_BUTTON);

    tool_btn_select = create_tool_ribbon_button(NULL,
                                                "select",
                                                "input-mouse-symbolic",
                                                "Select",
                                                "Select/Edit");
    tool_btn_arrow = create_tool_ribbon_button(tool_btn_select,
                                               "arrow",
                                               "draw-line-symbolic",
                                               "Arrow",
                                               "Arrow");
    tool_btn_rect = create_tool_ribbon_button(tool_btn_select,
                                              "rect",
                                              "selection-rectangular-symbolic",
                                              "Callout",
                                              "Callout Box");
    tool_btn_text = create_tool_ribbon_button(tool_btn_select,
                                              "text",
                                              "insert-text-symbolic",
                                              "Text",
                                              "Text");
    tool_btn_stamp = create_tool_ribbon_button(tool_btn_select,
                                               "stamp",
                                               "emblem-ok-symbolic",
                                               "Step",
                                               "Step/Stamp");
    gtk_box_pack_start(GTK_BOX(tool_ribbon), tool_btn_select, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tool_ribbon), tool_btn_arrow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tool_ribbon), tool_btn_rect, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tool_ribbon), tool_btn_text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tool_ribbon), tool_btn_stamp, FALSE, FALSE, 0);

    qs_arrow_classic = create_quick_style_button(state,
                                                 "arrow_classic",
                                                 "Classic",
                                                 "assets/svg/icons/tool-arrow-classic.svg",
                                                 "arrow");
    qs_arrow_bold = create_quick_style_button(state,
                                              "arrow_bold",
                                              "Bold",
                                              "assets/svg/icons/tool-arrow-bold.svg",
                                              "arrow");
    qs_arrow_pointer = create_quick_style_button(state,
                                                 "arrow_pointer",
                                                 "Pointer",
                                                 "assets/svg/icons/tool-arrow-classic.svg",
                                                 "arrow");
    qs_arrow_dashed = create_quick_style_button(state,
                                                "arrow_dashed",
                                                "Dashed",
                                                "assets/svg/icons/tool-arrow-classic.svg",
                                                "arrow");
    qs_arrow_dotted = create_quick_style_button(state,
                                                "arrow_dotted",
                                                "Dotted",
                                                "assets/svg/icons/tool-arrow-dotted.svg",
                                                "arrow");
    qs_arrow_double = create_quick_style_button(state,
                                                "arrow_double",
                                                "Double",
                                                "assets/svg/icons/tool-arrow-double.svg",
                                                "arrow");
    qs_callout_red = create_quick_style_button(state,
                                               "callout_red",
                                               "Rounded",
                                               "assets/svg/callouts/box-rounded.svg",
                                               "callout");
    qs_callout_green = create_quick_style_button(state,
                                                 "callout_green",
                                                 "Left",
                                                 "assets/svg/callouts/callout-left.svg",
                                                 "callout");
    qs_callout_right = create_quick_style_button(state,
                                                 "callout_right",
                                                 "Right",
                                                 "assets/svg/callouts/callout-right.svg",
                                                 "callout");
    qs_callout_highlight = create_quick_style_button(state,
                                                     "callout_highlight",
                                                     "Highlight",
                                                     "assets/svg/callouts/highlight-pill.svg",
                                                     "callout");
    qs_callout_shadow = create_quick_style_button(state,
                                                  "callout_shadow",
                                                  "Shadow",
                                                  "assets/svg/callouts/box-shadow.svg",
                                                  "callout");
    qs_step_badge = create_quick_style_button(state,
                                              "step_badge",
                                              "Step Red",
                                              "assets/svg/callouts/step-badge.svg",
                                              "step");
    qs_step_badge_blue = create_quick_style_button(state,
                                                   "step_badge_blue",
                                                   "Step Blue",
                                                   "assets/svg/callouts/step-badge-blue.svg",
                                                   "step");
    qs_step_badge_linked = create_quick_style_button(state,
                                                     "step_badge_linked",
                                                     "Step Link",
                                                     "assets/svg/callouts/step-badge-link.svg",
                                                     "step");
    qs_text_bold = create_quick_style_button(state,
                                             "text_bold",
                                             "Text Bold",
                                             "assets/svg/icons/tool-text.svg",
                                             "text");
    qs_text_outline = create_quick_style_button(state,
                                                "text_outline",
                                                "Text Out",
                                                "assets/svg/icons/tool-text.svg",
                                                "text");

    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_classic, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_bold, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_pointer, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_dashed, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_dotted, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_arrow_double, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_callout_red, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_callout_green, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_callout_right, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_callout_highlight, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_callout_shadow, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_step_badge, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_step_badge_blue, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_step_badge_linked, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_text_bold, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_styles_row), qs_text_outline, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(quick_styles_scroller), quick_styles_row);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(quick_styles_scroller),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_NEVER);
    gtk_widget_set_hexpand(quick_styles_scroller, TRUE);
    gtk_widget_set_size_request(quick_styles_scroller, -1, 96);
    gtk_widget_set_tooltip_text(quick_styles_scroller,
                                "Quick Styles: shows arrow/callout/step/text presets for the current tool.");

    gtk_box_pack_start(GTK_BOX(quick_actions), btn_quick_capture, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_actions), btn_quick_edit, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_actions), btn_quick_open, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_actions), btn_quick_compact, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(quick_actions), btn_quick_more, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(top_header), quick_actions, FALSE, FALSE, 0);
    gtk_revealer_set_transition_type(GTK_REVEALER(quick_actions_revealer),
                                     GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_transition_duration(GTK_REVEALER(quick_actions_revealer), 150);
    gtk_revealer_set_reveal_child(GTK_REVEALER(quick_actions_revealer), FALSE);
    gtk_container_add(GTK_CONTAINER(quick_actions_revealer), toolbar);
    gtk_box_pack_start(GTK_BOX(quick_start), quick_actions_revealer, FALSE, FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(quick_start), 2);
    gtk_widget_set_margin_top(quick_start, 2);

    gtk_box_pack_start(GTK_BOX(toolbar), btn_refresh, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_select_all, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_clear_selection, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_toggle_shell, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(toolbar), btn_import, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(actions), btn_copy_paths, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_copy_prompt, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(actions), btn_delete_selected, FALSE, FALSE, 0);

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
    gtk_icon_view_set_row_spacing(GTK_ICON_VIEW(icon_view), 10);
    gtk_icon_view_set_column_spacing(GTK_ICON_VIEW(icon_view), 10);
    gtk_icon_view_set_item_padding(GTK_ICON_VIEW(icon_view), 6);
    gtk_icon_view_set_columns(GTK_ICON_VIEW(icon_view), 8);
    gtk_icon_view_set_selection_mode(GTK_ICON_VIEW(icon_view), GTK_SELECTION_MULTIPLE);
    gtk_widget_set_can_focus(icon_view, TRUE);
    gtk_widget_set_tooltip_text(icon_view,
                                "Ctrl/Shift-click for multi-select. Press Delete to remove selected images.");
    gtk_container_add(GTK_CONTAINER(thumbs_scroller), icon_view);
    gtk_widget_set_size_request(thumbs_scroller, -1, 180);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(thumbs_scroller),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_style_context_add_class(gtk_widget_get_style_context(thumbs_scroller), "surface");
    gtk_style_context_add_class(gtk_widget_get_style_context(icon_view), "surface");

    gtk_label_set_text(GTK_LABEL(editor_header), "Quick Editor Workspace");
    gtk_label_set_xalign(GTK_LABEL(editor_header), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_header), "hero-sub");
    gtk_widget_set_halign(editor_top_actions, GTK_ALIGN_END);
    gtk_label_set_xalign(GTK_LABEL(editor_path), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(editor_path), "meta-info");
    gtk_label_set_xalign(GTK_LABEL(editor_status), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(editor_status), TRUE);

    gtk_container_set_border_width(GTK_CONTAINER(editor_shell), 10);
    gtk_container_set_border_width(GTK_CONTAINER(editor_canvas_panel), 10);
    gtk_container_set_border_width(GTK_CONTAINER(editor_sidebar), 8);
    gtk_container_set_border_width(GTK_CONTAINER(props_group_common), 8);
    gtk_container_set_border_width(GTK_CONTAINER(props_group_stamp), 8);
    gtk_container_set_border_width(GTK_CONTAINER(props_group_text), 8);
    gtk_container_set_border_width(GTK_CONTAINER(props_group_callout), 8);
    gtk_container_set_border_width(GTK_CONTAINER(props_group_arrow), 8);

    gtk_label_set_xalign(GTK_LABEL(tool_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(props_hint), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(props_hint), TRUE);
    gtk_style_context_add_class(gtk_widget_get_style_context(props_hint), "meta-info");
    gtk_label_set_xalign(GTK_LABEL(stamp_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(step_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(text_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(font_style_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(font_size_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(text_stroke_width_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(rect_style_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(rect_fill_opacity_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(color_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_width_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_head_len_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_head_angle_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_theme_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_head_align_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_dash_len_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_dash_gap_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_curve_bend_label), 0.0);
    gtk_label_set_xalign(GTK_LABEL(arrow_shadow_offset_label), 0.0);

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "select", "Select/Edit");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "arrow", "Arrow");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "rect", "Callout Box");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "stamp", "Step/Stamp");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tool_combo), "text", "Text");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(tool_combo), "arrow");
    gtk_widget_set_no_show_all(tool_label, TRUE);
    gtk_widget_set_no_show_all(tool_combo, TRUE);
    gtk_widget_hide(tool_label);
    gtk_widget_hide(tool_combo);

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "1", "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "2", "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "3", "3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "!", "!");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "?", "?");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(stamp_combo), "OK", "OK");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(stamp_combo), "1");

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "classic", "Classic");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "bold", "Bold");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "pointer", "Pointer");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "dashed", "Dashed");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "dotted", "Dotted");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_theme_combo), "double", "Double Head");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(arrow_theme_combo), "classic");

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_head_align_combo), "inside", "Inside (tip on endpoint)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_head_align_combo), "center", "Center (half outside)");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(arrow_head_align_combo), "outside", "Outside (beyond endpoint)");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(arrow_head_align_combo), "inside");

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(font_style_combo), "normal", "Normal");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(font_style_combo), "bold", "Bold");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(font_style_combo), "italic", "Italic");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(font_style_combo), "bolditalic", "Bold Italic");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(font_style_combo), "bold");

    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rect_style_combo), "box", "Rounded Box");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rect_style_combo), "callout_left", "Callout Left");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rect_style_combo), "callout_right", "Callout Right");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rect_style_combo), "highlight", "Highlight Pill");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(rect_style_combo), "box_shadow", "Box Shadow");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(rect_style_combo), "box");

    gtk_entry_set_placeholder_text(GTK_ENTRY(text_entry), "Enter label text...");
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color_btn), &default_color);
    gtk_widget_set_tooltip_text(step_link_check,
                                "When enabled, new steps draw an arrow link from the previous step.");
    gtk_widget_set_tooltip_text(arrow_theme_combo,
                                "Arrow visual preset (classic, bold, pointer, dashed, dotted, double).");
    gtk_widget_set_tooltip_text(arrow_head_align_combo,
                                "Arrow head placement relative to the line endpoint.");
    gtk_widget_set_tooltip_text(arrow_dash_len_scale,
                                "Dash length for dashed/dotted arrow themes.");
    gtk_widget_set_tooltip_text(arrow_dash_gap_scale,
                                "Gap between dashes/dots for dashed/dotted arrow themes.");
    gtk_widget_set_tooltip_text(arrow_curve_check,
                                "Enable editable Bezier control points for curved arrows.");
    gtk_widget_set_tooltip_text(arrow_curve_bend_scale,
                                "Curve direction and intensity. Negative bends to the opposite side.");
    gtk_widget_set_tooltip_text(rect_style_combo, "Callout geometry preset.");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(thumb_sort_newest), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_step_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(step_link_check), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_fill_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_stroke_check), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(text_shadow_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_fill_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_stroke_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rect_shadow_check), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arrow_curve_check), FALSE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(arrow_shadow_check), FALSE);
    g_object_set_data_full(G_OBJECT(btn_rect_scale_down), "scale-factor", g_strdup("0.90"), g_free);
    g_object_set_data_full(G_OBJECT(btn_rect_scale_up), "scale-factor", g_strdup("1.10"), g_free);
    gtk_range_set_value(GTK_RANGE(font_size_scale), 24.0);
    gtk_range_set_value(GTK_RANGE(text_stroke_width_scale), 2.0);
    gtk_range_set_value(GTK_RANGE(rect_fill_opacity_scale), 0.18);
    gtk_range_set_value(GTK_RANGE(arrow_width_scale), 6.0);
    gtk_range_set_value(GTK_RANGE(arrow_head_len_scale), 20.0);
    gtk_range_set_value(GTK_RANGE(arrow_head_angle_scale), 25.0);
    gtk_range_set_value(GTK_RANGE(arrow_dash_len_scale), 10.0);
    gtk_range_set_value(GTK_RANGE(arrow_dash_gap_scale), 7.0);
    gtk_range_set_value(GTK_RANGE(arrow_curve_bend_scale), 0.35);
    gtk_range_set_value(GTK_RANGE(arrow_shadow_offset_scale), 3.0);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_width_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_head_len_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_head_angle_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_dash_len_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_dash_gap_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_curve_bend_scale), TRUE);
    gtk_scale_set_draw_value(GTK_SCALE(arrow_shadow_offset_scale), TRUE);
    gtk_scale_set_digits(GTK_SCALE(rect_fill_opacity_scale), 2);
    gtk_scale_set_digits(GTK_SCALE(text_stroke_width_scale), 1);
    gtk_scale_set_digits(GTK_SCALE(arrow_width_scale), 1);
    gtk_scale_set_digits(GTK_SCALE(arrow_head_len_scale), 0);
    gtk_scale_set_digits(GTK_SCALE(arrow_head_angle_scale), 0);
    gtk_scale_set_digits(GTK_SCALE(arrow_dash_len_scale), 1);
    gtk_scale_set_digits(GTK_SCALE(arrow_dash_gap_scale), 1);
    gtk_scale_set_digits(GTK_SCALE(arrow_curve_bend_scale), 2);
    gtk_scale_set_digits(GTK_SCALE(arrow_shadow_offset_scale), 1);

    gtk_box_pack_start(GTK_BOX(props_group_common), color_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_common), color_btn, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(props_group_stamp), stamp_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_stamp), stamp_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_stamp), auto_step_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_stamp), step_link_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_stamp), step_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_stamp), btn_step_reset, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(props_group_text), text_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), btn_apply_text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), font_style_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), font_style_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), font_size_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), font_size_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_stroke_width_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_stroke_width_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_fill_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_stroke_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_text), text_shadow_check, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_style_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_style_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rect_scale_row), btn_rect_scale_down, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(rect_scale_row), btn_rect_scale_up, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_scale_row, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_fill_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_stroke_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_shadow_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_fill_opacity_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_callout), rect_fill_opacity_scale, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_theme_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_theme_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_align_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_align_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_width_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_width_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_len_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_len_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_angle_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_head_angle_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_dash_len_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_dash_len_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_dash_gap_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_dash_gap_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_curve_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_curve_bend_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_curve_bend_scale, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), btn_arrow_curve_reset, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_shadow_check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_shadow_offset_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(props_group_arrow), arrow_shadow_offset_scale, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_capture, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_edit_selected, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_undo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_clear, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_save, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), btn_save_svg, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), tool_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), tool_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_hint, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_group_common, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_group_stamp, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_group_text, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_group_callout, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_sidebar), props_group_arrow, FALSE, FALSE, 0);
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
                          GDK_POINTER_MOTION_MASK |
                          GDK_SCROLL_MASK |
                          GDK_SMOOTH_SCROLL_MASK);
    gtk_widget_set_hexpand(editor_canvas_overlay, TRUE);
    gtk_widget_set_vexpand(editor_canvas_overlay, TRUE);
    gtk_container_add(GTK_CONTAINER(editor_canvas_overlay), editor_canvas);
    gtk_overlay_add_overlay(GTK_OVERLAY(editor_canvas_overlay), editor_inline_entry);
    gtk_widget_set_halign(editor_inline_entry, GTK_ALIGN_START);
    gtk_widget_set_valign(editor_inline_entry, GTK_ALIGN_START);
    gtk_entry_set_placeholder_text(GTK_ENTRY(editor_inline_entry), "Edit text...");
    gtk_widget_set_no_show_all(editor_inline_entry, TRUE);
    gtk_widget_hide(editor_inline_entry);

    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_path, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_canvas_overlay, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(editor_canvas_panel), editor_status, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(editor_top_actions), btn_dock_toggle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_top_actions), btn_toggle_styles, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_top_actions), btn_toggle_thumbs, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_top_actions), btn_zoom_reset, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_top_row), editor_header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_top_row), tool_ribbon, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(editor_top_row), editor_top_actions, FALSE, FALSE, 0);
    gtk_widget_set_hexpand(editor_top_row, TRUE);

    gtk_widget_set_hexpand(editor_main_paned, TRUE);
    gtk_widget_set_vexpand(editor_main_paned, TRUE);
    gtk_box_pack_start(GTK_BOX(editor_shell), editor_top_row, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(editor_shell), quick_styles_scroller, FALSE, FALSE, 0);
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

    gtk_box_pack_start(GTK_BOX(thumb_sort_box), thumb_sort_newest, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(thumb_sort_box), thumb_sort_name, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(thumb_sort_box), thumb_sort_type, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(thumb_filter_row), thumb_search, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(thumb_filter_row), thumb_sort_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(thumb_filter_row), thumb_sort_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(browser_shell), thumb_filter_row, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(browser_shell), browser_split, TRUE, TRUE, 0);
    gtk_widget_set_vexpand(browser_shell, FALSE);
    gtk_widget_set_size_request(browser_shell, -1, 262);
    gtk_widget_set_margin_bottom(browser_shell, 10);

    g_object_set(G_OBJECT(workspace_split), "wide-handle", TRUE, NULL);
    gtk_widget_set_hexpand(workspace_split, TRUE);
    gtk_widget_set_vexpand(workspace_split, TRUE);
    gtk_widget_set_margin_bottom(workspace_split, 8);
    gtk_paned_pack1(GTK_PANED(workspace_split), editor_shell, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(workspace_split), browser_shell, FALSE, FALSE);
    gtk_paned_set_position(GTK_PANED(workspace_split), 430);

    gtk_box_pack_start(GTK_BOX(root), top_header, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), quick_start, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), workspace_split, TRUE, TRUE, 0);

    state->shots_icon_view = icon_view;
    state->shots_status = status;
    state->shots_selected = selected;
    state->shots_paths_view = paths_view;
    state->shots_folder_label = folder;
    state->shots_top_actions_revealer = quick_actions_revealer;
    state->shots_top_actions_toggle_btn = btn_quick_more;
    state->shots_top_compact_toggle_btn = btn_quick_compact;
    state->shots_quick_start_box = quick_start;
    state->shots_top_compact = FALSE;
    state->shots_search_entry = thumb_search;
    state->shots_sort_newest_btn = thumb_sort_newest;
    state->shots_sort_name_btn = thumb_sort_name;
    state->shots_sort_type_btn = thumb_sort_type;
    state->shots_browser_info_revealer = browser_info_revealer;
    state->shots_browser_toggle_btn = btn_toggle_shell;
    state->shots_browser_split_paned = browser_split;
    state->shots_editor_canvas = editor_canvas;
    state->shots_editor_canvas_overlay = editor_canvas_overlay;
    state->shots_editor_inline_entry = editor_inline_entry;
    state->shots_editor_inline_mark_index = -1;
    state->shots_editor_inline_active = FALSE;
    state->shots_editor_status = editor_status;
    state->shots_editor_tool_combo = tool_combo;
    state->shots_tool_btn_select = tool_btn_select;
    state->shots_tool_btn_arrow = tool_btn_arrow;
    state->shots_tool_btn_rect = tool_btn_rect;
    state->shots_tool_btn_text = tool_btn_text;
    state->shots_tool_btn_stamp = tool_btn_stamp;
    state->shots_editor_stamp_combo = stamp_combo;
    state->shots_editor_text_entry = text_entry;
    state->shots_editor_color_btn = color_btn;
    state->shots_editor_font_style_combo = font_style_combo;
    state->shots_editor_font_size_scale = font_size_scale;
    state->shots_editor_text_stroke_width_scale = text_stroke_width_scale;
    state->shots_editor_rect_fill_opacity_scale = rect_fill_opacity_scale;
    state->shots_editor_rect_style_combo = rect_style_combo;
    state->shots_editor_text_fill_check = text_fill_check;
    state->shots_editor_text_stroke_check = text_stroke_check;
    state->shots_editor_text_shadow_check = text_shadow_check;
    state->shots_editor_rect_fill_check = rect_fill_check;
    state->shots_editor_rect_stroke_check = rect_stroke_check;
    state->shots_editor_rect_shadow_check = rect_shadow_check;
    state->shots_editor_arrow_width_scale = arrow_width_scale;
    state->shots_editor_arrow_head_len_scale = arrow_head_len_scale;
    state->shots_editor_arrow_head_angle_scale = arrow_head_angle_scale;
    state->shots_editor_arrow_theme_combo = arrow_theme_combo;
    state->shots_editor_arrow_dash_len_scale = arrow_dash_len_scale;
    state->shots_editor_arrow_dash_gap_scale = arrow_dash_gap_scale;
    state->shots_editor_arrow_head_align_combo = arrow_head_align_combo;
    state->shots_editor_arrow_shadow_check = arrow_shadow_check;
    state->shots_editor_arrow_shadow_offset_scale = arrow_shadow_offset_scale;
    state->shots_editor_arrow_curve_check = arrow_curve_check;
    state->shots_editor_arrow_curve_bend_scale = arrow_curve_bend_scale;
    state->shots_editor_auto_step_check = auto_step_check;
    state->shots_editor_step_link_check = step_link_check;
    state->shots_editor_step_label = step_label;
    state->shots_editor_props_hint = props_hint;
    state->shots_editor_group_common = props_group_common;
    state->shots_editor_group_stamp = props_group_stamp;
    state->shots_editor_group_text = props_group_text;
    state->shots_editor_group_callout = props_group_callout;
    state->shots_editor_group_arrow = props_group_arrow;
    state->shots_editor_quick_styles_scroller = quick_styles_scroller;
    state->shots_editor_quick_styles_row = quick_styles_row;
    state->shots_editor_styles_toggle_btn = btn_toggle_styles;
    state->shots_editor_quick_styles_visible = TRUE;
    state->shots_editor_path_label = editor_path;
    state->shots_editor_main_paned = editor_main_paned;
    state->shots_editor_canvas_panel = editor_canvas_panel;
    state->shots_editor_sidebar = editor_sidebar;
    state->shots_editor_sidebar_scroller = editor_sidebar_scroller;
    state->shots_editor_dock_toggle_btn = btn_dock_toggle;
    state->shots_browser_shell = browser_shell;
    state->shots_browser_shell_toggle_btn = btn_toggle_thumbs;
    state->shots_browser_shell_visible = TRUE;
    state->shots_editor_tools_docked_right = TRUE;
    state->shots_editor_zoom_factor = 1.0;
    state->shots_editor_next_step = 1;
    shots_editor_ensure_marks(state);
    shots_editor_update_step_label(state);
    shots_editor_apply_dock_layout(state);

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(on_shots_refresh_clicked), state);
    g_signal_connect(btn_select_all, "clicked", G_CALLBACK(on_shots_select_all_clicked), state);
    g_signal_connect(btn_clear_selection, "clicked", G_CALLBACK(on_shots_clear_selection_clicked), state);
    g_signal_connect(btn_toggle_shell, "clicked", G_CALLBACK(on_shots_toggle_shell_clicked), state);
    g_signal_connect(btn_quick_open, "clicked", G_CALLBACK(on_shots_open_folder_clicked), state);
    g_signal_connect(btn_quick_compact, "clicked", G_CALLBACK(on_shots_toggle_top_compact_clicked), state);
    g_signal_connect(btn_quick_more, "clicked", G_CALLBACK(on_shots_toggle_more_actions_clicked), state);
    g_signal_connect(btn_import, "clicked", G_CALLBACK(on_shots_import_clicked), state);
    g_signal_connect(btn_copy_paths, "clicked", G_CALLBACK(on_shots_copy_paths_clicked), state);
    g_signal_connect(btn_copy_prompt, "clicked", G_CALLBACK(on_shots_copy_prompt_clicked), state);
    g_signal_connect(btn_delete_selected, "clicked", G_CALLBACK(on_shots_delete_selected_clicked), state);
    g_signal_connect(thumb_search, "changed", G_CALLBACK(on_shots_search_changed), state);
    g_signal_connect(thumb_search, "icon-press", G_CALLBACK(on_shots_search_icon_press), state);
    g_signal_connect(thumb_sort_newest, "toggled", G_CALLBACK(on_shots_sort_toggled), state);
    g_signal_connect(thumb_sort_name, "toggled", G_CALLBACK(on_shots_sort_toggled), state);
    g_signal_connect(thumb_sort_type, "toggled", G_CALLBACK(on_shots_sort_toggled), state);
    g_signal_connect(icon_view, "selection-changed", G_CALLBACK(on_shots_selection_changed), state);
    g_signal_connect(icon_view, "item-activated", G_CALLBACK(on_shots_item_activated), state);
    g_signal_connect(icon_view, "key-press-event", G_CALLBACK(on_shots_icon_view_key_press), state);
    g_signal_connect(btn_capture, "clicked", G_CALLBACK(on_shots_editor_capture_clicked), state);
    g_signal_connect(btn_quick_capture, "clicked", G_CALLBACK(on_shots_editor_capture_clicked), state);
    g_signal_connect(btn_edit_selected, "clicked", G_CALLBACK(on_shots_editor_edit_selected_clicked), state);
    g_signal_connect(btn_quick_edit, "clicked", G_CALLBACK(on_shots_editor_edit_selected_clicked), state);
    g_signal_connect(btn_undo, "clicked", G_CALLBACK(on_shots_editor_undo_clicked), state);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_shots_editor_clear_clicked), state);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_shots_editor_save_clicked), state);
    g_signal_connect(btn_save_svg, "clicked", G_CALLBACK(on_shots_editor_save_svg_clicked), state);
    g_signal_connect(btn_dock_toggle, "clicked", G_CALLBACK(on_shots_editor_toggle_dock_clicked), state);
    g_signal_connect(btn_toggle_styles, "clicked", G_CALLBACK(on_shots_editor_toggle_styles_clicked), state);
    g_signal_connect(btn_toggle_thumbs, "clicked", G_CALLBACK(on_shots_toggle_browser_shell_clicked), state);
    g_signal_connect(btn_zoom_reset, "clicked", G_CALLBACK(on_shots_editor_zoom_reset_clicked), state);
    g_signal_connect(btn_step_reset, "clicked", G_CALLBACK(on_shots_editor_step_reset_clicked), state);
    g_signal_connect(step_link_check, "toggled", G_CALLBACK(on_shots_editor_step_link_toggled), state);
    g_signal_connect(btn_apply_text, "clicked", G_CALLBACK(on_shots_editor_apply_text_clicked), state);
    g_signal_connect(text_entry, "activate", G_CALLBACK(on_shots_editor_apply_text_clicked), state);
    g_signal_connect(tool_combo, "changed", G_CALLBACK(on_shots_editor_tool_changed), state);
    g_signal_connect(tool_btn_select, "toggled", G_CALLBACK(on_shots_editor_tool_button_toggled), state);
    g_signal_connect(tool_btn_arrow, "toggled", G_CALLBACK(on_shots_editor_tool_button_toggled), state);
    g_signal_connect(tool_btn_rect, "toggled", G_CALLBACK(on_shots_editor_tool_button_toggled), state);
    g_signal_connect(tool_btn_text, "toggled", G_CALLBACK(on_shots_editor_tool_button_toggled), state);
    g_signal_connect(tool_btn_stamp, "toggled", G_CALLBACK(on_shots_editor_tool_button_toggled), state);
    g_signal_connect(qs_arrow_classic, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_arrow_bold, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_arrow_pointer, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_arrow_dashed, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_arrow_dotted, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_arrow_double, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_callout_red, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_callout_green, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_callout_right, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_callout_highlight, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_callout_shadow, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_step_badge, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_step_badge_blue, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_step_badge_linked, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_text_bold, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(qs_text_outline, "clicked", G_CALLBACK(on_shots_quick_style_clicked), state);
    g_signal_connect(color_btn, "color-set", G_CALLBACK(on_shots_editor_color_set), state);
    g_signal_connect(font_style_combo, "changed", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(rect_style_combo, "changed", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(font_size_scale, "value-changed", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(text_stroke_width_scale, "value-changed", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(rect_fill_opacity_scale, "value-changed", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(text_fill_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(text_stroke_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(text_shadow_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(rect_fill_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(rect_stroke_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(rect_shadow_check, "toggled", G_CALLBACK(on_shots_editor_text_style_control_changed), state);
    g_signal_connect(btn_rect_scale_down, "clicked", G_CALLBACK(on_shots_editor_rect_scale_clicked), state);
    g_signal_connect(btn_rect_scale_up, "clicked", G_CALLBACK(on_shots_editor_rect_scale_clicked), state);
    g_signal_connect(editor_inline_entry, "activate", G_CALLBACK(on_shots_editor_inline_activate), state);
    g_signal_connect(editor_inline_entry, "focus-out-event", G_CALLBACK(on_shots_editor_inline_focus_out), state);
    g_signal_connect(editor_inline_entry, "key-press-event", G_CALLBACK(on_shots_editor_inline_key_press), state);
    g_signal_connect(arrow_width_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_head_len_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_head_angle_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_dash_len_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_dash_gap_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_theme_combo, "changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_head_align_combo, "changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_curve_check, "toggled", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_curve_bend_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(btn_arrow_curve_reset, "clicked", G_CALLBACK(on_shots_editor_arrow_curve_reset_clicked), state);
    g_signal_connect(arrow_shadow_check, "toggled", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(arrow_shadow_offset_scale, "value-changed", G_CALLBACK(on_shots_editor_arrow_style_changed), state);
    g_signal_connect(editor_canvas, "draw", G_CALLBACK(on_shots_editor_draw), state);
    g_signal_connect(editor_canvas, "button-press-event", G_CALLBACK(on_shots_editor_button_press), state);
    g_signal_connect(editor_canvas, "button-release-event", G_CALLBACK(on_shots_editor_button_release), state);
    g_signal_connect(editor_canvas, "motion-notify-event", G_CALLBACK(on_shots_editor_motion), state);
    g_signal_connect(editor_canvas, "scroll-event", G_CALLBACK(on_shots_editor_scroll), state);

    shots_editor_sync_tool_buttons(state, gtk_combo_box_get_active_id(GTK_COMBO_BOX(tool_combo)));
    shots_editor_update_properties_visibility(state);
    shots_editor_update_quick_styles_visibility(state);
    shots_set_browser_info_visible(state, FALSE);
    shots_set_browser_shell_visible(state, TRUE);
    shots_set_top_compact(state, FALSE);
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
    GtkWidget *shortcuts_tab = NULL;
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
    state->main_notebook = notebook;

    night_tab = build_night_tab(state);
    audio_tab = build_audio_tab(state);
    utilities_tab = build_utilities_tab(state);
    shortcuts_tab = build_shortcuts_tab(state);
    shots_tab = build_screenshots_tab(state);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), night_tab, gtk_label_new("Night Light"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), audio_tab, gtk_label_new("Audio"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), utilities_tab, gtk_label_new("Utilities"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), shortcuts_tab, gtk_label_new("Shortcuts"));
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
