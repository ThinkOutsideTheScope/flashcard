#include <gtk/gtk.h>
#include <frontend/main.h>
#include <backend/popup.h>
void alert_popup(const char* str, void (*handler)(GtkDialog*, GAsyncResult*, gpointer)) {
	GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", str);
	gtk_alert_dialog_set_modal(dialog, TRUE);
	gtk_alert_dialog_set_buttons(dialog, (const char* ([])) {"_OK", NULL});
	if (handler) g_signal_connect(dialog, "response", G_CALLBACK(handler), NULL);
	gtk_alert_dialog_show(dialog, window);
}

void alert_popup(std::shared_ptr<char> str, void (*handler)(GtkDialog*, GAsyncResult*, gpointer)) {
	GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", str.get());
	gtk_alert_dialog_set_modal(dialog, TRUE);
	gtk_alert_dialog_set_buttons(dialog, (const char* ([])) {"_OK", NULL});
	if (handler) g_signal_connect(dialog, "response", G_CALLBACK(handler), NULL);
	gtk_alert_dialog_show(dialog, window);
}

void alert_popup_yn(const char* str, void (*handler)(GtkAlertDialog*, GAsyncResult*, gpointer)) {
	GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", str);
	gtk_alert_dialog_set_modal(dialog, TRUE);
	gtk_alert_dialog_set_buttons(dialog, (const char* ([])) {"_Yes", "_No", NULL});
	gtk_alert_dialog_choose(dialog, window, FALSE, (GAsyncReadyCallback)(handler), NULL);
}

void file_popup(const char* title, gboolean is_file_saver, void (*callback)(GtkFileDialog*, GAsyncResult*, gpointer)) {
	GtkFileDialog* dialog = gtk_file_dialog_new();
	gtk_file_dialog_set_modal(dialog, TRUE);
	if (title) gtk_file_dialog_set_title(dialog,title);
	if (is_file_saver) gtk_file_dialog_save(dialog, window, FALSE, (GAsyncReadyCallback)callback, NULL);
	else gtk_file_dialog_open(dialog, window, FALSE, (GAsyncReadyCallback)callback, NULL);
}