#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <cairo-svg.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <pango/pango.h>

typedef enum {
    GTK4_MARK_ARROW = 0,
    GTK4_MARK_RECT
} Gtk4MarkKind;

typedef enum {
    GTK4_TOOL_SELECT = 0,
    GTK4_TOOL_ARROW,
    GTK4_TOOL_RECT
} Gtk4Tool;

typedef struct {
    Gtk4MarkKind kind;
    gdouble x1;
    gdouble y1;
    gdouble x2;
    gdouble y2;
    GdkRGBA color;
    gdouble stroke_width;
} Gtk4Mark;

typedef struct {
    gchar *name;
    gchar *path;
    gint64 mtime;
} Gtk4ImageEntry;

typedef struct {
    GtkApplication *app;
    GtkWidget *window;

    gchar *launch_dir;
    gchar *shots_dir;
    gchar *editor_image_path;

    GtkWidget *status_label;
    GtkWidget *selected_label;
    GtkWidget *paths_view;
    GtkWidget *search_entry;
    GtkWidget *thumb_flow;
    GtkWidget *editor_area;
    GtkWidget *editor_status;
    GtkWidget *editor_path;

    GtkWidget *tool_select_btn;
    GtkWidget *tool_arrow_btn;
    GtkWidget *tool_rect_btn;
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
} Gtk4State;

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
    g_free(data);
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

static void gtk4_launch_in_background(const gchar *command) {
    GError *error = NULL;
    if (!command || *command == '\0') {
        return;
    }
    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Launch failed (%s): %s", command, error ? error->message : "unknown");
        g_clear_error(&error);
    }
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
    GListModel *selected = NULL;

    if (!state || !state->thumb_flow) {
        return paths;
    }

    selected = G_LIST_MODEL(gtk_flow_box_get_selected_children(GTK_FLOW_BOX(state->thumb_flow)));
    if (!selected) {
        return paths;
    }

    for (guint i = 0; i < g_list_model_get_n_items(selected); i += 1) {
        GtkFlowBoxChild *child = g_list_model_get_item(selected, i);
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
        g_object_unref(child);
    }

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
    gtk4_set_status(state->editor_status, "Image loaded. Use Arrow/Rect and drag on canvas.", "Editor ready.");
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
                if (!mark) {
                    continue;
                }
                cairo_set_source_rgba(cr, mark->color.red, mark->color.green, mark->color.blue, mark->color.alpha);
                cairo_set_line_width(cr, mark->stroke_width > 0.0 ? mark->stroke_width : 4.0);
                if (mark->kind == GTK4_MARK_ARROW) {
                    gdouble vx = mark->x2 - mark->x1;
                    gdouble vy = mark->y2 - mark->y1;
                    gdouble len = sqrt(vx * vx + vy * vy);
                    gdouble nx = 0.0;
                    gdouble ny = 0.0;
                    gdouble head = 16.0;
                    gdouble spread = 0.46;
                    cairo_move_to(cr, mark->x1, mark->y1);
                    cairo_line_to(cr, mark->x2, mark->y2);
                    cairo_stroke(cr);
                    if (len > 1e-6) {
                        nx = vx / len;
                        ny = vy / len;
                        cairo_move_to(cr, mark->x2, mark->y2);
                        cairo_line_to(cr,
                                      mark->x2 - head * (nx * cos(spread) - ny * sin(spread)),
                                      mark->y2 - head * (ny * cos(spread) + nx * sin(spread)));
                        cairo_move_to(cr, mark->x2, mark->y2);
                        cairo_line_to(cr,
                                      mark->x2 - head * (nx * cos(spread) + ny * sin(spread)),
                                      mark->y2 - head * (ny * cos(spread) - nx * sin(spread)));
                        cairo_stroke(cr);
                    }
                } else {
                    gdouble rx = MIN(mark->x1, mark->x2);
                    gdouble ry = MIN(mark->y1, mark->y2);
                    gdouble rw = fabs(mark->x2 - mark->x1);
                    gdouble rh = fabs(mark->y2 - mark->y1);
                    cairo_rectangle(cr, rx, ry, rw, rh);
                    cairo_stroke(cr);
                }
            }
        }

        if (state->dragging && (state->active_tool == GTK4_TOOL_ARROW || state->active_tool == GTK4_TOOL_RECT)) {
            cairo_set_source_rgba(cr, 0.41, 0.84, 0.99, 0.92);
            cairo_set_line_width(cr, 3.0);
            if (state->active_tool == GTK4_TOOL_ARROW) {
                cairo_move_to(cr, state->drag_start_x, state->drag_start_y);
                cairo_line_to(cr, state->drag_cur_x, state->drag_cur_y);
                cairo_stroke(cr);
            } else {
                cairo_rectangle(cr,
                                MIN(state->drag_start_x, state->drag_cur_x),
                                MIN(state->drag_start_y, state->drag_cur_y),
                                fabs(state->drag_cur_x - state->drag_start_x),
                                fabs(state->drag_cur_y - state->drag_start_y));
                cairo_stroke(cr);
            }
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

static void gtk4_on_canvas_pressed(GtkGestureClick *gesture, gint n_press, gdouble x, gdouble y, gpointer user_data) {
    Gtk4State *state = user_data;
    gdouble ix = 0.0;
    gdouble iy = 0.0;
    guint button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    (void)n_press;

    if (!state || !state->editor_pixbuf || button != GDK_BUTTON_PRIMARY) {
        return;
    }
    if (!gtk4_widget_to_image(state, x, y, &ix, &iy, FALSE)) {
        return;
    }
    if (state->active_tool == GTK4_TOOL_SELECT) {
        gtk4_set_status(state->editor_status, "Select tool is active. Switch to Arrow or Rect to draw.", "Select mode.");
        return;
    }

    state->dragging = TRUE;
    state->drag_kind = (state->active_tool == GTK4_TOOL_ARROW) ? GTK4_MARK_ARROW : GTK4_MARK_RECT;
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

    if ((state->drag_kind == GTK4_MARK_ARROW && sqrt(dx * dx + dy * dy) >= 3.0) ||
        (state->drag_kind == GTK4_MARK_RECT && fabs(dx) >= 6.0 && fabs(dy) >= 6.0)) {
        mark = g_new0(Gtk4Mark, 1);
        mark->kind = state->drag_kind;
        mark->x1 = state->drag_start_x;
        mark->y1 = state->drag_start_y;
        mark->x2 = state->drag_cur_x;
        mark->y2 = state->drag_cur_y;
        mark->stroke_width = 4.0;
        gdk_rgba_parse(&mark->color, state->drag_kind == GTK4_MARK_ARROW ? "#ef5b5b" : "#4dd3ff");
        g_ptr_array_add(state->marks, mark);
        gtk4_set_status(state->editor_status,
                        state->drag_kind == GTK4_MARK_ARROW ? "Arrow annotation added." : "Rectangle annotation added.",
                        "Annotation added.");
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
    if (state->tool_rect_btn) {
        gtk_check_button_set_active(GTK_CHECK_BUTTON(state->tool_rect_btn), tool == GTK4_TOOL_RECT);
    }
    state->tool_syncing = FALSE;
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
    } else if (g_strcmp0(tool_id, "rect") == 0) {
        gtk4_set_active_tool(state, GTK4_TOOL_RECT);
        gtk4_set_status(state->editor_status, "Rectangle tool selected.", "Rect tool.");
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
        GdkPixbuf *pix = gdk_pixbuf_new_from_file_at_scale(entry->path, 220, 140, TRUE, NULL);
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
        GtkWidget *label = gtk_label_new(entry->name);
        GtkWidget *preview = NULL;

        gtk_widget_set_tooltip_text(row, entry->path);
        g_object_set_data_full(G_OBJECT(row), "shot-path", g_strdup(entry->path), g_free);
        gtk_widget_add_css_class(row, "surface");
        gtk_widget_set_margin_start(row, 4);
        gtk_widget_set_margin_end(row, 4);
        gtk_widget_set_margin_top(row, 4);
        gtk_widget_set_margin_bottom(row, 4);

        if (pix) {
            GdkTexture *texture = gdk_texture_new_for_pixbuf(pix);
            preview = gtk_picture_new_for_paintable(GDK_PAINTABLE(texture));
            gtk_picture_set_can_shrink(GTK_PICTURE(preview), TRUE);
            gtk_picture_set_content_fit(GTK_PICTURE(preview), GTK_CONTENT_FIT_CONTAIN);
            gtk_widget_set_size_request(preview, 220, 140);
            g_object_unref(texture);
            g_object_unref(pix);
        } else {
            preview = gtk_image_new_from_icon_name("image-x-generic-symbolic");
            gtk_widget_set_size_request(preview, 220, 140);
        }

        gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
        gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
        gtk_widget_set_size_request(label, 220, -1);
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
    gtk4_launch_in_background(cmd);
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

    if ((state & GDK_CONTROL_MASK) && (keyval == GDK_KEY_a || keyval == GDK_KEY_A)) {
        for (GtkWidget *child = gtk_widget_get_first_child(app->thumb_flow); child; child = gtk_widget_get_next_sibling(child)) {
            if (GTK_IS_FLOW_BOX_CHILD(child)) {
                gtk_flow_box_select_child(GTK_FLOW_BOX(app->thumb_flow), GTK_FLOW_BOX_CHILD(child));
            }
        }
        gtk4_update_selection_ui(app);
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

static GtkWidget *gtk4_build_ui(Gtk4State *state) {
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *header = gtk_label_new("Linux Utilities Control Center (GTK4 Port: Screenshot Browser + Event Canvas)");
    GtkWidget *toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *btn_refresh = gtk_button_new_with_label("Refresh");
    GtkWidget *btn_open_folder = gtk_button_new_with_label("Open Folder");
    GtkWidget *btn_delete = gtk_button_new_with_label("Delete Selected");
    GtkWidget *search = gtk_search_entry_new();
    GtkWidget *split = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *browser_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *browser_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *thumb_scroll = gtk_scrolled_window_new();
    GtkWidget *thumb_flow = gtk_flow_box_new();
    GtkWidget *selected = gtk_label_new("No images selected.");
    GtkWidget *paths_scroll = gtk_scrolled_window_new();
    GtkWidget *paths_view = gtk_text_view_new();
    GtkWidget *editor_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    GtkWidget *tool_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *tool_select = gtk_check_button_new_with_label("Select");
    GtkWidget *tool_arrow = gtk_check_button_new_with_label("Arrow");
    GtkWidget *tool_rect = gtk_check_button_new_with_label("Rect");
    GtkWidget *editor_path = gtk_label_new("No image selected");
    GtkWidget *editor_area = gtk_drawing_area_new();
    GtkWidget *editor_status = gtk_label_new("Editor ready.");
    GtkWidget *status = gtk_label_new("Status: ready.");
    GtkEventController *motion = NULL;
    GtkEventController *scroll = NULL;
    GtkGesture *click = NULL;
    GtkEventController *key = NULL;

    gtk_widget_set_margin_top(root, 10);
    gtk_widget_set_margin_bottom(root, 10);
    gtk_widget_set_margin_start(root, 10);
    gtk_widget_set_margin_end(root, 10);

    gtk_label_set_xalign(GTK_LABEL(header), 0.0f);
    gtk_widget_add_css_class(header, "hero-title");
    gtk_label_set_xalign(GTK_LABEL(status), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(selected), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(editor_path), 0.0f);
    gtk_label_set_xalign(GTK_LABEL(editor_status), 0.0f);

    gtk_editable_set_text(GTK_EDITABLE(search), "");
    gtk_widget_set_hexpand(search, TRUE);
    gtk_box_append(GTK_BOX(toolbar), btn_refresh);
    gtk_box_append(GTK_BOX(toolbar), btn_open_folder);
    gtk_box_append(GTK_BOX(toolbar), btn_delete);
    gtk_box_append(GTK_BOX(toolbar), search);

    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(thumb_flow), GTK_SELECTION_MULTIPLE);
    gtk_flow_box_set_activate_on_single_click(GTK_FLOW_BOX(thumb_flow), FALSE);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(thumb_flow), 8);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(thumb_flow), 8);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(thumb_flow), 8);
    gtk_widget_set_focusable(thumb_flow, TRUE);
    gtk_widget_set_tooltip_text(thumb_flow, "Ctrl+A select all, Delete remove selected.");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(thumb_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(thumb_scroll, -1, 220);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(thumb_scroll), thumb_flow);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(paths_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(paths_view), GTK_WRAP_CHAR);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(paths_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(paths_scroll, -1, 88);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(paths_scroll), paths_view);

    gtk_box_append(GTK_BOX(browser_info), selected);
    gtk_box_append(GTK_BOX(browser_info), paths_scroll);
    gtk_box_append(GTK_BOX(browser_box), thumb_scroll);
    gtk_box_append(GTK_BOX(browser_box), browser_info);

    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_arrow), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_group(GTK_CHECK_BUTTON(tool_rect), GTK_CHECK_BUTTON(tool_select));
    gtk_check_button_set_active(GTK_CHECK_BUTTON(tool_select), TRUE);
    g_object_set_data(G_OBJECT(tool_select), "tool-id", "select");
    g_object_set_data(G_OBJECT(tool_arrow), "tool-id", "arrow");
    g_object_set_data(G_OBJECT(tool_rect), "tool-id", "rect");
    gtk_box_append(GTK_BOX(tool_row), tool_select);
    gtk_box_append(GTK_BOX(tool_row), tool_arrow);
    gtk_box_append(GTK_BOX(tool_row), tool_rect);

    gtk_widget_set_hexpand(editor_area, TRUE);
    gtk_widget_set_vexpand(editor_area, TRUE);
    gtk_widget_set_size_request(editor_area, -1, 320);
    gtk_widget_add_css_class(editor_area, "surface");
    gtk_box_append(GTK_BOX(editor_box), tool_row);
    gtk_box_append(GTK_BOX(editor_box), editor_path);
    gtk_box_append(GTK_BOX(editor_box), editor_area);
    gtk_box_append(GTK_BOX(editor_box), editor_status);

    gtk_paned_set_start_child(GTK_PANED(split), browser_box);
    gtk_paned_set_end_child(GTK_PANED(split), editor_box);
    gtk_paned_set_position(GTK_PANED(split), 340);
    gtk_widget_set_hexpand(split, TRUE);
    gtk_widget_set_vexpand(split, TRUE);

    gtk_box_append(GTK_BOX(root), header);
    gtk_box_append(GTK_BOX(root), toolbar);
    gtk_box_append(GTK_BOX(root), split);
    gtk_box_append(GTK_BOX(root), status);

    state->status_label = status;
    state->selected_label = selected;
    state->paths_view = paths_view;
    state->search_entry = search;
    state->thumb_flow = thumb_flow;
    state->editor_area = editor_area;
    state->editor_status = editor_status;
    state->editor_path = editor_path;
    state->tool_select_btn = tool_select;
    state->tool_arrow_btn = tool_arrow;
    state->tool_rect_btn = tool_rect;
    state->active_tool = GTK4_TOOL_SELECT;
    state->zoom_factor = 1.0;

    g_signal_connect(btn_refresh, "clicked", G_CALLBACK(gtk4_on_refresh_clicked), state);
    g_signal_connect(btn_open_folder, "clicked", G_CALLBACK(gtk4_on_open_folder_clicked), state);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(gtk4_on_delete_clicked), state);
    g_signal_connect(search, "changed", G_CALLBACK(gtk4_on_search_changed), state);
    g_signal_connect(thumb_flow, "selected-children-changed", G_CALLBACK(gtk4_on_thumb_selection_changed), state);
    g_signal_connect(thumb_flow, "child-activated", G_CALLBACK(gtk4_on_thumb_activated), state);
    g_signal_connect(tool_select, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_arrow, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
    g_signal_connect(tool_rect, "toggled", G_CALLBACK(gtk4_on_tool_toggled), state);
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

    return root;
}

static void gtk4_state_free(Gtk4State *state) {
    if (!state) {
        return;
    }
    if (state->editor_pixbuf) {
        g_object_unref(state->editor_pixbuf);
    }
    if (state->marks) {
        g_ptr_array_free(state->marks, TRUE);
    }
    g_free(state->launch_dir);
    g_free(state->shots_dir);
    g_free(state->editor_image_path);
    g_free(state);
}

static void gtk4_on_activate(GtkApplication *app, gpointer user_data) {
    Gtk4State *state = user_data;
    GtkWidget *root = NULL;

    state->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(state->window), "Linux Utilities Control Center");
    gtk_window_set_default_size(GTK_WINDOW(state->window), 1260, 820);
    state->marks = g_ptr_array_new_with_free_func(gtk4_free_mark);

    root = gtk4_build_ui(state);
    gtk_window_set_child(GTK_WINDOW(state->window), root);
    gtk_window_present(GTK_WINDOW(state->window));
    gtk4_reload(state);
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
