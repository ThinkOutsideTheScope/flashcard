#ifndef __POPUP_H__
#define __POPUP_H__
#ifdef __cplusplus
#pragma once
#endif // __cplusplus
#include <gtk/gtk.h>
#include <memory>
void alert_popup(const char* str, void (*handler)(GtkDialog*, GAsyncResult*, gpointer));
void alert_popup(std::shared_ptr<char> str, void (*handler)(GtkDialog*, GAsyncResult*, gpointer));
void alert_popup_yn(const char* str, void (*handler)(GtkAlertDialog*, GAsyncResult*, gpointer));
void file_popup(const char* title, gboolean is_file_saver, void (*callback)(GtkFileDialog*, GAsyncResult*, gpointer));
#endif // __POPUP_H__