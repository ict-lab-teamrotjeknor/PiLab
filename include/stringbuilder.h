#ifndef _PILAB_STRINGBUILDER_H
#define _PILAB_STRINGBUILDER_H
#include <stdlib.h>

#define SB_OK    0
#define SB_ERR  -1

#define GROW_FACTOR 2

struct stringbuilder {
	size_t capacity;
	size_t length;
	char* string;
};

struct stringbuilder* string_builder_create();
int string_builder_free(struct stringbuilder* sb);
char* string_builder_append(struct stringbuilder* sb, const char* str);
char* string_builder_to_string(struct stringbuilder *sb, char* buff, size_t size);

#endif
