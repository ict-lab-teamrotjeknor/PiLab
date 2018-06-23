#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include "pilab-log.h"
#include "pilab-readline.h"

char *read_line(FILE *file)
{
	size_t length, size;
	char *string;
	char last_char;

	length = 0;
	size = 128;
	string = malloc(size);
	last_char = '\0';

	if (!string) {
		pilab_log(LOG_ERROR,
			  "Unable to allocate memory for read_line.");
		return NULL;
	}

	while (1) {
		int c;

		c = getc(file);
		if (c == '\n' && last_char == '\\') {
			/* ignore the last character. */
			--length;
			last_char = '\0';
			continue;
		}

		if (c == EOF || c == '\n' || c == '\0')
			break;
		if (c == '\r')
			continue;

		last_char = c;
		if (length == size) {
			char *new_string;
			new_string = realloc(string, size *= 2);
			if (!new_string) {
				free(string);
				pilab_log(
					LOG_ERROR,
					"Unable to allocate memory for readline.");
				return NULL;
			}
			string = new_string;
		}
		string[length++] = c;
	}
	if (length + 1 == size) {
		char *new_string;

		new_string = realloc(string, length + 1);

		if (!new_string) {
			free(string);
			return NULL;
		}

		string = new_string;
	}

	string[length] = '\0';

	return string;
}

char *peek_line(FILE *file, int line_offset, long *position)
{
	size_t length;
	ssize_t read;
	char *line;
	long pos;
	int i;

	pos = ftell(file);
	length = 0;
	line = NULL;

	for (i = 0; i <= line_offset; i++) {
		read = getline(&line, &length, file);
		if (read < 0) {
			free(line);
			line = NULL;
			break;
		}

		if (read > 0 && line[read - 1] == '\n')
			line[read - 1] = '\0';
	}

	if (position)
		*position = ftell(file);

	fseek(file, pos, SEEK_SET);

	return line;
}

char *read_line_buffer(FILE *file, char *string, size_t string_len)
{
	size_t length;
	int c;

	length = 0;

	if (!string)
		return NULL;

	while (1) {
		c = getc(file);

		if (c == EOF || c == '\n' || c == '\0')
			break;

		if (c == '\r')
			continue;

		string[length++] = c;

		if (string_len <= length)
			return NULL;
	}

	if (length + 1 == string_len)
		return NULL;

	string[length] = '\0';

	return string;
}
