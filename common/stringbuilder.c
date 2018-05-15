#include "stringbuilder.h"
#include <stdio.h>
#include <string.h>

struct stringbuilder* string_builder_create()
{
	size_t length = 0, capacity = 128;
	struct stringbuilder* sb = (struct stringbuilder*)
		malloc(sizeof(struct stringbuilder));

	if (!sb)
		return NULL;
	sb->string = (char*)malloc(sizeof(char) * capacity);
	if (!sb->string)
		return NULL;
	sb->length = length;
	sb->capacity = capacity;
	return sb;
}

int string_builder_free(struct stringbuilder* sb)
{
	if (!sb)
		return SB_ERR;
	free(sb->string);
	free(sb);
	return SB_OK;
}

static int string_builder_resize(struct stringbuilder* sb)
{
	if (!sb)
		return SB_ERR;
	if (sb->length == sb->capacity) {
		sb->capacity *= GROW_FACTOR;
		sb->string = (char*)realloc(sb->string, sizeof(char) * sb->capacity);
		if (!sb->string)
			return SB_ERR;
	}
	return SB_OK;
}

char* string_builder_append(struct stringbuilder* sb, const char* str)
{
	if (!str || string_builder_resize(sb) != SB_OK)
		return NULL;
	size_t str_len = strlen(str);
	size_t sb_len = sb->length;
	sb->length += str_len;
	return memcpy(sb->string + sb_len, str, str_len);
}

char* string_builder_to_string(struct stringbuilder *sb, char* buff, size_t size)
{
	if (!sb || !buff)
		return NULL;
	if (size < sb->length + 1)
		return NULL;
	char* dest = strncpy(buff, sb->string, sb->length);
	buff[sb->length+1] = '\0';
	string_builder_free(sb);
	return dest;
}
