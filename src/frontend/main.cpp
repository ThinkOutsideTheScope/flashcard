#include "backend/backend.h"
#include "frontend/main.h"
#include <vector>
#include <algorithm>
#include <cctype>
#include <gio/gio.h>
#include <rapidfuzz/fuzz.hpp>
#include <numeric>

GTKWindow window;
GtkTextView* input;
GtkTextView* input2;
GtkLabel* flashcard_id;
flashcard current;
bool is_back = FALSE;
std::vector<flashcard> flashcards;
bool is_test_mode = FALSE;
unsigned long current_flashcard_num = 1;
std::vector<double> accuracy_per_flashcard;
void save_set_impl(std::vector<flashcard> f, GFile* file);

extern "C" G_MODULE_EXPORT void next_flashcard(GtkButton* button, gpointer data) {
	GtkTextBuffer* front_buffer = current.front;
	GtkTextIter start, end;
	gtk_text_buffer_get_bounds(front_buffer, &start, &end);
	if (gtk_text_iter_equal(&start, &end)) {
		alert_popup("The front of a flashcard cannot be blank", NULL);
		return;
	}
	is_back = FALSE;
	GtkWidget* parent = gtk_widget_get_parent(GTK_WIDGET(button));
	GtkWidget* child = gtk_widget_get_first_child(parent);

	while (child) {
		if (g_strcmp0(gtk_widget_get_name(child), "sw_face") == 0) {
			gtk_button_set_label(GTK_BUTTON(child), "Switch to back face ");
			break;
		}
		child = gtk_widget_get_next_sibling(child);
	}

	if (current_flashcard_num-1 >= flashcards.size() || current_flashcard_num >= flashcards.size()) {
		flashcards.push_back(current);
		current.front = gtk_text_buffer_new(NULL);
		current.back = gtk_text_buffer_new(NULL);
		current.flags.in_use = TRUE;
		current.flags.tested_twice = FALSE;
		g_object_ref(current.front);
		g_object_ref(current.back);
		gtk_text_view_set_buffer(input, current.front);
		current_flashcard_num++;
	} else {
		current_flashcard_num++;
		current.front = flashcards[current_flashcard_num-1].front;
		current.back = flashcards[current_flashcard_num-1].back;
		current.flags = flashcards[current_flashcard_num-1].flags;
		gtk_text_view_set_buffer(input, current.front);
	}
	gtk_label_set_label(flashcard_id, ("Flashcard " + std::to_string(current_flashcard_num)).c_str());
}

extern "C" G_MODULE_EXPORT void confirm_all(GtkButton* button, gpointer data) {
	is_back = 0;
	flashcards.push_back(current);
	current.front = flashcards[0].front;
	current.back = flashcards[0].back;
	current.flags.in_use = TRUE;
	current.flags.tested_twice = FALSE;
	gtk_text_view_set_buffer(input, current.front);
	gtk_text_view_set_buffer(input2, gtk_text_buffer_new(NULL));
	alert_popup_yn("Would you like to save this set?", [](GtkAlertDialog* dialog, GAsyncResult* response, gpointer data){
		GError* error = NULL;
		int res = gtk_alert_dialog_choose_finish(dialog, response, &error);
		if (error && error->code && error->domain) {
			if ((error->domain == 1698 && error->code == 2) || (error->domain == 1714 && error->code == 2) || (error->domain == 1866 && error->code == 2)) {
				g_error_free(error);
				return;
			} else if (error->message) alert_popup(ssnprintf(128, "Error getting response. %s", error->message), NULL);
			else alert_popup("Error getting response.", NULL);
			g_error_free(error);
			return;
		}
		if (res == 0 /* Yes */) {
			file_popup("Open File", TRUE, [](GtkFileDialog* dialog, GAsyncResult* res, gpointer user_data){
				GError* error = NULL;
				GFile* file = gtk_file_dialog_save_finish(dialog, res, &error);
				if (error && error->code && error->domain) {
					if ((error->domain == 1698 && error->code == 2) || (error->domain == 1714 && error->code == 2) || (error->domain == 1866 && error->code == 2)) {
						g_error_free(error);
						return;
					} else if (error->message) alert_popup(ssnprintf(128, "Error saving file. %s", error->message), NULL);
					else alert_popup("Error saving file.", NULL);
					g_error_free(error);
					return;
				}
				else save_set_impl(flashcards, file);
			});
		}
	});
	is_test_mode = TRUE;
	GtkBox* buttons = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(button)));
	GtkWidget* child = gtk_widget_get_first_child(GTK_WIDGET(buttons));
	GtkWidget* root_box = gtk_widget_get_parent(GTK_WIDGET(buttons));
	GtkWidget* root_child = gtk_widget_get_first_child(GTK_WIDGET(root_box));
	while (child) {
		if (g_strcmp0(gtk_widget_get_name(child), "check_answ") == 0 || g_strcmp0(gtk_widget_get_name(child), "skip") == 0 || g_strcmp0(gtk_widget_get_name(child), "back") == 0)
			gtk_widget_set_visible(child, TRUE);
		else gtk_widget_set_visible(child, FALSE);
		child = gtk_widget_get_next_sibling(child);
	}
	while (root_child) {
		if (g_strcmp0(gtk_widget_get_name(root_child), "test_mode_input") == 0) gtk_widget_set_visible(root_child, TRUE);
		if (g_strcmp0(gtk_widget_get_name(root_child), "input") == 0) gtk_text_view_set_editable(GTK_TEXT_VIEW(root_child), FALSE);
		if (g_strcmp0(gtk_widget_get_name(root_child), "flashcard_id") == 0) {
			gtk_widget_set_visible(root_child, TRUE);
			gtk_label_set_label(GTK_LABEL(root_child), "Flashcard 1");
		}
		root_child = gtk_widget_get_next_sibling(root_child);
	}
	current_flashcard_num = 1;
}

extern "C" G_MODULE_EXPORT void previous_flashcard(GtkButton* button, gpointer data) {
	if (current_flashcard_num <= 1 || flashcards.empty()) return;
	current_flashcard_num--;
	current.front = flashcards[current_flashcard_num-1].front;
	current.back = flashcards[current_flashcard_num-1].back;
	current.flags = flashcards[current_flashcard_num-1].flags;
	gtk_label_set_label(flashcard_id, ("Flashcard " + std::to_string(current_flashcard_num)).c_str());
	is_back = FALSE;
	gtk_text_view_set_buffer(input, current.front);
}

extern "C" G_MODULE_EXPORT void sw_face(GtkButton* button, gpointer data) {
	gtk_button_set_label(button, (is_back ? "Switch to back face " : "Switch to front face"));
	if (is_back) gtk_text_view_set_buffer(input, current.front);
	else gtk_text_view_set_buffer(input, current.back);
	is_back = !is_back;
}

extern "C" G_MODULE_EXPORT void new_set_impl(GtkButton* button, gpointer data) {
   gtk_widget_set_visible(GTK_WIDGET(button), FALSE);
   GtkBox* root_box = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(button)));
   GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(root_box));

	while (child) {
		if (g_strcmp0(gtk_widget_get_name(child), "load_set") == 0 || g_strcmp0(gtk_widget_get_name(child), "exit") == 0) {
			gtk_widget_set_visible(child, FALSE);
		} else if (g_strcmp0(gtk_widget_get_name(child), "flashcard_id") == 0) {
			gtk_widget_set_visible(child, TRUE);
		} else if (g_strcmp0(gtk_widget_get_name(child), "input") == 0) {
			gtk_widget_set_visible(child, TRUE);
			gtk_text_view_set_editable(GTK_TEXT_VIEW(child), TRUE);
		} else if (g_strcmp0(gtk_widget_get_name(child), "buttons") == 0) {
			gtk_widget_set_visible(child, TRUE);
			GtkWidget* buttons_child = gtk_widget_get_first_child(child);

			while (buttons_child) {
				if (g_strcmp0(gtk_widget_get_name(buttons_child), "check_answ") && g_strcmp0(gtk_widget_get_name(buttons_child), "skip") && g_strcmp0(gtk_widget_get_name(buttons_child), "back")) {
					gtk_widget_set_visible(buttons_child, TRUE);
				} else {
					gtk_widget_set_visible(buttons_child, FALSE);
				}
				buttons_child = gtk_widget_get_next_sibling(buttons_child);
			}
		} else if (g_strcmp0(gtk_widget_get_name(child), "test_mode_input") == 0) {
			gtk_widget_set_visible(child, FALSE);
		}
		child = gtk_widget_get_next_sibling(child);
	}


}

#define MAGIC (0xa1b2c3d4U) // example file magic bytes that this app will use

void load_set(GFile* file) {
	char* filename = g_file_get_path(file);
	gchar* file_data_raw;
	gsize file_data_length = 0;
	gboolean success = g_file_get_contents(filename, &file_data_raw, &file_data_length, NULL);
	if (!success) {
		alert_popup("Failed to read file contents.", NULL);
		return;
	}

	if (file_data_length < sizeof(MAGIC) || *((decltype(MAGIC)*)file_data_raw) != MAGIC) {
		alert_popup("File is not a valid Flashcard data file.", NULL);
		return;
	}

	std::vector<char*> strings;

	for (gsize i = sizeof(MAGIC), last_string = sizeof(MAGIC); i < file_data_length; i++) {
		if (file_data_raw[i] == '\0') {
			strings.push_back(&(file_data_raw[last_string]));
			last_string = i + 1;
		}
	}

	std::vector<std::pair<char*, char*>> as_pair;

	std::pair<char*, char*> current_pair = std::make_pair<char*, char*>(NULL, NULL);

	for (char* i : strings) {
		if (current_pair.first == NULL) current_pair.first = i;
		else {
			current_pair.second = i;
			as_pair.push_back(current_pair);
			current_pair = std::make_pair<char*, char*>(NULL, NULL);
		}
	}
	flashcards.clear();
	for (std::pair<char*, char*>& i : as_pair) {
		GtkTextBuffer* front = gtk_text_buffer_new(NULL);
		GtkTextBuffer* back = gtk_text_buffer_new(NULL);
		gtk_text_buffer_set_text(front, i.first, -1);
		gtk_text_buffer_set_text(back, i.second, -1);
		flashcard f;
		f.front = front;
		f.back = back;
		f.flags.in_use = TRUE;
		flashcards.push_back(f);
	}
	current.front = flashcards[0].front;
	current.back = flashcards[0].back;
	current.flags = flashcards[0].flags;
	for (flashcard i : flashcards) {
		g_object_ref(i.front);
		g_object_ref(i.back);
	}
	gtk_text_view_set_buffer(input, current.front);
	current_flashcard_num = 1;
	gtk_label_set_label(flashcard_id, "Flashcard 1");
	alert_popup_yn("Would you like to edit the set now?", [](GtkAlertDialog* dialog, GAsyncResult* response, gpointer data){
		GError* error = NULL;
		int result = gtk_alert_dialog_choose_finish(dialog, response, &error);
		if (error) {
			if ((error->domain == 1698 && error->code == 2) || (error->domain == 1714 && error->code == 2) || (error->domain == 1866 && error->code == 2)) return;
			else if (error->message) alert_popup(ssnprintf(128, "Failed to get response. %s", error->message), NULL);
			else alert_popup(ssnprintf(128, "Failed to get response"), NULL);
			return;
		}
		if (result == 0) {
			is_test_mode = FALSE;
			gtk_text_view_set_editable(input, TRUE);
			current_flashcard_num = 1;
			accuracy_per_flashcard.clear();
			GtkBox* root_box = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(input)));
   			GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(root_box));

			while (child) {
				if (g_strcmp0(gtk_widget_get_name(child), "new_set") == 0 || g_strcmp0(gtk_widget_get_name(child), "load_set") == 0 || g_strcmp0(gtk_widget_get_name(child), "exit") == 0) {
					gtk_widget_set_visible(child, FALSE);
				} else if (g_strcmp0(gtk_widget_get_name(child), "flashcard_id") == 0) {
					gtk_widget_set_visible(child, TRUE);
				} else if (g_strcmp0(gtk_widget_get_name(child), "input") == 0) {
					gtk_widget_set_visible(child, TRUE);
					gtk_text_view_set_editable(GTK_TEXT_VIEW(child), TRUE);
				} else if (g_strcmp0(gtk_widget_get_name(child), "buttons") == 0) {
					gtk_widget_set_visible(child, TRUE);
					GtkWidget* buttons_child = gtk_widget_get_first_child(child);

					while (buttons_child) {
						if (g_strcmp0(gtk_widget_get_name(buttons_child), "check_answ") && g_strcmp0(gtk_widget_get_name(buttons_child), "skip") && g_strcmp0(gtk_widget_get_name(buttons_child), "back")) {
							gtk_widget_set_visible(buttons_child, TRUE);
						} else {
							gtk_widget_set_visible(buttons_child, FALSE);
						}
						buttons_child = gtk_widget_get_next_sibling(buttons_child);
					}
				} else if (g_strcmp0(gtk_widget_get_name(child), "test_mode_input") == 0) {
					gtk_widget_set_visible(child, FALSE);
				}
				child = gtk_widget_get_next_sibling(child);
			}
		} else {
			is_test_mode = TRUE;
			is_back = FALSE;
			GtkBox* root_box = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(input)));
   			GtkWidget *root_child = gtk_widget_get_first_child(GTK_WIDGET(root_box));

			while (root_child) {
				if (g_strcmp0(gtk_widget_get_name(root_child), "test_mode_input") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "flashcard_id") == 0) gtk_widget_set_visible(root_child, TRUE);
				else if (g_strcmp0(gtk_widget_get_name(root_child), "input") == 0) {
					gtk_widget_set_visible(root_child, TRUE);
					gtk_text_view_set_editable(GTK_TEXT_VIEW(root_child), FALSE);
					gtk_text_view_set_buffer(GTK_TEXT_VIEW(root_child), flashcards[0].front);
				}
				else if (g_strcmp0(gtk_widget_get_name(root_child), "buttons") == 0) {
					GtkWidget* buttons_child = gtk_widget_get_first_child(root_child);
					gtk_widget_set_visible(root_child, TRUE);
					while (buttons_child) {
						if (g_strcmp0(gtk_widget_get_name(buttons_child), "check_answ") == 0 || g_strcmp0(gtk_widget_get_name(buttons_child), "skip") == 0 || g_strcmp0(gtk_widget_get_name(buttons_child), "back") == 0)
							gtk_widget_set_visible(buttons_child, TRUE);
						else gtk_widget_set_visible(buttons_child, FALSE);
						buttons_child = gtk_widget_get_next_sibling(buttons_child);
					}
				} else if (g_strcmp0(gtk_widget_get_name(root_child), "new_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "load_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "exit")) gtk_widget_set_visible(root_child, FALSE);
				root_child = gtk_widget_get_next_sibling(root_child);
			}
		}
	});

	g_free(filename);
	g_free(file_data_raw);
}

void _load_set_impl(GtkButton* button) {
	file_popup("Select a file", FALSE, [](GtkFileDialog* dialog, GAsyncResult* res, gpointer){
		GError* error = NULL;
		GFile* file = gtk_file_dialog_open_finish(dialog, res, &error);
		if (error && error->code && error->domain) {
			if ((error->domain == 1698 && error->code == 2) || (error->domain == 1714 && error->code == 2) || (error->domain == 1866 && error->code == 2)) {
				g_error_free(error);
				return;
			}
			else if (error->message) alert_popup(ssnprintf(128, "Error reading file. %s", error->message), NULL);
			else alert_popup(ssnprintf(128, "Error reading file."), NULL);
			g_error_free(error);
			return;
		}
		else load_set(file);
	});
}


extern "C" G_MODULE_EXPORT void load_set_impl(GtkButton* button, gpointer data) {
	_load_set_impl(button);
}

bool chk_answ_btn_label_is_NEXT = FALSE;
GtkLabel* is_correct;
GtkCssProvider* is_correct_css;

#define CORRECT_THRESHOLD 75.0

void next_impl(GtkButton* button) {
	std::pair<flashcard*, gsize> first_incorrect;
	if (button) gtk_button_set_label(button, "Check Answer");
	gtk_widget_set_visible(GTK_WIDGET(is_correct), FALSE);
	if (accuracy_per_flashcard.size() >= flashcards.size()) {
		for (int i = 0; i < accuracy_per_flashcard.size(); i++) {
			if (accuracy_per_flashcard[i] < CORRECT_THRESHOLD && !(is_tested_twice(flashcards[i].flags))) {
				first_incorrect = std::make_pair(&(flashcards[i]), i);
				break;
			}
		}
		if (first_incorrect.first != NULL) {
			current_flashcard_num = first_incorrect.second+1;
			first_incorrect.first->flags.tested_twice = TRUE;
			current.front = first_incorrect.first->front;
			current.back = first_incorrect.first->back;
			current.flags.tested_twice = TRUE;
			gtk_text_view_set_buffer(input2, gtk_text_buffer_new(NULL));
			gtk_label_set_label(flashcard_id, (std::string("Flashcard ") + std::to_string(current_flashcard_num)).c_str());
		} else {
			double out = std::accumulate(accuracy_per_flashcard.begin(), accuracy_per_flashcard.end(), 0.0) / accuracy_per_flashcard.size();
			alert_popup(ssnprintf(128, "Congrats! You've finished this set. Your average score was %.1f%%.", out), NULL);

			GtkWidget* buttons = gtk_widget_get_parent(GTK_WIDGET(button));
			GtkWidget* root_box = gtk_widget_get_parent(buttons);
			GtkWidget* root_child = gtk_widget_get_first_child(root_box);
			gtk_widget_set_visible(GTK_WIDGET(buttons), FALSE);

			while (root_child) {
				if (g_strcmp0(gtk_widget_get_name(root_child), "new_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "load_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "exit") == 0) {
					gtk_widget_set_visible(root_child, TRUE);
				} else gtk_widget_set_visible(root_child, FALSE);
				root_child = gtk_widget_get_next_sibling(root_child);
			}
			
		}
	} else {
		current.front = flashcards[current_flashcard_num++].front;
		current.back = flashcards[current_flashcard_num-1].back;
		current.flags = flashcards[current_flashcard_num-1].flags;
		gtk_text_view_set_buffer(input, current.front);
		gtk_text_view_set_buffer(input2, gtk_text_buffer_new(NULL));
		gtk_label_set_label(flashcard_id, ("Flashcard " + std::to_string(current_flashcard_num)).c_str());
	}
}

extern "C" G_MODULE_EXPORT void check_answ_impl(GtkButton* button, gpointer data) {
	if (chk_answ_btn_label_is_NEXT) {
		next_impl(button);
		is_back = FALSE;
		chk_answ_btn_label_is_NEXT = FALSE;
		gtk_text_view_set_buffer(input, current.front);
	} else {
		chk_answ_btn_label_is_NEXT = TRUE;
		gtk_button_set_label(button, "    Next    ");

		GtkTextIter input_start, input_end,
			input2_start, input2_end;

		gtk_text_buffer_get_bounds(current.back, &input_start, &input_end);
		char* input_buf = gtk_text_buffer_get_text(current.back, &input_start, &input_end, FALSE);

		GtkTextBuffer* input2_gbuf = gtk_text_view_get_buffer(input2);
		gtk_text_buffer_get_bounds(input2_gbuf, &input2_start, &input2_end);
		char* input2_buf = gtk_text_buffer_get_text(input2_gbuf, &input2_start, &input2_end, FALSE);

		double score = rapidfuzz::fuzz::ratio(input_buf, input2_buf);
		if (!(accuracy_per_flashcard.size() >= flashcards.size())) accuracy_per_flashcard.push_back(score);
		else accuracy_per_flashcard[current_flashcard_num-1] = (score + accuracy_per_flashcard[current_flashcard_num-1]) / 2;

		static const char* correct = "Correct! %.1f%% accuracy.";
		static const char* incorrect = "Incorrect. %.1f%% accuracy.";

		char out_buf[128];

		if (score >= CORRECT_THRESHOLD) {
			snprintf(out_buf, 128, correct, score);
			gtk_label_set_label(is_correct, out_buf);

			gtk_widget_set_visible(GTK_WIDGET(is_correct), TRUE);
			gtk_css_provider_load_from_string(is_correct_css, "#is_correct { color: green; font-weight: bold; }");
		} else {
			snprintf(out_buf, 128, incorrect, score);
			gtk_label_set_label(is_correct, out_buf);
			gtk_widget_set_visible(GTK_WIDGET(is_correct), TRUE);
			gtk_css_provider_load_from_string(is_correct_css, "#is_correct { color: red; font-weight: bold; }");
		}

		is_back = TRUE;
		gtk_text_view_set_buffer(input, current.back);

		g_free(input2_buf);
		g_free(input_buf);

	}
}

#undef CORRECT_THRESHOLD

extern "C" G_MODULE_EXPORT void skip_impl(GtkButton* button, gpointer data) {
	if (chk_answ_btn_label_is_NEXT) return;
	next_impl(NULL);
	gtk_text_view_set_buffer(input, current.front);
	accuracy_per_flashcard.push_back(0.0);
}

extern "C" G_MODULE_EXPORT void back_impl(GtkButton* button, gpointer data) {
	GtkBox* buttons = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(button)));
	GtkBox* root_box = GTK_BOX(gtk_widget_get_parent(GTK_WIDGET(buttons)));
	GtkWidget* root_child = gtk_widget_get_first_child(GTK_WIDGET(root_box));

	gtk_widget_set_visible(GTK_WIDGET(buttons), FALSE);

	while (root_child) {
		if (g_strcmp0(gtk_widget_get_name(root_child), "new_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "load_set") == 0 || g_strcmp0(gtk_widget_get_name(root_child), "exit") == 0) {
			gtk_widget_set_visible(root_child, TRUE);
		} else gtk_widget_set_visible(root_child, FALSE);
		root_child = gtk_widget_get_next_sibling(root_child);
	}
}

extern "C" G_MODULE_EXPORT void exit_impl(GtkButton* button, gpointer data) {
	gtk_window_close(window);
}

void save_set_impl(std::vector<flashcard> f, GFile* file) {
	std::vector<_mid_serialize_buf> serialized;
	for (gsize i = 0; i < f.size() && i < 256; i++) {
		GtkTextBuffer *front = f[i].front, *back = f[i].back;
		GtkTextIter front_start, front_end, back_start, back_end;
		char *front_buf, *back_buf, *buf;
		gsize front_sz, back_sz;
		gtk_text_buffer_get_bounds(front, &front_start, &front_end);
		front_buf = gtk_text_buffer_get_text(front, &front_start, &front_end, FALSE);
		gtk_text_buffer_get_bounds(back, &back_start, &back_end);
		back_buf = gtk_text_buffer_get_text(back, &back_start, &back_end, FALSE);
		front_sz = strnlen(front_buf, 1024) + 1;
		back_sz = strnlen(back_buf, 1024) + 1;
		buf = (char*)g_malloc0(front_sz + back_sz);
		memcpy(buf, front_buf, front_sz);
		memcpy((buf + front_sz), back_buf, back_sz);

		_mid_serialize_buf packaged;
		packaged.buf = buf;
		packaged.size = (front_sz + back_sz);

		serialized.push_back(packaged);

		g_free(front_buf);
		g_free(back_buf);
	}

	gsize size = sizeof(MAGIC);
	for (gsize i = 0; i < serialized.size() && i < 256; i++) {
		size += serialized[i].size;
		if (size > (524800 + sizeof(MAGIC))) {
			alert_popup(ssnprintf(128, "Corruption detected: Illogical allocation size for g_malloc0 found."), NULL);
			return;
		}
	}

	char* final = (char*)g_malloc0(size);
	gsize index = sizeof(MAGIC);
	(*(decltype(MAGIC)*)final) = MAGIC;
	for (std::size_t i = 0; i < serialized.size(); i++) {
		memcpy((final + index), serialized[i].buf, serialized[i].size);
		g_free(serialized[i].buf);
		index += serialized[i].size;
	}
	char* file_str = g_file_get_path(file);
	gboolean success = g_file_set_contents(file_str, final, size, NULL);
	if (!success) alert_popup("Failed to write to file.", NULL);
	g_free(final);
	g_free(file_str);
}

#undef MAGIC

void destroy_impl(GtkWindow *window, gpointer user_data) {
	if (current.front && G_IS_OBJECT(current.front)) g_object_unref(current.front);
	if (current.back && G_IS_OBJECT(current.back)) g_object_unref(current.back);
	for (flashcard& i : flashcards) {
		if (i.front && G_IS_OBJECT(i.front)) g_object_unref(i.front);
		if (i.back && G_IS_OBJECT(i.back)) g_object_unref(i.back);
	}
}

void do_flashcard_rt() {
	GtkBuilder* builder = gtk_builder_new_from_resource("/com/example/flashcard/main.ui");
	window = GTK_WINDOW(gtk_builder_get_object(builder, "main_window"));
	gtk_window_set_application(window, app);
	is_correct_css = gtk_css_provider_new();
	gtk_css_provider_load_from_string(is_correct_css, "#is_correct_css { font-weight: bold; } ");
	gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(window)), GTK_STYLE_PROVIDER(is_correct_css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	GtkCssProvider* css = gtk_css_provider_new();
	gtk_css_provider_load_from_resource(css, "/com/example/flashcard/main.css");
	gtk_style_context_add_provider_for_display(gtk_widget_get_display(GTK_WIDGET(window)), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	input = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "input"));
	input2 = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "input2"));
	flashcard_id = GTK_LABEL(gtk_builder_get_object(builder, "flashcard_id"));
	is_correct = GTK_LABEL(gtk_builder_get_object(builder, "is_correct"));
	mkdir_userdir();
	current.front = gtk_text_view_get_buffer(input);
	current.back = gtk_text_buffer_new(NULL);
	current.flags.in_use = TRUE;
	current.flags.tested_twice = FALSE;
	g_object_ref(current.front);
	g_object_ref(current.back);

	gtk_window_present(window);
	g_object_unref(css);
	g_object_unref(builder);
	g_signal_connect(window, "destroy", G_CALLBACK(destroy_impl), NULL);
}
