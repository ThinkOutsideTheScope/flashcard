#include <backend/io.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <vector>
#include <frontend/main.h>

gchar* app_data_dir;

void mkdir_userdir() {
	app_data_dir = g_build_filename(
	g_get_user_data_dir(),
	"flashcard",
	"sets",
	NULL);

	if (g_mkdir_with_parents(app_data_dir, 0700) != 0) g_error("Failed to create data directory");
}

bool wrdata(const char* filename, const char* data, std::size_t len) {
	gchar* path = g_build_filename(app_data_dir, filename, NULL);

	gboolean failed = g_file_set_contents(path, data, len, NULL); 
	g_free(path);
	return failed;
}

bool wrdata(const char* filename, const char* data) {
	return wrdata(filename, data, -1);
}

terminated_buffer<char> rddata(const char* filename) {
	gchar* path = g_build_filename(app_data_dir, filename, NULL);
	char* contents = nullptr;
	gsize length = 0;

	if (g_file_get_contents(path, &contents, &length, NULL)) return terminated_buffer<char>(contents, length);
	else return terminated_buffer<char>(NULL, 0);
}

std::vector<const char*> scan_dir_with_prefix(const char *prefix)
{
	std::vector<const char*> vec;
	GFile *dir = g_file_new_for_path(app_data_dir);
	GError *error = NULL;

	GFileEnumerator *enumerator =
		g_file_enumerate_children(
			dir,
			G_FILE_ATTRIBUTE_STANDARD_NAME,
			G_FILE_QUERY_INFO_NONE,
			NULL,
			&error
		);

	if (!enumerator) {
		g_printerr("Error opening directory: %s\n", error->message);
		g_error_free(error);
		g_object_unref(dir);
		return vec;
	}

	GFileInfo *info;
	while ((info = g_file_enumerator_next_file(enumerator, NULL, &error))) {
		const char *name = g_file_info_get_name(info);

		if (g_str_has_prefix(name, prefix)) {
			vec.push_back(name);
		}

		g_object_unref(info);
	}

	if (error) {
		g_printerr("Enumeration error: %s\n", error->message);
		g_error_free(error);
	}

	g_object_unref(enumerator);
	g_object_unref(dir);
	g_print("%s", app_data_dir);
	return vec;
}