#ifndef _PILAB_STRINGBUILDER_H
#define _PILAB_STRINGBUILDER_H
#include <stdint.h>

#define GROW_FACTOR 2

typedef uint32_t string_dyn_size_t;

struct t_stringbuilder {
	/* allocated size */
	string_dyn_size_t size_alloc;
	/* length of the string (without '\0') */
	string_dyn_size_t length;
	/* the string */
	char *string;
};

struct t_stringbuilder *stringbuilder_create_size(size_t alloc_size);
struct t_stringbuilder *stringbuilder_create(void);
extern void stringbuilder_free(struct t_stringbuilder *sb);
extern char *stringbuilder_resize(struct t_stringbuilder *sb, size_t capacity);
extern void stringbuilder_append(struct t_stringbuilder *sb, const char *str);
extern void stringbuilder_prepend(struct t_stringbuilder *sb, const char *str);
extern char *stringbuilder_to_string(struct t_stringbuilder *sb);

#endif
