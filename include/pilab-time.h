#ifndef _PILAB_TIME_H
#define _PILAB_TIME_H
#include <time.h>

#define PILAB_TIME_DEFAULT_FORMAT "%a, %m %b %Y %H:%M:%S"

extern time_t time_to_sec(const char *date, const char *fmt);
extern int time_cmpstr_fmt(const char *fmt, const char *date1,
			   const char *date2);
extern int time_cmpstr(const char *date1, const char *date2);
extern char *time_get_time_fmt(const char *fmt);
extern char *time_get_time(void);

#endif
