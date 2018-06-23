#define _POSIX_C_SOURCE 1
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include "pilab-log.h"

static const char *pilab_log_priority_colors[] = {
	[LOG_SILENT] = "",
	[LOG_ERROR] = "\x1b[31m", // red
	[LOG_WARNING] = "\x1b[33m", // yellow
	[LOG_NOTICE] = "\x1b[34m", // blue
	[LOG_INFO] = "\x1b[35m", // magenta
	[LOG_DEBUG] = "\x1b[36m", // cyan
};

static t_pilab_log_priority p = LOG_SILENT;
static t_pilab_log_colors colors = ON;

void pilab_log_init(t_pilab_log_priority priority)
{
	p = priority;
}

void pilab_log_set_priority(t_pilab_log_priority priority)
{
	p = priority;
}

void pilab_log_set_colors(t_pilab_log_colors mode)
{
	colors = mode;
}

void pilab_log_toggle_colors()
{
	colors = (colors == ON) ? OFF : ON;
}

t_pilab_log_priority logger_get_priority()
{
	return p;
}

size_t _localtime(char *buf, size_t size)
{
	static time_t raw_t;
	static struct tm result;

	setlocale(LC_ALL, "");
	raw_t = time(NULL);
	localtime_r(&raw_t, &result);
	return strftime(buf, size, "%x %X - ", &result);
}

void _log(const char *filepath, int line, t_pilab_log_priority priority,
	  const char *format, va_list args)
{
	if (priority <= p) {
		char buf[26];
		_localtime(buf, 26);
		fprintf(stderr, "%s", buf);

		unsigned int c = priority;
		if (isatty(STDERR_FILENO) && colors)
			fprintf(stderr, "%s", pilab_log_priority_colors[c]);

		if (filepath && line) {
			const char *file = filepath + strlen(filepath);
			while (file != filepath && *file != '/')
				--file;
			if (*file == '/')
				++file;
			fprintf(stderr, "[%s:%d] ", file, line);
		}

		vfprintf(stderr, format, args);

		if (isatty(STDERR_FILENO) && colors)
			fprintf(stderr, "\x1b[0m");

		fprintf(stderr, "\n");
	}
}

bool _pilab_log_assert(bool condition, const char *filepath, int line,
		       const char *format, ...)
{
	if (condition)
		return true;

	va_list args;
	va_start(args, format);
	_log(filepath, line, LOG_ERROR, format, args);
	va_end(args);

#ifndef NDEBUG
	raise(SIGABRT);
#endif

	return false;
}

void _pilab_log(const char *filepath, int line, t_pilab_log_priority priority,
		const char *format, ...)
{
	va_list args;
	va_start(args, format);
	_log(filepath, line, priority, format, args);
	va_end(args);
}

void _pilab_log_abort(const char *filepath, int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	_log(filepath, line, LOG_ERROR, format, args);
	va_end(args);
	exit(EXIT_FAILURE);
}
