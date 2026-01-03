#include <backend/misc.h>
#include <cstdio>
std::shared_ptr<char> ssnprintf(gsize max, const char* fmt, ...) {
	va_list args;
	std::shared_ptr<char> buf(new char[max]);
	va_start(args, fmt);
	vsnprintf(buf.get(), max, fmt, args);
	va_end(args);
	return buf;
}