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
extern int string_strcasencmp(const char *string1, const char *string2, size_t n);
extern char *string_strdup(const char *string);

#endif
