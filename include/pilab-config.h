#ifndef _PILAB_CONFIG_H
#define _PILAB_CONFIG_H
#include <stdio.h>
#include "pilab-list.h"

enum t_config_field_types {
	CONFIG_FIELD_CLASSROOM = 0,
	CONFIG_FIELD_EMAIL,
	CONFIG_FIELD_PASS,
	CONFIG_FIELD_ADDRESS,
	CONFIG_FIELD_PORT,
	CONFIG_FIELD_MAC,
	/*
	 * Number of fields.
	 */
	CONFIG_FIELD_NUM_TYPES,
};

struct t_pilab_config {
	/*
	 * E-mail, login-credentials
	 */
	char *email;
	/*
	 * Password, yes i know stored as plain text...
	 */
	char *password;
	/*
	 * File descriptor of the configuration file.
	 */
	FILE *config_file;
	/*
	 * Config file path
	 */
	const char *config_file_path;
	/*
	 * Port of the host you want to connect to.
	 *
	 * Keep this 0, if no port is specified.
	 */
	char *port;
	/*
	 * Address of the host you want to connect to.
	 */
	char *address;
	/*
	 * Mac address of the device sending the request.
	 * Used for registration purposes.
	 */
	char *mac;
	/*
	 * Classroom of where the device is setup.
	 * Used for registration purposes.
	 */
	char *classroom;
	/*
	 * Full url of the host
	 *
	 * NOTE: This will be set automatically.
	 */
	char *base_url;
};

/* Keywords config */
#define PILAB_CONFIG_FIELD_CLASSROOM "classroom"
#define PILAB_CONFIG_FIELD_EMAIL "email"
#define PILAB_CONFIG_FIELD_PASS "password"
#define PILAB_CONFIG_FIELD_ADDRESS "address"
#define PILAB_CONFIG_FIELD_PORT "port"
#define PILAB_CONFIG_FIELD_MAC "mac"

extern int config_get_field_type(const char *type);
extern struct t_pilab_config *config_create_custom(const char *path);
extern struct t_pilab_config *config_create(int index);
extern void config_append_to_configuration(struct t_pilab_config *config,
					   const char *string, int keep_open);
extern int config_assign_value(struct t_pilab_config *config,
			       struct t_pilist *key_value);
extern char *config_create_base_url(struct t_pilab_config *config);
extern int config_has_necessary_configurations(struct t_pilab_config *config);
extern int config_try_open(struct t_pilab_config *config, const char *mode);
extern void config_clear_file(struct t_pilab_config *config);
extern int config_read_configuration_file(struct t_pilab_config *config);
extern int config_try_close(struct t_pilab_config *config);
extern void config_free(struct t_pilab_config *config);

#endif
