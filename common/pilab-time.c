#define _XOPEN_SOURCE 500
#include "pilab-time.h"
#include "pilab-log.h"
#include "pilab-string.h"

/*
 * Convert a date to seconds time_t struct.
 */

time_t time_to_sec(const char *date, const char *format)
{
	struct tm storage = { 0 };
	char *p;
	time_t retval;

	p = (char *)strptime(date, format, &storage);

	retval = (!p) ? 0 : mktime(&storage);

	return retval;
}

/*
 * Compares two date strings using a specific format.
 *
 * Returns:
 * -2: something else went wrong.
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int time_cmpstr_fmt(const char *format, const char *date1, const char *date2)
{
	time_t d1;
	time_t d2;

	d1 = time_to_sec(date1, format);
	d2 = time_to_sec(date2, format);

	if (d1 == 0 || d2 == 0)
		return -2;

	if (date1 < date2)
		return -1;
	else if (date1 == date2)
		return 0;
	else if (date1 > date2)
		return 1;

	return -2;
}

/*
 * Compares two date strings.
 *
 * Returns:
 * -2: something else went wrong.
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int time_cmpstr(const char *date1, const char *date2)
{
	return time_cmpstr_fmt(PILAB_TIME_DEFAULT_FORMAT, date1, date2);
}

/*
 * Get current time with specified format.
 *
 * NOTE: The time string needs to be cleaned afterwards.
 *
 * Returns pointer to the new time string, NULL otherwise.
 */

char *time_get_time_fmt(const char *fmt)
{
	time_t rawtime;
	struct tm *timeinfo;
	char out[200];

	if (!fmt)
		return NULL;

	time(&rawtime);
	timeinfo = gmtime(&rawtime);
	if (strftime(out, sizeof(out), fmt, timeinfo) == 0) {
		pilab_log(LOG_DEBUG, "Could not retrieve time in fmt %s:",
			  PILAB_TIME_DEFAULT_FORMAT);
		return NULL;
	}

	return string_strdup(out);
}

/*
 * Get current time.
 *
 * NOTE: The time string needs to be cleaned afterwards.
 *
 * Returns pointer to the new time string, NULL otherwise.
 */

char *time_get_time()
{
	return time_get_time_fmt(PILAB_TIME_DEFAULT_FORMAT);
}
