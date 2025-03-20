// Description: A simple web browser using WebKitGTK and GTK+ 3.
// #############################################################
// # This file is part of the mono browser project.            #
// #                                                           #
// # BSD 2-Clause License                                      #
// # Copyright (c) 2025, Ignis Softwares                       #
// # All rights reserved.                                      #
// #                                                           #
// #############################################################	



#include <gtk-3.0/gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

/* Structure to store the necessary data to close a tab */
typedef struct {
    GtkNotebook *notebook;
    GtkWidget *page;
} TabData;

/* Prototype */
GtkWidget* create_browser_tab(GtkNotebook *notebook);

/* Function to get the path of the start page */
char *get_start_page_path() {
    char *current_dir = g_get_current_dir();
    char *start_page_path = g_strdup_printf("file://%s/../StartPage/index.html", current_dir);
    g_free(current_dir);
    return start_page_path;
}

/* Function to load a URL or perform a DuckDuckGo search */
static void load_url(GtkEntry *entry, gpointer user_data) {
    WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
    const gchar *url = gtk_entry_get_text(entry);

    if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://")) {
        if (g_strrstr(url, ".com") || g_strrstr(url, ".org") ||
            g_strrstr(url, ".net") || g_strrstr(url, ".gov") ||
            g_strrstr(url, ".edu") || g_strrstr(url, ".io")) {
            gchar *full_url = g_strdup_printf("https://%s", url);
            webkit_web_view_load_uri(webview, full_url);
            g_free(full_url);
        } else {
            gchar *search_url = g_strdup_printf("https://duckduckgo.com/?q=%s", url);
            webkit_web_view_load_uri(webview, search_url);
            g_free(search_url);
        }
    } else {
        webkit_web_view_load_uri(webview, url);
    }
}


/* Callback for the "Back" button */
static void on_back_button_clicked(GtkButton *button, gpointer user_data) {
    WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
    if (webkit_web_view_can_go_back(webview))
        webkit_web_view_go_back(webview);
}

/* Loads the start page in the WebView */
static void create_first_tab(WebKitWebView *webview) {
    char *start_page = get_start_page_path();
    if (g_file_test(start_page + 7, G_FILE_TEST_EXISTS))  // skips "file://"
        webkit_web_view_load_uri(webview, start_page);
    else
        webkit_web_view_load_uri(webview, "https://www.duckduckgo.com");
    g_free(start_page);
}

/* Callback to update the tab label when the page title changes.
   'user_data' is the GtkLabel used as the tab label. */
static void title_changed_cb(WebKitWebView *webview, GParamSpec *pspec, gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    const gchar *title = webkit_web_view_get_title(webview);
    if (title && strlen(title) > 0)
        gtk_label_set_text(label, title);
    else
        gtk_label_set_text(label, "New Tab");
}

/* Callback to close the current tab.
   'user_data' is a TabData containing the Notebook and the page (tab) */
static void close_tab_cb(GtkButton *button, gpointer user_data) {
    TabData *data = (TabData*) user_data;
    gint page_num = gtk_notebook_page_num(data->notebook, data->page);
    if (page_num != -1)
        gtk_notebook_remove_page(data->notebook, page_num);
    if (gtk_notebook_get_n_pages(data->notebook) == 0) {
        GtkWidget *toplevel = gtk_widget_get_toplevel(GTK_WIDGET(data->notebook));
        gtk_window_close(GTK_WINDOW(toplevel));
    }
    g_free(data);
}

/* Callback to create a new tab (used by the "+" button in each tab) */
static void new_tab_cb(GtkButton *button, gpointer user_data) {
    GtkNotebook *notebook = GTK_NOTEBOOK(user_data);
    GtkWidget *page = create_browser_tab(notebook);
    /* Creates the tab label (initially "New Tab") */
    GtkWidget *tab_label = gtk_label_new("New Tab");
    gint page_num = gtk_notebook_append_page(notebook, page, tab_label);
    gtk_widget_show_all(GTK_WIDGET(notebook));
    gtk_notebook_set_current_page(notebook, page_num);

    /* Retrieves the WebView stored in the tab container and connects the title change signal */
    WebKitWebView *webview = WEBKIT_WEB_VIEW(g_object_get_data(G_OBJECT(page), "webview"));
    g_signal_connect(webview, "notify::title", G_CALLBACK(title_changed_cb), tab_label);
}

/* Creates the content for a tab and sets up the toolbar.
   Receives the Notebook to connect the "close" and "new tab" buttons.
   Returns the container (vbox) that will be the tab page. */
GtkWidget* create_browser_tab(GtkNotebook *notebook) {
    GtkWidget *vbox, *toolbar, *scroll;
    GtkWidget *back_button, *entry, *close_button, *plus_button;
    GtkWidget *webview;
    WebKitSettings *settings;

    /* Creates the vertical container */
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    /* Creates the toolbar */
    toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

    /* Back button */
    back_button = gtk_button_new_with_label("←");
    gtk_box_pack_start(GTK_BOX(toolbar), back_button, FALSE, FALSE, 0);

    /* URL entry */
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a URL or search...");
    gtk_box_pack_start(GTK_BOX(toolbar), entry, TRUE, TRUE, 0);


    /* Close button (X) – placed to the left of the "+" button */
    close_button = gtk_button_new_with_label("X");
    gtk_box_pack_start(GTK_BOX(toolbar), close_button, FALSE, FALSE, 0);

    /* "+" button to create a new tab */
    plus_button = gtk_button_new_with_label("+");
    gtk_box_pack_start(GTK_BOX(toolbar), plus_button, FALSE, FALSE, 0);

    /* Creates the WebView and enables hardware acceleration */
    webview = webkit_web_view_new();
    settings = webkit_settings_new();
	webkit_settings_set_enable_accelerated_2d_canvas(settings, TRUE);
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(webview), settings);
    g_object_unref(settings);

    /* Places the WebView inside a scrolled window */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), webview);
    gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

    /* Associates the webview with the tab container for easy access */
    g_object_set_data(G_OBJECT(vbox), "webview", webview);

    /* Connects the signals */
    g_signal_connect(entry, "activate", G_CALLBACK(load_url), webview);
    g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), webview);
    g_signal_connect(entry, "activate", G_CALLBACK(load_url), webview);

    /* Connects the "+" button to create a new tab (passes the notebook as user_data) */
    g_signal_connect(plus_button, "clicked", G_CALLBACK(new_tab_cb), notebook);

    /* Connects the "X" button to close the current tab.
       Prepares a TabData structure for the callback */
    {
        TabData *data = g_malloc(sizeof(TabData));
        data->notebook = notebook;
        data->page = vbox;
        g_signal_connect(close_button, "clicked", G_CALLBACK(close_tab_cb), data);
    }

    /* Loads the start page */
    create_first_tab(WEBKIT_WEB_VIEW(webview));

    return vbox;
}

/* GTK initialization function */
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *main_vbox, *notebook;

    /* Activates the dark theme */
    GtkSettings *settings = gtk_settings_get_default();
    g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);

    /* Creates the main window */
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Mono Browser");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    /* Main vertical container */
    main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    /* Creates the Notebook (tab manager) */
    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);
    gtk_box_pack_start(GTK_BOX(main_vbox), notebook, TRUE, TRUE, 0);

    /* Creates the first tab */
    {
        GtkWidget *page = create_browser_tab(GTK_NOTEBOOK(notebook));
        /* Creates a simple label for the tab; it will be updated via the callback */
        GtkWidget *tab_label = gtk_label_new("New Tab");
        gint page_num = gtk_notebook_append_page(GTK_NOTEBOOK(notebook), page, tab_label);
        WebKitWebView *webview = WEBKIT_WEB_VIEW(g_object_get_data(G_OBJECT(page), "webview"));
        g_signal_connect(webview, "notify::title", G_CALLBACK(title_changed_cb), tab_label);
    }

    gtk_widget_show_all(window);
}

/* Main function */
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.ignis.monobrowser", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
