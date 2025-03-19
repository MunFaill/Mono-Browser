// Dependencies that's make me crazy  (I Wrote This Right? Idk my english sucks) - Mun

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

// Start Page Func
char *get_start_page_path() {
	char *current_dir = g_get_current_dir(); // Obtém diretório atual
	char *start_page_path = g_strdup_printf("file://%s/../StartPage/index.html", current_dir);
	g_free(current_dir); // Get memory free to avoid computers explosion :D
	return start_page_path;
}

// Search on Duck Duck Go Func
static void load_url(GtkEntry *entry, gpointer user_data) {
	WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
	const gchar *url = gtk_entry_get_text(entry);

	// Verify if the url has "http://" or "https://"
	if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://")) {
		// Verify if is a valid domain
		if (g_strrstr(url, ".com") || g_strrstr(url, ".org") || g_strrstr(url, ".net") ||
			g_strrstr(url, ".gov") || g_strrstr(url, ".edu") || g_strrstr(url, ".io")) {
			// If it is a domain, add: "https://"
			gchar *full_url = g_strdup_printf("https://%s", url);
			webkit_web_view_load_uri(webview, full_url);
			g_free(full_url);
		} else {
			// If not, make a search on duck duck go
			gchar *search_url = g_strdup_printf("https://duckduckgo.com/?q=%s", url);
			webkit_web_view_load_uri(webview, search_url);
			g_free(search_url);
		}
	} else {
		// Load the url if there's no problem
		webkit_web_view_load_uri(webview, url);
	}
}

// On "Go" Button Clicled Func
static void on_button_clicked(GtkButton *button, gpointer user_data) {
	GtkEntry *entry = GTK_ENTRY(user_data);
	load_url(entry, g_object_get_data(G_OBJECT(entry), "webview"));
}

// Func to go back
static void on_back_button_clicked(GtkButton *button, gpointer user_data) {
	WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
	if (webkit_web_view_can_go_back(webview)) {
		webkit_web_view_go_back(webview);
	}
}

// Load init page
static void create_first_tab(WebKitWebView *webview) {
	char *start_page = get_start_page_path();
	if (g_file_test(start_page + 7, G_FILE_TEST_EXISTS)) { // Verify if the file exists
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), start_page);
	} else {
		// If not, Load DuckDuckGo
		webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), "https://www.duckduckgo.com");
	}
	g_free(start_page);
}

// Initialize GTK
static void activate(GtkApplication *app, gpointer user_data) {
	GtkWidget *window, *vbox, *hbox, *entry, *go_button, *back_button, *webview, *scroll;

	// GTK Dark theme (Your'e Wellcome)
	GtkSettings *settings = gtk_settings_get_default();
	g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);

	// Main Window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Mono Browser");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	// Vertical Layout (SearchBar + Web)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	// Horizontal Layout (SearchBar)
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	// "Back" Button (←)
	back_button = gtk_button_new_with_label("←");
	gtk_box_pack_start(GTK_BOX(hbox), back_button, FALSE, FALSE, 0);

	// SearchBar (GtkEntry)
	entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a URL or search...");
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

	// "Go" Button (🔍)
	go_button = gtk_button_new_with_label("🔍");
	gtk_box_pack_start(GTK_BOX(hbox), go_button, FALSE, FALSE, 0);

	// WebKit WebView
	webview = webkit_web_view_new();
	create_first_tab(WEBKIT_WEB_VIEW(webview)); // Load InitPage

	// Scroll container for the browser
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll), webview);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

	// Save the WebView on HtkEntry for referencies
	g_object_set_data(G_OBJECT(entry), "webview", webview);

	// Conect events
	g_signal_connect(entry, "activate", G_CALLBACK(load_url), webview);
	g_signal_connect(go_button, "clicked", G_CALLBACK(on_button_clicked), entry);
	g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), webview);

	// Show All :)
	gtk_widget_show_all(window);
}

// Main Func
int main(int argc, char **argv) {
	GtkApplication *app;
	int status;

	app = gtk_application_new("com.ignis.monobrowser", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;

}
// be happy
