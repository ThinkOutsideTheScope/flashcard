#include <gtk/gtk.h>
#include "frontend/main.h"

GtkApplication* app;

static void activate(GtkApplication* app, gpointer user_data) {
  ::app = app;
  
  do_flashcard_rt();
}

int main(int argc,char** argv) {
  gtk_init();
  GtkApplication *app;
  int status;

  app = gtk_application_new("com.example.flashcard", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}
