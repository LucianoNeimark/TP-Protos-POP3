#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/**
 * Implementación de "logger.h".
 */

void Log(FILE * const stream, const char * prefix, const char * const format, const char * suffix, va_list arguments) {
    time_t loginternal_time = time(NULL); \
    struct tm loginternal_tm = *localtime(&loginternal_time); \
	fprintf(stream, "%s", prefix);
    //agregar la fecha y la hora
    fprintf(stream, "[%d-%02d-%02d %02d:%02d:%02d] ", loginternal_tm.tm_year + 1900, loginternal_tm.tm_mon + 1, loginternal_tm.tm_mday, loginternal_tm.tm_hour, loginternal_tm.tm_min, loginternal_tm.tm_sec);
	vfprintf(stream, format, arguments);
	fprintf(stream, "%s", suffix);
}

void LogDebug(const char * const format, ...) {
	if(DEBUG == true){
	va_list arguments;
	va_start(arguments, format);
	Log(stdout, "[DEBUG] ", format, "\n", arguments);
	va_end(arguments);
	}
}

void LogError(const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	Log(stderr, "[ERROR] ", format, "\n", arguments);
	va_end(arguments);
}

void LogErrorRaw(const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	Log(stderr, "", format, "", arguments);
	va_end(arguments);
}

void LogInfo(const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	Log(stdout, "[INFO] ", format, "\n", arguments);
	va_end(arguments);
}

void LogRaw(const char * const format, ...) {
	va_list arguments;
	va_start(arguments, format);
	Log(stdout, "", format, "", arguments);
	va_end(arguments);
}

void LogText(const char * text, const int length) {
	if(DEBUG == false){
		return;
	}
	for (int i = 0; i < length; ++i) {
		switch (text[i]) {
			case '\0':
				LogRaw("\\0");
				break;
			case '\n':
				LogRaw("\\n");
				break;
			case '\r':
				LogRaw("\\r");
				break;
			case '\t':
				LogRaw("\\t");
				break;
			default:
				LogRaw("%c", text[i]);
		}
	}
}
