#ifndef __CSS_H__
#define __CSS_H__
#ifdef __cplusplus
#pragma once
#endif // __cplusplus
#include <gtk/gtk.h>
void ApplyCss(GtkWidget* widget, const char* name, GtkWidget* window, const char* css);
#endif // __CSS_H__