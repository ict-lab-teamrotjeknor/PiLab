#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pilab-string.h"
#include "pilab-list.h"
#include "pilab-log.h"

/*
 * Converts string to upper case.
 */

void string_to_upper(char *string)
{
	while (*string) {
		if (*string >= 'a' && *string <= 'z')
			*string &= ~(1UL << 5);
		string++;
	}
}

/*
 * Converts string to lower case.
 */

void string_to_lower(char *string)
{
	while (*string) {
		if (*string >= 'A' && *string <= 'Z')
			*string |= 1UL << 5;
		string++;
	}
}

/*
 * Compares two ASCII chars (case dependent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_charcmp(const char *string1, const char *string2)
{
	size_t wchar1, wchar2;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);
	wchar1 = *string1;
	wchar2 = *string2;
	return (wchar1 < wchar2) ? -1 : ((wchar1 == wchar2) ? 0 : 1);
}

/*
 * Compares two ASCII chars (case independent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_charcasecmp(const char *string1, const char *string2)
{
	size_t wchar1, wchar2;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);

	wchar1 = *string1;
	if ((wchar1 >= 'A') && (wchar1 <= 'Z'))
		wchar1 += ('a' - 'A');

	wchar2 = *string2;
	if ((wchar2 >= 'A') && (wchar2 <= 'Z'))
		wchar2 += ('a' - 'A');

	return (wchar1 < wchar2) ? -1 : ((wchar1 == wchar2) ? 0 : 1);
}

/*
 * Compares two strings (case dependent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_strcmp(const char *string1, const char *string2)
{
	size_t diff, slen1, slen2;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);

	slen1 = strlen(string1);
	slen2 = strlen(string2);

	if (slen1 != slen2)
		return (slen1 < slen2) ? -1 : 1;

	while (*string1 && *string2) {
		diff = string_charcmp(string1, string2);
		if (diff != 0)
			return diff;
		string1++;
		string2++;
	}
	return 0;
}

/*
 * Compares two strings with the first n bytes (case dependent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_strncmp(const char *string1, const char *string2, size_t n)
{
	size_t diff, slen1, slen2, count;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);

	count = 0;
	while (count < n && *string1 && *string2) {
		diff = string_charcmp(string1, string2);
		if (diff != 0)
			return diff;
		string1++;
		string2++;
		count++;
	}

	slen1 = strlen(string1);
	slen2 = strlen(string2);

	if (slen1 != slen2)
		return (slen1 < slen2) ? -1 : 1;

	if (count >= n)
		return 0;

	return slen1 < slen2 ? -1 : ((slen1 > slen2) ? 1 : 0);
}

/*
 * Compares two strings (case independent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_strcasecmp(const char *string1, const char *string2)
{
	size_t diff, slen1, slen2;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);

	slen1 = strlen(string1);
	slen2 = strlen(string2);

	if (slen1 != slen2)
		return (slen1 < slen2) ? -1 : 1;

	while (*string1 && *string2) {
		diff = string_charcasecmp(string1, string2);
		if (diff != 0)
			return diff;
		string1++;
		string2++;
	}
	return 0;
}

/*
 * Compares two strings with the first n bytes (case independent).
 *
 * Returns:
 * -1: string1 < string2
 *  0: string1 == string2
 *  1: string1 > string2
 */

int string_strcasencmp(const char *string1, const char *string2, size_t n)
{
	size_t diff, slen1, slen2, count;

	if (!string1 || !string2)
		return (string1) ? 1 : ((string2) ? -1 : 0);

	count = 0;
	while (count < n && *string1 && *string2) {
		diff = string_charcasecmp(string1, string2);
		if (diff != 0)
			return diff;
		string1++;
		string2++;
		count++;
	}

	slen1 = strlen(string1);
	slen2 = strlen(string2);

	if (slen1 != slen2)
		return (slen1 < slen2) ? -1 : 1;

	if (count >= n)
		return 0;

	return slen1 < slen2 ? -1 : ((slen1 > slen2) ? 1 : 0);
}

/*
 * Duplicate a string.
 *
 * Returns a pointer the new string, or NULL otherwise.
 */

char *string_strdup(const char *string)
{
	char *new_str;
	size_t slen;

	if (!string)
		return NULL;

	slen = strlen(string);
	/* We need room for 1 more char, available for adding the '\0' */
	new_str = malloc(sizeof(char) * slen + 1);

	if (!new_str) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate enough memory for string_strdup");
		return NULL;
	}

	strncpy(new_str, string, slen + 1);

	return new_str;
}

/*
 * String concatenation with a delimiter.
 *
 * NOTE: The returned pointer needs to be freed afterwards.
 *
 * Returns a pointer to the concatenated string, NULL otherwise.
 */

char *string_strcat_delimiter(const char *string1, const char *string2,
			      const char *delimiter)
{
	size_t slen1, slen2, slen3, i;
	char *new_str;

	if (!string1 || !string2 || !delimiter)
		return NULL;

	slen1 = strlen(string1);
	slen2 = strlen(string2);
	slen3 = strlen(delimiter);

	new_str = malloc(sizeof(*new_str) * (slen1 + slen3 + slen2) + 1);

	if (!new_str)
		return NULL;

	for (i = 0; i < slen1; i++, string1++)
		new_str[i] = *string1;

	for (; i < (slen1 + slen3); i++, delimiter++)
		new_str[i] = *delimiter;

	for (; i < (slen1 + slen3 + slen2); i++, string2++)
		new_str[i] = *string2;

	new_str[slen1 + slen3 + slen2] = '\0';

	return new_str;
}

/*
 * String concatenation with a delimiter, recursively.
 *
 * NOTE: The pointer returned needs to be freed afterwards.
 *
 * Returns a pointer to the concatenated string, NULL otherwise.
 */

char *string_strcat_delimiter_recursive(char *string1, const char *delimiter,
					int depth, ...)
{
	va_list ap;
	int i;
	char *new_string;
	char *old_string;

	if (!string1 || !delimiter)
		return NULL;

	va_start(ap, depth);
	for (i = 0; i < depth; i++) {
		if (i == 0) {
			if (string_strcmp(string1, "") == 0)
				new_string = string_strcat(string1,
							   va_arg(ap, char *));
			else
				new_string = string_strcat_delimiter(
					string1, va_arg(ap, char *), delimiter);
		} else {
			old_string = new_string;
			new_string = string_strcat_delimiter(
				new_string, va_arg(ap, char *), delimiter);
			free(old_string);
		}
	}
	va_end(ap);
	return new_string;
}

/*
 * Concatenate two strings, effectively appending string2 to string1.
 *
 * NOTE: The pointer returned needs to be freed afterwards.
 *
 * Returns a pointer the new string, or NULL otherwise.
 */

char *string_strcat(const char *string1, const char *string2)
{
	return string_strcat_delimiter(string1, string2, "");
}

/*
 * Concatenate two string, with n bytes specifying the number of bytes that
 * will be appended to string1.
 *
 * NOTE: If n > strlen(string2), n will be shrunken to the length of string2.
 *
 * NOTE: The returned pointer needs to be freed afterwards.
 *
 * Returns a pointer the new string, or NULL otherwise.
 */

char *string_strncat(const char *string1, const char *string2, size_t n)
{
	size_t slen1, slen2;
	char *new_str;
	size_t i;

	if (!string1 || !string2)
		return NULL;

	slen1 = strlen(string1);
	slen2 = strlen(string2);

	new_str = malloc(sizeof(*new_str) * (slen1 + slen2) + 1);

	if (!new_str)
		return NULL;

	for (i = 0; i < slen1; i++, string1++)
		new_str[i] = *string1;
	/*
	 * Shrink n, effectively making it the same length as slen1.
	 */
	if (n > slen2)
		n -= n % slen2;
	for (; i < (slen1 + n); i++, string2++)
		new_str[i] = *string2;
	new_str[slen1 + slen2 + 1] = '\0';

	return new_str;
}

/*
 * Split a string on delimiters.
 *
 * Returns a new pilist with the split parts, NULL otherwise.
 */

struct t_pilist *string_split(const char *string, const char *delimiter)
{
	struct t_pilist *result;
	char *copy;
	char *token;
	char *tok_res;

	result = pilist_create();
	if (!result)
		return NULL;

	copy = string_strdup(string);
	if (!copy) {
		free(result);
		return NULL;
	}

	tok_res = copy;
	while ((token = strtok_r(tok_res, delimiter, &tok_res))) {
		if (!token) {
			free(token);
			free(copy);
			free(result);
			return NULL;
		}
		pilist_add_last(result, token);
	}

	free(copy);

	return result;
}

/*
 * Split the string on the first occurrence of delimiter and return the string
 * BEFORE the delimiter.
 *
 * NOTE: The pointer in this function needs to be cleaned afterwards.
 *
 * Return the first part of the string split, NULL otherwise.
 */

char *string_split_first(char *string, const char *delimiter)
{
	char *new_str;

	new_str = string_read_until(string, delimiter);

	return new_str;
}

/*
 * Split the string on the last occurrence of delimiter and return the string
 * AFTER the delimiter.
 *
 * NOTE: The pointer in this function needs to be cleaned afterwards.
 *
 * Return the last part of the string split, NULL otherwise.
 */

char *string_split_last(char *string, const char *delimiter)
{
	int index;
	char *new_str;
	size_t dlen;

	if (!string || !delimiter)
		return NULL;

	index = string_find_first(string, delimiter);
	if (index < 0)
		return NULL;

	dlen = strlen(delimiter);

	new_str = string_strdup(string + index + dlen);
	if (!new_str) {
		pilab_log(
			LOG_DEBUG,
			"Could not allocate memory for string split in string_split_last");
		return NULL;
	}
	free(string);

	return string_strip_whitespace(new_str);
}

/*
 * Find the first occurrence of string2 inside a string1.
 *
 * Returns index of the first occurrence, otherwise -1.
 */

int string_find_first(const char *string1, const char *string2)
{
	int i, found, index;

	if (!string1 || !string2)
		return -1;

	index = 0;
	found = 0;
	while (string1[index] != '\0') {
		if (string1[index] == string2[0]) {
			i = 0;
			found = 1;
			while (string2[i] != '\0') {
				if (string1[index + i] != string2[i]) {
					found = 0;
					break;
				}
				i++;
			}
		}

		if (found)
			break;

		index++;
	}

	return (found) ? index : -1;
}

/*
 * Replace first occurrence of string2 in string1 with string3.
 *
 * NOTE: This function returns a new string, which must be freed afterwards.
 *
 * Returns pointer to the new string, or NULL otherwise.
 */

char *string_replace_first(const char *string1, const char *string2,
			   const char *string3)
{
	int index, slen1, slen3, i;
	char *copy;

	if (!string1 || !string2 || !string3)
		return NULL;

	if (string_strcmp(string3, ""))
		return (copy = string_strdup(string1));

	index = string_find_first(string1, string2);

	slen1 = strlen(string1);
	slen3 = strlen(string3);

	if (index > -1) {
		copy = string_strdup(string1);
		copy = realloc(copy, sizeof(char) * (slen1 + slen3));
		memmove(&copy[index + slen3 - 1], &copy[index], slen1 - index);

		for (i = 0; i < slen3; ++i)
			copy[index + i] = string3[i];

		return copy;
	}

	return NULL;
}

/*
 * Strip all the leading and trailing whitespace.
 *
 * NOTE: The string will be created with a call to string_strdup, so it needs to
 * be freed afterwards.
 *
 * Returns pointer to the new string, NULL otherwise.
 */

char *string_strip_whitespace(char *string)
{
	char *new_str;
	char *old_str;
	int i;

	if (!string || *string == '\0')
		return string;

	old_str = string;

	while (*string == ' ' || *string == '\t')
		string++;

	new_str = string_strdup(string);
	free(old_str);

	for (i = 0; new_str[i] != '\0'; ++i)
		;

	do {
		i--;
	} while (i >= 0 && (new_str[i] == ' ' || new_str[i] == '\t'));

	new_str[i + 1] = '\0';

	return new_str;
}

/*
 * Read a string until first encountering the occurrence of string2.
 *
 * NOTE: The returned pointer needs to be cleaned afterwards.
 *
 * Returns NULL, if it couldn't find the occurrence, otherwise a pointer to a
 * new allocated string.
 */

char *string_read_until(const char *string1, const char *string2)
{
	int index;
	char *new_str;

	if (!string1 || !string2)
		return NULL;

	index = string_find_first(string1, string2);
	if (index < 0)
		return NULL;

	new_str = calloc(index + 1, sizeof(char));
	if (!new_str) {
		pilab_log(
			LOG_DEBUG,
			"Could not allocate memory for read until in string_read_until");
		return NULL;
	}

	strncpy(new_str, string1, index);

	return new_str;
}
