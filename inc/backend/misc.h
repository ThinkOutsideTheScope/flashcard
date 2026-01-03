#ifndef __MISC_H__
#define __MISC_H__
#ifdef __cplusplus
#pragma once
#endif // __cplusplus
#include <gtk/gtk.h>
#include <memory>

std::shared_ptr<char> ssnprintf(gsize max, const char* fmt, ...);

struct _mid_serialize_buf {
	char* buf;
	gsize size;
};

struct flashcard_flags {
	bool in_use = FALSE;
	bool tested_twice = FALSE;
};

inline constexpr bool is_tested_twice(const flashcard_flags f) {
	return (f.in_use && f.tested_twice);
}

struct flashcard {
	GtkTextBuffer* front;
	GtkTextBuffer* back;
	flashcard_flags flags;
};

#endif // __MISC_H__