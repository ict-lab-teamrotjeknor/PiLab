#include <string.h>
#include <stdlib.h>
#include "pilab-stringbuilder.h"
#include "pilab-string.h"

/*
 * Conjure up a new stringbuilder.
 *
 * Returns pointer to new stringbuilder object, NULL if error.
 */

struct t_stringbuilder *stringbuilder_create_size(size_t alloc_size)
{
	size_t size_alloc;
	struct t_stringbuilder *sb;

	size_alloc = alloc_size;
	sb = malloc(sizeof(*sb));

	if (!sb) {
		/* TODO: Failed to allocate memory for char buffer in stringbuilder_create_size */
		return NULL;
	}

	sb->string = calloc(size_alloc, sizeof(char));

	if (!sb->string) {
		/* TODO: Failed to allocate memory for char buffer in stringbuilder_create_size */
		free(sb);
		return NULL;
	}

	sb->length = 0;
	sb->size_alloc = size_alloc;

	return sb;
}

/*
 * Conjure up a new stringbuilder, with an initial capacity of 128.
 *
 * Returns a pointer to the newly allocated stringbuilder, NULL otherwise.
 */

struct t_stringbuilder *stringbuilder_create()
{
	return stringbuilder_create_size(128);
}

/*
 * Free the string builder and the internally allocated string.
 */

void stringbuilder_free(struct t_stringbuilder *sb)
{
	if (!sb)
		return;

	if (sb->string)
		free(sb->string);
	free(sb);
}

/*
 * Make sure that the stringbuilder contains enough allocated space.
 *
 * Grow the stringbuilders if the sum of the provided capacity and the current
 * string length is bigger than the allocated size.
 *
 * Returns pointer to the newly allocated memory, or NULL otherwise.
 */

char *stringbuilder_resize(struct t_stringbuilder *sb, size_t capacity)
{
	char *new_str;

	if (!sb)
		return NULL;

	if ((sb->length + capacity) >= sb->size_alloc) {
		new_str = realloc(sb->string, sizeof(char) * capacity);
		if (!new_str) {
			/* TODO: Insert error in log: Failed to allocate memory for stringbuilder resize */
			return NULL;
		}

		sb->size_alloc *= GROW_FACTOR;
		sb->string = new_str;
	}

	return sb->string;
}

/*
 * Add n-bytes the end of the internal string buffer.
 */

void stringbuilder_append_nbytes(struct t_stringbuilder *sb, const char *str, size_t n)
{

	if (!str || !sb || *str == '\0')
		return;

	if (!stringbuilder_resize(sb, n))
		return;

	memcpy(sb->string + sb->length , str, n);
	sb->length += n;
	sb->string[sb->length + 1] = '\0';
}

/*
 * Add the provided string to the end of the internal string buffer.
 */

void stringbuilder_append(struct t_stringbuilder *sb, const char *str)
{
	size_t str_len;

	if (!str)
		return;

	str_len = strlen(str);

	stringbuilder_append_nbytes(sb, str, str_len);
}

/*
 * Add the given string to the front of the internal string buffer.
 */

void stringbuilder_prepend(struct t_stringbuilder *sb, const char *str)
{
	size_t str_len, sb_len;

	if (!str || !sb || *str == '\0')
		return;

	if (!stringbuilder_resize(sb, strlen(str)))
		return;

	str_len = strlen(str);

	sb_len = sb->length;

	sb->length += str_len;
	memmove(&sb->string[str_len], sb->string, sb_len);
	for (size_t i = 0; i < str_len; ++i)
		sb->string[i] = str[i];
	sb->string[sb->length + 1] = '\0';
}

/*
 * Get a string and cleanup stringbuilder.
 *
 * Returns pointer to the string, or NULL if the string could not be retrieved.
 */

char *stringbuilder_to_string(struct t_stringbuilder *sb)
{
	char *str_value;

	if (!sb)
		return NULL;

	str_value = string_strdup(sb->string);

	stringbuilder_free(sb);
	return str_value;
}
