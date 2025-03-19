#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

// Function to load a URL or perform a DuckDuckGo search
static void load_url(GtkEntry *entry, gpointer user_data) {
	WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
	const gchar *url = gtk_entry_get_text(entry);

	// Check if the URL does not have "http://" or "https://"
	if (!g_str_has_prefix(url, "http://") && !g_str_has_prefix(url, "https://")) {
		// Check if the input looks like a valid domain (e.g., "example.com")
		if (g_strrstr(url, ".com") || g_strrstr(url, ".org") || g_strrstr(url, ".net") ||
			g_strrstr(url, ".gov") || g_strrstr(url, ".edu") || g_strrstr(url, ".io")) {
			// If it's a domain, prepend "https://" and load it
			gchar *full_url = g_strdup_printf("https://%s", url);
			webkit_web_view_load_uri(webview, full_url);
			g_free(full_url);
		} else {
			// If it's not a domain, perform a DuckDuckGo search
			gchar *search_url = g_strdup_printf("https://duckduckgo.com/?q=%s", url);
			webkit_web_view_load_uri(webview, search_url);
			g_free(search_url);
		}
	} else {
		// If the user entered a full URL, load it directly
		webkit_web_view_load_uri(webview, url);
	}
}

// Function triggered when the "Go" button is clicked
static void on_button_clicked(GtkButton *button, gpointer user_data) {
	GtkEntry *entry = GTK_ENTRY(user_data);
	load_url(entry, g_object_get_data(G_OBJECT(entry), "webview"));
}

// Function to navigate back to the previous page
static void on_back_button_clicked(GtkButton *button, gpointer user_data) {
	WebKitWebView *webview = WEBKIT_WEB_VIEW(user_data);
	if (webkit_web_view_can_go_back(webview)) {
		webkit_web_view_go_back(webview);
	}
}

// Function that initializes the GTK application
static void activate(GtkApplication *app, gpointer user_data) {
	GtkWidget *window, *vbox, *hbox, *entry, *go_button, *back_button, *webview, *scroll;

	// Enable dark mode using GTK
	GtkSettings *settings = gtk_settings_get_default();
	g_object_set(settings, "gtk-application-prefer-dark-theme", TRUE, NULL);

	// Create the main application window
	window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "Mono Browser");
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

	// Create a vertical box layout (contains the search bar and web view)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	// Create a horizontal box for the navigation bar
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	// Create the "Back" button (←)
	back_button = gtk_button_new_with_label("←");
	gtk_box_pack_start(GTK_BOX(hbox), back_button, FALSE, FALSE, 0);

	// Create the search bar (GtkEntry)
	entry = gtk_entry_new();
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Enter a URL or search...");
	gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);

	// Create the "Go" button (🔍︎)
	go_button = gtk_button_new_with_label("🔍︎");
	gtk_box_pack_start(GTK_BOX(hbox), go_button, FALSE, FALSE, 0);

	// Create the WebKit web view
	webview = webkit_web_view_new();
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(webview), "https://start.duckduckgo.com/");

	// Create a scrollable container for the web view
	scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scroll), webview);
	gtk_box_pack_start(GTK_BOX(vbox), scroll, TRUE, TRUE, 0);

	// Store the webview pointer inside the GtkEntry for later use
	g_object_set_data(G_OBJECT(entry), "webview", webview);

	// Connect signals (event listeners)
	g_signal_connect(entry, "activate", G_CALLBACK(load_url), webview); // Load when Enter is pressed
	g_signal_connect(go_button, "clicked", G_CALLBACK(on_button_clicked), entry); // Load when "Go" is clicked
	g_signal_connect(back_button, "clicked", G_CALLBACK(on_back_button_clicked), webview); // Navigate back

	// Show all widgets
	gtk_widget_show_all(window);
}

// Main function: initializes GTK and runs the application
int main(int argc, char **argv) {
	GtkApplication *app;
	int status;

	// Create a new GTK application
	app = gtk_application_new("com.ignis.monobrowser", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

	// Run the application
	status = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);

	return status;
}
