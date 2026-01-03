#ifndef __MAIN_H__
#define __MAIN_H__
#ifdef __cplusplus
#pragma once
#endif // __cplusplus
#include <string>
typedef GtkWindow* GTKWindow;
extern GTKWindow window;
extern GtkApplication* app;
extern gpointer user_data;
void do_flashcard_rt();

#endif // __MAIN_H__