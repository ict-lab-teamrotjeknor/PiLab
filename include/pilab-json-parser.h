#ifndef _PILAB_JSON_PARSER_H
#define _PILAB_JSON_PARSER_H
#include <json-c/json.h>

extern struct json_object *json_parser_find_object(struct json_object *jobj,
						   const char *key);
extern struct json_object *json_parser_to_json(const char *string);
extern const char *json_parser_object_to_string(struct json_object *jobj);

#endif
