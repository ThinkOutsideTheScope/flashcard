#include "backend/backend.h"

void ApplyCss(GtkWidget* widget, const char* name, GtkWidget* window, const char* css) {
	GtkCssProvider* css_provider = gtk_css_provider_new();
	gtk_css_provider_load_from_string(css_provider, css);
	gtk_style_context_add_provider_for_display(gtk_widget_get_display(window), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
	gtk_widget_add_css_class(widget, name);
	g_object_unref(css_provider);
}