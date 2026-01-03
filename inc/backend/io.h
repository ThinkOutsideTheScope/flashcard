#ifndef __IO_H__
#define __IO_H__
#ifndef __cplusplus
#error io.h requires a C++ environment
#endif // __cplusplus
#pragma once
#include <glib.h>
#include <vector>
template <typename T>
class terminated_buffer {
	private:
	T* buffer;
	gsize length;
	public:
	terminated_buffer(T* buffer, gsize length) : buffer(buffer), length(length) {}
	~terminated_buffer() {}
	T& operator[](gsize index) {
		if (index >= length) g_error("Error: attempted to access out-of-bounds of terminated buffer.");
		else return buffer[index];
	}
	gsize get_size() { return length; }

	T* get_buffer() { return buffer; }

	gboolean buf_is_null() {
		return (buffer == NULL);
	}
};

void mkdir_userdir();
bool wrdata(const char* filename, const char* data, std::size_t len);
bool wrdata(const char* filename, const char* data);
terminated_buffer<char> rddata(const char* filename);
std::vector<const char*> scan_dir_with_prefix(const char *prefix);

#endif // __IO_H__