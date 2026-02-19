#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>

typedef struct {
    GtkApplication *app;
    gchar *layout_path;
} DemoState;

static GQuark dsl_error_quark(void) {
    return g_quark_from_static_string("gtk-dsl-error");
}

static gboolean kf_has_key(GKeyFile *kf, const gchar *group, const gchar *key) {
    GError *error = NULL;
    gboolean ok = g_key_file_has_key(kf, group, key, &error);
    g_clear_error(&error);
    return ok;
}

static gchar *kf_get_string_default(GKeyFile *kf, const gchar *group, const gchar *key, const gchar *fallback) {
    GError *error = NULL;
    gchar *value = NULL;

    if (!kf_has_key(kf, group, key)) {
        return g_strdup(fallback ? fallback : "");
    }

    value = g_key_file_get_string(kf, group, key, &error);
    if (error) {
        g_clear_error(&error);
        return g_strdup(fallback ? fallback : "");
    }
    return value;
}

static gboolean kf_get_bool_default(GKeyFile *kf, const gchar *group, const gchar *key, gboolean fallback) {
    GError *error = NULL;
    gboolean value = fallback;

    if (!kf_has_key(kf, group, key)) {
        return fallback;
    }

    value = g_key_file_get_boolean(kf, group, key, &error);
    if (error) {
        g_clear_error(&error);
        return fallback;
    }
    return value;
}

static gint kf_get_int_default(GKeyFile *kf, const gchar *group, const gchar *key, gint fallback) {
    GError *error = NULL;
    gint value = fallback;

    if (!kf_has_key(kf, group, key)) {
        return fallback;
    }

    value = g_key_file_get_integer(kf, group, key, &error);
    if (error) {
        g_clear_error(&error);
        return fallback;
    }
    return value;
}

static gdouble kf_get_double_default(GKeyFile *kf, const gchar *group, const gchar *key, gdouble fallback) {
    GError *error = NULL;
    gdouble value = fallback;

    if (!kf_has_key(kf, group, key)) {
        return fallback;
    }

    value = g_key_file_get_double(kf, group, key, &error);
    if (error) {
        g_clear_error(&error);
        return fallback;
    }
    return value;
}

static GtkOrientation parse_orientation(const gchar *text, GtkOrientation fallback) {
    if (!text) {
        return fallback;
    }
    if (g_ascii_strcasecmp(text, "horizontal") == 0 || g_ascii_strcasecmp(text, "h") == 0 || g_ascii_strcasecmp(text, "row") == 0) {
        return GTK_ORIENTATION_HORIZONTAL;
    }
    if (g_ascii_strcasecmp(text, "vertical") == 0 || g_ascii_strcasecmp(text, "v") == 0 || g_ascii_strcasecmp(text, "column") == 0) {
        return GTK_ORIENTATION_VERTICAL;
    }
    return fallback;
}

static GtkAlign parse_align(const gchar *text, GtkAlign fallback) {
    if (!text) {
        return fallback;
    }
    if (g_ascii_strcasecmp(text, "fill") == 0) {
        return GTK_ALIGN_FILL;
    }
    if (g_ascii_strcasecmp(text, "start") == 0) {
        return GTK_ALIGN_START;
    }
    if (g_ascii_strcasecmp(text, "end") == 0) {
        return GTK_ALIGN_END;
    }
    if (g_ascii_strcasecmp(text, "center") == 0) {
        return GTK_ALIGN_CENTER;
    }
    if (g_ascii_strcasecmp(text, "baseline") == 0) {
        return GTK_ALIGN_BASELINE;
    }
    return fallback;
}

static void launch_command_async(const gchar *command) {
    GError *error = NULL;
    if (!command || *command == '\0') {
        return;
    }
    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Failed to launch '%s': %s", command, error ? error->message : "unknown");
        g_clear_error(&error);
    }
}

static void on_dsl_button_clicked(GtkButton *button, gpointer user_data) {
    const gchar *command = g_object_get_data(G_OBJECT(button), "dsl_on_click");
    (void)user_data;
    launch_command_async(command);
}

static void on_dsl_switch_notify(GObject *obj, GParamSpec *pspec, gpointer user_data) {
    const gchar *on_cmd = g_object_get_data(obj, "dsl_on_toggle_on");
    const gchar *off_cmd = g_object_get_data(obj, "dsl_on_toggle_off");
    gboolean active = gtk_switch_get_active(GTK_SWITCH(obj));
    (void)pspec;
    (void)user_data;

    if (active) {
        launch_command_async(on_cmd);
    } else {
        launch_command_async(off_cmd);
    }
}

static void add_style_classes(GtkWidget *widget, const gchar *class_list) {
    GtkStyleContext *ctx = NULL;
    gchar **tokens = NULL;

    if (!widget || !class_list || *class_list == '\0') {
        return;
    }

    ctx = gtk_widget_get_style_context(widget);
    tokens = g_strsplit_set(class_list, ",; ", -1);
    for (gint i = 0; tokens && tokens[i]; ++i) {
        if (tokens[i][0] != '\0') {
            gtk_style_context_add_class(ctx, tokens[i]);
        }
    }
    g_strfreev(tokens);
}

static void apply_common_widget_props(GKeyFile *kf, const gchar *group, GtkWidget *widget) {
    const gint margin_all = kf_get_int_default(kf, group, "margin", -1);
    const gint margin_top = kf_get_int_default(kf, group, "margin_top", margin_all >= 0 ? margin_all : 0);
    const gint margin_bottom = kf_get_int_default(kf, group, "margin_bottom", margin_all >= 0 ? margin_all : 0);
    const gint margin_start = kf_get_int_default(kf, group, "margin_start", margin_all >= 0 ? margin_all : 0);
    const gint margin_end = kf_get_int_default(kf, group, "margin_end", margin_all >= 0 ? margin_all : 0);
    const gboolean hexpand = kf_get_bool_default(kf, group, "hexpand", FALSE);
    const gboolean vexpand = kf_get_bool_default(kf, group, "vexpand", FALSE);
    const gint width_request = kf_get_int_default(kf, group, "width_request", -1);
    const gint height_request = kf_get_int_default(kf, group, "height_request", -1);
    gchar *halign_str = kf_get_string_default(kf, group, "halign", "");
    gchar *valign_str = kf_get_string_default(kf, group, "valign", "");
    gchar *class_str = kf_get_string_default(kf, group, "class", "");

    gtk_widget_set_hexpand(widget, hexpand);
    gtk_widget_set_vexpand(widget, vexpand);
    gtk_widget_set_margin_top(widget, margin_top);
    gtk_widget_set_margin_bottom(widget, margin_bottom);
    gtk_widget_set_margin_start(widget, margin_start);
    gtk_widget_set_margin_end(widget, margin_end);
    if (width_request > 0 || height_request > 0) {
        gtk_widget_set_size_request(widget, width_request, height_request);
    }

    if (halign_str && *halign_str) {
        gtk_widget_set_halign(widget, parse_align(halign_str, GTK_ALIGN_FILL));
    }
    if (valign_str && *valign_str) {
        gtk_widget_set_valign(widget, parse_align(valign_str, GTK_ALIGN_FILL));
    }
    add_style_classes(widget, class_str);

    g_free(halign_str);
    g_free(valign_str);
    g_free(class_str);
}

static GtkWidget *create_widget_for_group(GKeyFile *kf, const gchar *group, GError **error) {
    GtkWidget *widget = NULL;
    gchar *type = kf_get_string_default(kf, group, "type", "");

    if (g_ascii_strcasecmp(type, "box") == 0) {
        gchar *orientation = kf_get_string_default(kf, group, "orientation", "vertical");
        gint spacing = kf_get_int_default(kf, group, "spacing", 0);
        widget = gtk_box_new(parse_orientation(orientation, GTK_ORIENTATION_VERTICAL), spacing);
        g_free(orientation);
    } else if (g_ascii_strcasecmp(type, "grid") == 0) {
        widget = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(widget), kf_get_int_default(kf, group, "row_spacing", 0));
        gtk_grid_set_column_spacing(GTK_GRID(widget), kf_get_int_default(kf, group, "column_spacing", 0));
    } else if (g_ascii_strcasecmp(type, "label") == 0) {
        gchar *text = kf_get_string_default(kf, group, "text", group);
        gboolean use_markup = kf_get_bool_default(kf, group, "use_markup", FALSE);
        gdouble xalign = kf_get_double_default(kf, group, "xalign", 0.0);
        gboolean wrap = kf_get_bool_default(kf, group, "wrap", FALSE);
        widget = gtk_label_new(NULL);
        if (use_markup) {
            gtk_label_set_markup(GTK_LABEL(widget), text);
        } else {
            gtk_label_set_text(GTK_LABEL(widget), text);
        }
        gtk_label_set_xalign(GTK_LABEL(widget), (gfloat)xalign);
        gtk_label_set_line_wrap(GTK_LABEL(widget), wrap);
        g_free(text);
    } else if (g_ascii_strcasecmp(type, "button") == 0) {
        gchar *text = kf_get_string_default(kf, group, "text", group);
        gchar *on_click = kf_get_string_default(kf, group, "on_click", "");
        widget = gtk_button_new_with_label(text);
        if (on_click && *on_click) {
            g_object_set_data_full(G_OBJECT(widget), "dsl_on_click", g_strdup(on_click), g_free);
            g_signal_connect(widget, "clicked", G_CALLBACK(on_dsl_button_clicked), NULL);
        }
        g_free(on_click);
        g_free(text);
    } else if (g_ascii_strcasecmp(type, "switch") == 0) {
        gchar *on_cmd = kf_get_string_default(kf, group, "on_toggle_on", "");
        gchar *off_cmd = kf_get_string_default(kf, group, "on_toggle_off", "");
        gboolean active = kf_get_bool_default(kf, group, "active", FALSE);
        widget = gtk_switch_new();
        gtk_switch_set_active(GTK_SWITCH(widget), active);
        if ((on_cmd && *on_cmd) || (off_cmd && *off_cmd)) {
            g_object_set_data_full(G_OBJECT(widget), "dsl_on_toggle_on", g_strdup(on_cmd), g_free);
            g_object_set_data_full(G_OBJECT(widget), "dsl_on_toggle_off", g_strdup(off_cmd), g_free);
            g_signal_connect(widget, "notify::active", G_CALLBACK(on_dsl_switch_notify), NULL);
        }
        g_free(on_cmd);
        g_free(off_cmd);
    } else if (g_ascii_strcasecmp(type, "scale") == 0) {
        gchar *orientation = kf_get_string_default(kf, group, "orientation", "horizontal");
        gdouble min = kf_get_double_default(kf, group, "min", 0.0);
        gdouble max = kf_get_double_default(kf, group, "max", 100.0);
        gdouble step = kf_get_double_default(kf, group, "step", 1.0);
        gdouble value = kf_get_double_default(kf, group, "value", min);
        gboolean draw_value = kf_get_bool_default(kf, group, "draw_value", FALSE);
        widget = gtk_scale_new_with_range(parse_orientation(orientation, GTK_ORIENTATION_HORIZONTAL), min, max, step);
        gtk_range_set_value(GTK_RANGE(widget), value);
        gtk_scale_set_draw_value(GTK_SCALE(widget), draw_value);
        g_free(orientation);
    } else if (g_ascii_strcasecmp(type, "separator") == 0) {
        gchar *orientation = kf_get_string_default(kf, group, "orientation", "horizontal");
        widget = gtk_separator_new(parse_orientation(orientation, GTK_ORIENTATION_HORIZONTAL));
        g_free(orientation);
    } else {
        g_set_error(error, dsl_error_quark(), 1, "Unknown widget type '%s' in group [%s]", type, group);
    }

    if (widget) {
        apply_common_widget_props(kf, group, widget);
    }

    g_free(type);
    return widget;
}

static gboolean attach_widget_to_parent(GKeyFile *kf, const gchar *group, GHashTable *widgets, GError **error) {
    GtkWidget *child = g_hash_table_lookup(widgets, group);
    gchar *parent_id = kf_get_string_default(kf, group, "parent", "");
    GtkWidget *parent = NULL;

    if (!child) {
        g_free(parent_id);
        return TRUE;
    }

    if (!parent_id || *parent_id == '\0') {
        g_free(parent_id);
        return TRUE;
    }

    parent = g_hash_table_lookup(widgets, parent_id);
    if (!parent) {
        g_set_error(error, dsl_error_quark(), 2, "Unknown parent '%s' for group [%s]", parent_id, group);
        g_free(parent_id);
        return FALSE;
    }

    if (GTK_IS_BOX(parent)) {
        gboolean expand = kf_get_bool_default(kf, group, "expand", FALSE);
        gboolean fill = kf_get_bool_default(kf, group, "fill", expand);
        guint padding = (guint)kf_get_int_default(kf, group, "padding", 0);
        gtk_box_pack_start(GTK_BOX(parent), child, expand, fill, padding);
    } else if (GTK_IS_GRID(parent)) {
        gint left = kf_get_int_default(kf, group, "left", 0);
        gint top = kf_get_int_default(kf, group, "top", 0);
        gint width = MAX(1, kf_get_int_default(kf, group, "width", 1));
        gint height = MAX(1, kf_get_int_default(kf, group, "height", 1));
        gtk_grid_attach(GTK_GRID(parent), child, left, top, width, height);
    } else if (GTK_IS_CONTAINER(parent)) {
        gtk_container_add(GTK_CONTAINER(parent), child);
    } else {
        g_set_error(error, dsl_error_quark(), 3, "Parent '%s' is not a container for [%s]", parent_id, group);
        g_free(parent_id);
        return FALSE;
    }

    g_free(parent_id);
    return TRUE;
}

static GtkWidget *build_widget_tree_from_keyfile(GKeyFile *kf, gchar **title_out, GError **error) {
    gsize count = 0;
    gchar **groups = g_key_file_get_groups(kf, &count);
    GHashTable *widgets = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    GtkWidget *root = NULL;
    gchar *root_id = kf_get_string_default(kf, "meta", "root", "");
    gchar *title = kf_get_string_default(kf, "meta", "title", "GTK DSL Launcher");

    for (gsize i = 0; i < count; ++i) {
        GtkWidget *widget = NULL;
        if (g_ascii_strcasecmp(groups[i], "meta") == 0) {
            continue;
        }
        widget = create_widget_for_group(kf, groups[i], error);
        if (!widget) {
            g_strfreev(groups);
            g_hash_table_destroy(widgets);
            g_free(root_id);
            g_free(title);
            return NULL;
        }
        g_hash_table_insert(widgets, g_strdup(groups[i]), widget);
    }

    for (gsize i = 0; i < count; ++i) {
        if (g_ascii_strcasecmp(groups[i], "meta") == 0) {
            continue;
        }
        if (!attach_widget_to_parent(kf, groups[i], widgets, error)) {
            g_strfreev(groups);
            g_hash_table_destroy(widgets);
            g_free(root_id);
            g_free(title);
            return NULL;
        }
    }

    if (!root_id || *root_id == '\0') {
        g_set_error(error, dsl_error_quark(), 4, "Missing [meta] root value");
    } else {
        root = g_hash_table_lookup(widgets, root_id);
        if (!root) {
            g_set_error(error, dsl_error_quark(), 5, "Root widget '%s' not found", root_id);
        }
    }

    if (title_out) {
        *title_out = g_strdup(title);
    }

    g_strfreev(groups);
    g_hash_table_destroy(widgets);
    g_free(root_id);
    g_free(title);
    return root;
}

static GtkWidget *build_widget_tree_from_file(const gchar *path, gchar **title_out, GError **error) {
    GKeyFile *kf = g_key_file_new();
    GtkWidget *root = NULL;

    if (!g_key_file_load_from_file(kf, path, G_KEY_FILE_NONE, error)) {
        g_key_file_unref(kf);
        return NULL;
    }

    root = build_widget_tree_from_keyfile(kf, title_out, error);
    g_key_file_unref(kf);
    return root;
}

static void apply_demo_css(void) {
    const gchar *css =
        "window {"
        "  background: #10151e;"
        "}"
        ".title {"
        "  font-size: 17px;"
        "  font-weight: 700;"
        "  color: #f3f5f8;"
        "}"
        ".subtitle {"
        "  font-size: 11px;"
        "  color: #b4c1d3;"
        "}"
        "button {"
        "  background: #2a3444;"
        "  color: #edf2fa;"
        "  border: 1px solid #455774;"
        "  border-radius: 8px;"
        "  padding: 8px 10px;"
        "}"
        "button:hover {"
        "  background: #35455d;"
        "}";
    GtkCssProvider *provider = gtk_css_provider_new();
    GError *error = NULL;

    gtk_css_provider_load_from_data(provider, css, -1, &error);
    if (error) {
        g_warning("CSS load failed: %s", error->message);
        g_clear_error(&error);
    }
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    DemoState *state = user_data;
    GtkWidget *window = gtk_application_window_new(app);
    GtkWidget *root = NULL;
    GtkWidget *error_label = NULL;
    gchar *title = NULL;
    GError *error = NULL;

    apply_demo_css();
    gtk_window_set_default_size(GTK_WINDOW(window), 920, 640);

    root = build_widget_tree_from_file(state->layout_path, &title, &error);
    if (!root) {
        gchar *message = g_strdup_printf("Failed to load DSL layout:\n%s", error ? error->message : "unknown error");
        gtk_window_set_title(GTK_WINDOW(window), "GTK DSL Demo (Error)");
        error_label = gtk_label_new(message);
        gtk_label_set_xalign(GTK_LABEL(error_label), 0.0f);
        gtk_label_set_yalign(GTK_LABEL(error_label), 0.0f);
        gtk_label_set_line_wrap(GTK_LABEL(error_label), TRUE);
        gtk_widget_set_margin_top(error_label, 16);
        gtk_widget_set_margin_bottom(error_label, 16);
        gtk_widget_set_margin_start(error_label, 16);
        gtk_widget_set_margin_end(error_label, 16);
        gtk_container_add(GTK_CONTAINER(window), error_label);
        g_clear_error(&error);
        g_free(message);
    } else {
        gtk_window_set_title(GTK_WINDOW(window), title ? title : "GTK DSL Demo");
        gtk_container_add(GTK_CONTAINER(window), root);
    }

    g_free(title);
    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    DemoState state = {0};
    gint rc = 0;
    gchar *argv0_only[] = { NULL, NULL };

    state.layout_path = g_strdup((argc > 1 && argv[1] && *argv[1]) ? argv[1] : "dsl/workbench.gdsl");
    state.app = gtk_application_new("com.antshiv.gtkdsl.demo", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(state.app, "activate", G_CALLBACK(on_activate), &state);
    /* We consume the optional layout path ourselves. Passing extra argv items to
     * GtkApplication makes it treat them as files to open, which raises
     * "This application can not open files." warnings. */
    argv0_only[0] = argv[0];
    rc = g_application_run(G_APPLICATION(state.app), 1, argv0_only);
    g_object_unref(state.app);
    g_free(state.layout_path);
    return rc;
}
