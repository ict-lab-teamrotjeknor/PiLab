#ifndef _PILAB_STRING_H
#define _PILAB_STRING_H
#include <unistd.h>

extern void string_to_upper(char *string);
extern void string_to_lower(char *string);
extern int string_charcmp(const char *string1, const char *string2);
extern int string_charcasecmp(const char *string1, const char *string2);
extern int string_strcmp(const char *string1, const char *string2);
extern int string_strncmp(const char *string1, const char *string2, size_t n);
extern int string_strcasecmp(const char *string1, const char *string2);
extern int string_strcasencmp(const char *string1, const char *string2,
			      size_t n);
extern char *string_strdup(const char *string);
extern char *string_strcat_delimiter(const char *string1, const char *string2,
				     const char *delimiter);

extern char *string_strcat_delimiter_recursive(char *string1,
					       const char *delimiter, int depth,
					       ...);
extern char *string_strcat(const char *string1, const char *string2);
extern char *string_strncat(const char *string1, const char *string2, size_t n);
extern struct t_pilist *string_split(const char *string, const char *delimiter);
extern char *string_split_first(char *string, const char *delimiter);
extern char *string_split_last(char *string, const char *delimiter);
extern int string_find_first(const char *string1, const char *string2);
extern char *string_replace_first(const char *string1, const char *string2,
				  const char *string3);
extern char *string_strip_whitespace(char *string);
extern char *string_read_until(const char *string1, const char *string2);
#endif
