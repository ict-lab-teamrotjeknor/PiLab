#ifndef _PILAB_LOG_H
#define _PILAB_LOG_H
#include <stdbool.h>
#include <stddef.h>

typedef enum {
	LOG_SILENT = 0,
	LOG_ERROR = 1,
	LOG_WARNING = 2,
	LOG_NOTICE = 3,
	LOG_INFO = 4,
	LOG_DEBUG = 5,
} t_pilab_log_priority;

typedef enum {
	OFF = 0,
	ON = 1,
} t_pilab_log_colors;

extern void pilab_log_init(t_pilab_log_priority priority);
extern void pilab_log_set_priority(t_pilab_log_priority priority);
extern void pilab_log_set_colors(t_pilab_log_colors mode);
extern void pilab_log_toggle_colors(void);
extern t_pilab_log_priority logger_get_priority(void);
extern size_t _localtime(char *buf, size_t size);

extern void _pilab_log_abort(const char *filepath, int line, const char *format,
			     ...) __attribute__((format(printf, 3, 4)));
#define pilab_log_abort(FMT, ...)                                              \
	_pilab_log_abort(__FILE__, __LINE__, FMT, ##__VA_ARGS__)

extern void _pilab_log(const char *filepath, int line,
		       t_pilab_log_priority priority, const char *format, ...)
	__attribute__((format(printf, 4, 5)));
#define pilab_log(PRIORITY, FMT, ...)                                          \
	_pilab_log(__FILE__, __LINE__, PRIORITY, FMT, ##__VA_ARGS__)

extern bool _pilab_log_assert(bool condition, const char *filepath, int line,
			      const char *format, ...)
	__attribute__((format(printf, 4, 5)));
#define pilab_log_assert(COND, FMT, ...)                                       \
	_pilab_log_assert(COND, __FILE__, __LINE__, "%s: " FMT,                \
			  __PRETTY_FUNCTION__, ##__VA_ARGS__)
#endif
