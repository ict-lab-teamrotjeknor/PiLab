#include "pilab-json-parser.h"

/*
 * Find a json value by using the key.
 *
 * Returns json object with the value, or NULL.
 */

struct json_object *json_parser_find_object(struct json_object *jobj,
					    const char *key)
{
	struct json_object *tmp;

	if (!jobj || !key)
		return NULL;

	json_object_object_get_ex(jobj, key, &tmp);

	return tmp;
}

/*
 * Returns the string as a json object, NULL otherwise.
 */

struct json_object *json_parser_to_json(const char *string)
{
	struct json_object *tmp;

	if (!string)
		return NULL;

	tmp = json_tokener_parse(string);

	return tmp;
}

/*
 * Returns the string representation of a json object, NULL otherwise.
 */

const char *json_parser_object_to_string(struct json_object *jobj)
{
	if (!jobj)
		return NULL;

	return json_object_to_json_string(jobj);
}
