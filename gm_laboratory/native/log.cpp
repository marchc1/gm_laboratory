#include "native/log.h"

#include <cstdarg>
#include <cstdio>

#include <windows.h>

namespace gm_laboratory {

void Log(const char* category, const char* fmt, ...) {
	char message[1024];
	va_list args;
	va_start(args, fmt);
	_vsnprintf_s(message, _TRUNCATE, fmt, args);
	va_end(args);

	char line[1152];
	_snprintf_s(line, _TRUNCATE, "[%s] %s", category, message);

	std::printf("%s", line);
}

}
