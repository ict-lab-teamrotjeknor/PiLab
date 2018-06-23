#include <stdlib.h>
#include <string.h>
#include "pilab-config.h"
#include "pilab-readline.h"
#include "pilab-log.h"
#include "pilab-string.h"

static const char *configuration_paths[] = {
	SYSCONFDIR "/pilab/config",
};

char *config_field_type_string[CONFIG_FIELD_NUM_TYPES] = {
	PILAB_CONFIG_FIELD_CLASSROOM, PILAB_CONFIG_FIELD_EMAIL,
	PILAB_CONFIG_FIELD_PASS,      PILAB_CONFIG_FIELD_ADDRESS,
	PILAB_CONFIG_FIELD_PORT,      PILAB_CONFIG_FIELD_MAC,
};

/*
 * Search for a config field type.
 *
 * Return index of type, -1 if the type could not be found.
 */

int config_get_field_type(const char *type)
{
	if (!type)
		return -1;

	for (int i = 0; i < CONFIG_FIELD_NUM_TYPES; ++i)
		if (string_strcmp(config_field_type_string[i], type) == 0)
			return i;

	/* type was not found */
	return -1;
}

/*
 * Conjure up a new configuration instance, and let's you set the path to the
 * configuration file.
 *
 * Returns a pointer to a newly create config, otherwise NULL.
 */

struct t_pilab_config *config_create_custom(const char *path)
{
	struct t_pilab_config *new_config;

	new_config = malloc(sizeof(*new_config));

	if (!new_config)
		return NULL;

	new_config->config_file_path = path;
	new_config->base_url = NULL;
	new_config->config_file = NULL;
	new_config->classroom = NULL;
	new_config->email = NULL;
	new_config->password = NULL;
	new_config->address = NULL;
	new_config->port = NULL;

	return new_config;
}

/*
 * Conjure up a new configuration instance, pass 0 for default config.
 *
 * Returns a pointer to a newly create config, otherwise NULL.
 */

struct t_pilab_config *config_create(int index)
{
	return config_create_custom(configuration_paths[index]);
}

/*
 * Try to open config file.
 *
 * Returns:
 *  -1: invalid argument.
 *  0: file error.
 *  1: if file found.
 */

int config_try_open(struct t_pilab_config *config, const char *mode)
{
	FILE *file;
	const char *file_path;

	if (!config || !mode)
		return -1;

	if (config->config_file && config->config_file_path) {
		return 1;
	} else if (config->config_file_path && !config->config_file) {
		file = fopen(config->config_file_path, mode);
		config->config_file = file;
		return 1;
	} else {
		file_path = configuration_paths[0];
		file = fopen(file_path, mode);
		if (file) {
			config->config_file = file;
			config->config_file_path = file_path;
			return 1;
		}
	}
	return 0;
}

/*
 * Check if the configuration has all the necessary configurations.
 *
 * Returns:
 * -1: if invalid config parameter.
 *  0: if not all necessary fields.
 *  1: all necessary fields.
 */

int config_has_necessary_configurations(struct t_pilab_config *config)
{
	int all;

	if (!config)
		return -1;

	all = 1;

	/*  necessary fields */

	if (!config->classroom)
		all = 0;
	if (!config->email)
		all = 0;
	if (!config->password)
		all = 0;
	if (!config->address)
		all = 0;
	if (!config->port)
		all = 0;

	return all;
}

/*
 * Clear the configuration file.
 */

void config_clear_file(struct t_pilab_config *config)
{
	FILE *file;

	if (!config)
		return;

	if (!config->config_file_path)
		return;

	/* open, clear, close */
	file = fopen(config->config_file_path, "w");
	if (file)
		fclose(file);
}

/*
 * Append to the configuration file.
 *
 * The third argument servers as a switch, so you can leave the config file open.
 */

void config_append_to_configuration(struct t_pilab_config *config,
				    const char *string, int keep_open)
{
	if (!config)
		return;

	if (config_try_open(config, "a")) {
		fprintf(config->config_file, "%s\n", string);
	}

	if (!keep_open)
		config_try_close(config);
}

/*
 * Validate a key value config line to the configuration.
 *
 * Returns:
 * -1: invalid argument
 *  0: on error.
 *  1: on success;
 */

int config_assign_value(struct t_pilab_config *config,
			struct t_pilist *key_value)
{
	int valid;
	char *keyword, *value;

	if (!config)
		return -1;

	keyword = (char *)pilist_get_data(key_value, 0);
	value = (char *)pilist_get_data(key_value, 1);
	valid = config_get_field_type(keyword);

	if (valid > -1) {
		switch (valid) {
		case CONFIG_FIELD_PORT:
			if (config->port)
				free(config->port);
			config->port = value;
			break;
		case CONFIG_FIELD_ADDRESS:
			if (config->address)
				free(config->address);
			config->address = value;
			break;
		case CONFIG_FIELD_MAC:
			if (config->mac)
				free(config->mac);
			config->mac = value;
			break;
		case CONFIG_FIELD_CLASSROOM:
			if (config->classroom)
				free(config->classroom);
			config->classroom = value;
			break;
		case CONFIG_FIELD_EMAIL:
			if (config->email)
				free(config->email);
			config->email = value;
			break;
		case CONFIG_FIELD_PASS:
			if (config->password)
				free(config->password);
			config->password = value;
			break;
		case CONFIG_FIELD_NUM_TYPES:;
		}
	}

	return 1;
}

/*
 * Create a base url from the address and the port.
 *
 * NOTE: Memory needs to be freed after using.
 *
 * Returns pointer to the base url, NULL otherwise.
 */

char *config_create_base_url(struct t_pilab_config *config)
{
	char *base_url;
	if (!config)
		return NULL;

	/* we have everything for creating a base url */
	if (!config->address && !config->port)
		return NULL;
	if (config->base_url)
		free(config->base_url);

	base_url = string_strcat_delimiter(config->address, config->port, ":");

	return base_url;
}

/*
 * Read a configuration file.
 *
 * NOTE: This call automatically closes the file descriptor.
 *
 * Returns:
 * -1: invalid argument
 *  0: on error.
 *  1: on success;
 */

int config_read_configuration_file(struct t_pilab_config *config)
{
	if (!config)
		return -1;

	char *line;
	FILE *file;
	int line_no;
	struct t_pilist *split_config_line;

	line_no = 0;
	if (config_try_open(config, "r")) {
		file = config->config_file;
		while (!feof(file)) {
			line = read_line(file);

			if (!line)
				continue;

			line_no++;
			pilab_log(LOG_DEBUG, "Read line %d: %s", line_no, line);
			line = string_strip_whitespace(line);

			/* this is a comment, skip it */
			if (*line == '#') {
				free(line);
				continue;
			}

			/* empty is just empty */
			if (strlen(line) == 0) {
				free(line);
				continue;
			}

			split_config_line = NULL;
			/* we split on whitespace */
			split_config_line = string_split(line, " ");
			if (split_config_line) {
				config_assign_value(config, split_config_line);
				free(split_config_line);
				split_config_line = NULL;
			}
			free(line);
			line = NULL;
		}
	} else {
		pilab_log(
			LOG_ERROR, "%s",
			"config_read_configuration_file could not open file!");
		return 0;
	}

	config->base_url = config_create_base_url(config);

	/* whats open needs to be closed */
	config_try_close(config);

	return 1;
}

/*
 * Try to close config file.
 *
 * Returns:
 * -1: config could not be found.
 *  0: file could not be closed.
 *  1: file was closed.
 */

int config_try_close(struct t_pilab_config *config)
{
	int rc;

	if (!config)
		return -1;

	rc = -1;
	if (config->config_file) {
		fclose(config->config_file);
		config->config_file = NULL;
		rc = 1;
	}

	return rc;
}

/*
 * Free memory.
 */

void config_free(struct t_pilab_config *config)
{
	if (!config)
		return;

	if (config->config_file)
		fclose(config->config_file);
	if (config->classroom)
		free(config->classroom);
	if (config->address)
		free(config->address);
	if (config->port)
		free(config->port);
	if (config->mac)
		free(config->mac);
	if (config->base_url)
		free(config->base_url);

	free(config);
}
