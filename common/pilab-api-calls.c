#include "pilab-log.h"
#include "pilab-api-client.h"
#include "pilab-json-parser.h"
#include "pilab-string.h"

void pilab_login(struct t_api_client *client)
{
	struct t_api_client_request *request;
	struct t_api_client_cookie *cookie;
	json_object *json;
	json_object *response_body;
	char *cookie_header;

	/* create new post fields for request */
	json = json_object_new_object();
	/* build post data */
	json_object_object_add(json, "email",
			       json_object_new_string(client->config->email));
	json_object_object_add(
		json, "password",
		json_object_new_string(client->config->password));

	/* create a new json request */
	request = api_client_request_post_json(
		client, json, "authentication/signin", "signin");

	/* execute the request */
	api_client_request_execute(request);

	/* get set-cookie */
	cookie_header = api_client_request_get_header(request, "Set-Cookie");

	/* create a new cookie */
	cookie = api_client_cookie_create(cookie_header);

	/* set the cookie */
	api_client_set_cookie(client, cookie);

	response_body = api_client_request_get_response_body_json(request);

	/* handle response */
	if (string_strcmp(json_parser_object_to_string(json_parser_find_object(
				  response_body, "Succeed")),
			  "true") == 0) {
		pilab_log(LOG_INFO, "Successful login for %s",
			  client->config->email);
	} else {
		pilab_log(LOG_INFO, "Could not login: %s",
			  json_parser_object_to_string(json_parser_find_object(
				  response_body, "Error")));
	}

	/* close the request */
	api_client_close_request(client, request);
}

void pilab_add_pi(struct t_api_client *client)
{
	struct t_api_client_request *request;
	json_object *json;
	json_object *response_body;

	if (!api_client_is_valid_cookie(client->cookie))
		pilab_login(client);

	/* create new post fields for request */
	json = json_object_new_object();
	/* build post data */
	json_object_object_add(
		json, "MacAdress",
		json_object_new_string(client->config->classroom));
	/* Mac address is the same as the classroom, this is intentional */
	json_object_object_add(
		json, "ClassroomName",
		json_object_new_string(client->config->classroom));

	/* create a new json request */
	request = api_client_request_post_json(client, json, "manage/add/pi",
					       "addpi");

	/* add cookie header */
	api_client_request_add_header(
		request, string_strcat("Cookie: ",
				       api_client_get_cookie_content(client)));

	api_client_request_execute(request);

	response_body = api_client_request_get_response_body_json(request);

	/* handle response */
	if (string_strcmp(json_parser_object_to_string(json_parser_find_object(
				  response_body, "Succeed")),
			  "true") == 0) {
		pilab_log(LOG_INFO,
			  "Successfully added Raspberry Pi to room: %s",
			  client->config->classroom);
	} else {
		pilab_log(LOG_INFO, "Could not add pi: %s",
			  json_parser_object_to_string(json_parser_find_object(
				  response_body, "Error")));
	}

	api_client_close_request(client, request);
}

void pilab_add_sensor(struct t_api_client *client, const char *name,
		      const char *type_value)
{
	struct t_api_client_request *request;
	json_object *json;
	json_object *response_body;

	if (!api_client_is_valid_cookie(client->cookie))
		pilab_login(client);

	/* create new post fields for request */
	json = json_object_new_object();
	/* build post data */
	json_object_object_add(json, "Name", json_object_new_string(name));
	json_object_object_add(json, "Type",
			       json_object_new_string(type_value));
	/* Mac address is the same as the classroom, this is intentional */
	json_object_object_add(
		json, "Room",
		json_object_new_string(client->config->classroom));

	/* create a new json request */
	request = api_client_request_post_json(client, json,
					       "sensor/addnewsensor", name);

	/* add cookie header */
	api_client_request_add_header(
		request, string_strcat("Cookie: ",
				       api_client_get_cookie_content(client)));

	api_client_request_execute(request);

	response_body = api_client_request_get_response_body_json(request);

	/* handle response */
	if (string_strcmp(json_parser_object_to_string(json_parser_find_object(
				  response_body, "Succeed")),
			  "true") == 0) {
		pilab_log(LOG_INFO, "Successfully added Sensor to room: %s",
			  client->config->classroom);
	} else {
		pilab_log(LOG_INFO, "Could not add sensor: %s",
			  json_parser_object_to_string(json_parser_find_object(
				  response_body, "Error")));
	}

	api_client_close_request(client, request);
}

void pilab_add_data(struct t_api_client *client, const char *value)
{
	struct t_api_client_request *request;
	json_object *json;
	json_object *response_body;

	if (!api_client_is_valid_cookie(client->cookie))
		pilab_login(client);

	/* create new post fields for request */
	json = json_object_new_object();
	/* build post data */
	json_object_object_add(json, "Name",
			       json_object_new_string("Temperature"));
	json_object_object_add(json, "value", json_object_new_string(value));
	/* Mac address is the same as the classroom, this is intentional */
	json_object_object_add(
		json, "Room",
		json_object_new_string(client->config->classroom));

	/* create a new json request */
	request = api_client_request_post_json(client, json, "sensor/adddata",
					       "");

	/* add cookie header */
	api_client_request_add_header(
		request, string_strcat("Cookie: ",
				       api_client_get_cookie_content(client)));

	api_client_request_execute(request);

	response_body = api_client_request_get_response_body_json(request);

	/* handle response */
	if (string_strcmp(json_parser_object_to_string(json_parser_find_object(
				  response_body, "Succeed")),
			  "true") == 0) {
		pilab_log(LOG_DEBUG, "Added reading for room: %s",
			  client->config->classroom);
	} else {
		pilab_log(LOG_ERROR, "Could not add data: %s",
			  json_parser_object_to_string(json_parser_find_object(
				  response_body, "Error")));
	}

	api_client_close_request(client, request);
}
