#include <stdlib.h>
#include <string.h>
#include "pilab-list.h"
#include "pilab-string.h"
#include "pilab-api-client.h"
#include "pilab-log.h"
#include "pilab-time.h"
#include "pilab-json-parser.h"

char *api_client_request_type_string[API_CLIENT_REQUEST_NUM_TYPES] = {
	PILAB_API_CLIENT_REQUEST_GET, PILAB_API_CLIENT_REQUEST_POST
};

static void
	api_client_request_free_request_fields_default_cb(char *request_fields)
{
	if (request_fields)
		free(request_fields);
}

static void api_client_request_free_default_cb(struct t_hashtable *hashtable,
					       const void *key, void *value)
{
	struct t_api_client_request *req_value;
	char *url_key;
	unsigned long long hash;

	url_key = (char *)key;
	req_value = (struct t_api_client_request *)value;

	hash = hashtable->callback_hash_key(hashtable, url_key) %
	       hashtable->size;
	if (hashtable->htable[hash]->value == value)
		api_client_request_free(req_value);
}

/*
 * Search for a request type.
 *
 * Return index of type in enum t_api_client_request_type, -1 if the type could
 * not be found.
 */

int api_client_get_request_type(const char *type)
{
	if (!type)
		return -1;

	for (int i = 0; i < API_CLIENT_REQUEST_NUM_TYPES; ++i)
		if (string_strcasecmp(api_client_request_type_string[i],
				      type) == 0)
			return i;

	/* type not found */
	return -1;
}

/*
 * Create a new api client with the option to set the callbacks on init.
 *
 * Returns a pointer to the newly created api client, NULL otherwise.
 */

struct t_api_client *api_client_create_custom(
	struct t_pilab_config *config,
	t_api_client_write_response_body *callback_write_response_body,
	t_api_client_write_response_headers *callback_write_response_headers)
{
	struct t_api_client *new_client;
	struct t_hashtable *new_hashtable;

	if (!config)
		return NULL;

	new_client = malloc(sizeof(*new_client));
	if (!new_client)
		return NULL;

	new_hashtable = hashtable_create(8, PILAB_HASHTABLE_STRING,
					 PILAB_HASHTABLE_POINTER, NULL, NULL);
	if (!new_hashtable)
		return NULL;

	new_client->config = config;
	new_client->request_table = new_hashtable;
	new_client->cookie = NULL;
	new_hashtable->callback_free_value =
		&api_client_request_free_default_cb;
	new_client->callback_write_response_body =
		(callback_write_response_body) ?
			callback_write_response_body :
			&api_client_write_response_body_default_cb;
	new_client->callback_write_response_headers =
		(callback_write_response_headers) ?
			callback_write_response_headers :
			&api_client_write_response_headers_default_cb;

	return new_client;
}

/*
 * Create a new api client.
 *
 * Returns a pointer to the newly created api client, NULL otherwise.
 */

struct t_api_client *api_client_create(struct t_pilab_config *config)
{
	return api_client_create_custom(config, NULL, NULL);
}

/*
 * Create a new cookie.
 *
 * Returns a pointer to the newly created cookie, NULL otherwise.
 */

struct t_api_client_cookie *api_client_cookie_create(char *content)
{
	struct t_api_client_cookie *new_cookie;
	struct t_stringbuilder *new_cookie_content;
	char *expiration_date;

	if (!content)
		return NULL;

	if (string_find_first(content, "Set-Cookie") > -1)
		content = string_split_last(content, ":");

	expiration_date = string_strdup(content);
	expiration_date = string_split_last(expiration_date, "expires=");
	expiration_date = string_split_last(expiration_date, ";");

	if (!expiration_date) {
		pilab_log(LOG_DEBUG,
			  "Could not find an expiration date on the cookie!");
		free(content);
		return NULL;
	}

	/* validate date */
	if (time_cmpstr(expiration_date, time_get_time()) >= 0) {
		/* expired cookie */
		free(content);
		free(expiration_date);
		return 0;
	}

	new_cookie = malloc(sizeof(*new_cookie));
	if (!new_cookie) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new cookie.");
		return NULL;
	}

	new_cookie_content = stringbuilder_create();
	if (!new_cookie_content) {
		pilab_log(LOG_DEBUG,
			  "Could not allocate memory for new cookie content.");
		return NULL;
	}

	new_cookie->content = new_cookie_content;

	stringbuilder_append(new_cookie->content, content);
	new_cookie->expires = expiration_date;

	return new_cookie;
}

/*
 * Conjure up a new request with the option to set the callbacks on init.
 *
 * Returns a pointer to the newly created request, NULL otherwise.
 */

struct t_api_client_request *api_client_request_create_custom(
	struct t_api_client *client, const char *url, const char *type_request,
	const char *name, char *request_fields,
	t_api_client_request_free_request_fields *callback_free_request_fields)
{
	struct t_api_client_request *new_request;
	struct t_api_client_response *new_response;
	int type_request_int;

	if (!client)
		return NULL;

	new_request = malloc(sizeof(*new_request));
	if (!new_request)
		return NULL;

	type_request_int = api_client_get_request_type(type_request);
	if (type_request_int < 0)
		return NULL;

	new_response = api_client_response_create();
	if (!new_response) {
		free(new_request);
		return NULL;
	}

	new_request->name = name;
	new_request->callback_free_request_fields =
		(callback_free_request_fields) ?
			callback_free_request_fields :
			&api_client_request_free_request_fields_default_cb;
	new_request->response = new_response;
	new_request->headers = NULL;
	new_request->type_request = type_request_int;
	new_request->request_fields = (request_fields) ? request_fields : "";
	new_request->handle = NULL;
	new_request->url =
		string_strcat_delimiter(client->config->base_url, url, "/");

	/* add request to client */
	hashtable_set(client->request_table, name, new_request);

	return new_request;
}

/*
 * Conjure up a new request.
 *
 * Returns a pointer to the newly created request, NULL otherwise.
 */

struct t_api_client_request *
	api_client_request_create(struct t_api_client *client, const char *url,
				  const char *name, const char *type_request,
				  char *request_fields)
{
	return api_client_request_create_custom(client, url, name, type_request,
						request_fields, NULL);
}

/*
 * Conjure up a new response.
 *
 * Returns a pointer to the newly created response, NULL otherwise.
 */

struct t_api_client_response *api_client_response_create()
{
	struct t_api_client_response *new_response;
	struct t_stringbuilder *new_response_body, *new_response_headers;

	new_response = malloc(sizeof(*new_response));
	if (!new_response)
		return NULL;

	new_response_body = stringbuilder_create();
	if (!new_response_body)
		return NULL;

	new_response_headers = stringbuilder_create();
	if (!new_response_headers) {
		free(new_response_body);
		return NULL;
	}

	new_response->response_headers = new_response_headers;
	new_response->response_body = new_response_body;

	return new_response;
}

/*
 * Check if a request is initialised.
 *
 * Returns:
 * -1: invalid request.
 *  0: request is uninitialised.
 *  1: request is initialised.
 */

int api_client_request_is_initialized(struct t_api_client_request *request)
{
	return (request) ? ((request->handle) ? 1 : 0) : -1;
}

/*
 * Add post fields to a request.
 */

void api_client_request_add_request_fields(struct t_api_client_request *request,
					   char *request_fields)
{
	if (!request || !request_fields)
		return;

	request->request_fields = request_fields;
}

/*
 * Add a new request header.
 */

void api_client_request_add_header(struct t_api_client_request *request,
				   const char *header)
{
	request->headers = curl_slist_append(request->headers, header);
}

/*
 * Get a request based on stored name.
 *
 * Returns a pointer to the request for the provided name, or NULL if the URL is
 * not found.
 */

struct t_api_client_request *api_client_get_request(struct t_api_client *client,
						    const char *name)
{
	struct t_api_client_request *request;

	if (!client)
		return NULL;

	request = (struct t_api_client_request *)hashtable_get(
		client->request_table, name);

	return (request) ? request : NULL;
}

/*
 * Initialise the request.
 *
 * NOTE: This function has to be called after api_client_request_create and
 * before any other functions.
 *
 * Returns:
 * -1: invalid parameters.
 *  0: failed to initialise request.
 *  1: initialisation successful.
 */

int api_client_init_request(struct t_api_client *client,
			    struct t_api_client_request *request,
			    int with_headers)
{
	char *request_type;

	if (!client || !request)
		return -1;

	if (api_client_get_request(client, request->name)) {
		if ((request->handle = curl_easy_init()) == NULL) {
			pilab_log(LOG_DEBUG,
				  "Failed to initialise the request");
			return 0;
		}

		/* make curl aware of the url we want to fetch */
		curl_easy_setopt(request->handle, CURLOPT_URL, request->url);

		/* make curl aware of our write callback */
		curl_easy_setopt(request->handle, CURLOPT_WRITEFUNCTION,
				 client->callback_write_response_body);

		/* feed the fetched body to the struct pointer */
		curl_easy_setopt(request->handle, CURLOPT_WRITEDATA,
				 request->response->response_body);

		/* provide a default user agent */
		curl_easy_setopt(request->handle, CURLOPT_USERAGENT,
				 PILAB_API_CLIENT_USER_AGENT);

		/* set timeout */
		curl_easy_setopt(request->handle, CURLOPT_TIMEOUT, 5);

		/* follow redirects */
		curl_easy_setopt(request->handle, CURLOPT_FOLLOWLOCATION, 1);

		/* only one redirect though */
		curl_easy_setopt(request->handle, CURLOPT_MAXREDIRS, 1);

		request_type =
			api_client_request_type_string[request->type_request];

		if (request->type_request == API_CLIENT_REQUEST_POST) {
			curl_easy_setopt(request->handle, CURLOPT_CUSTOMREQUEST,
					 request_type);

			curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER,
					 request->headers);

			curl_easy_setopt(request->handle, CURLOPT_POSTFIELDS,
					 request->request_fields);
		}

		/* if we want access to the response headers */
		if (with_headers) {
			curl_easy_setopt(
				request->handle, CURLOPT_HEADERFUNCTION,
				client->callback_write_response_headers);
			curl_easy_setopt(request->handle, CURLOPT_HEADERDATA,
					 request->response->response_headers);
		}
	}

	return 1;
}

/*
 * Check if the client has a cookie.
 *
 * Returns:
 *  0: no valid cookie.
 *  1: valid cookie.
 */

int api_client_is_valid_cookie(struct t_api_client_cookie *cookie)
{
	if (!cookie)
		return 0;

	if (cookie->content->length > 0 && cookie->expires)
		/* validate date */
		if (time_cmpstr(cookie->expires, time_get_time()) >= 0)
			return 0;

	return 1;
}

/*
 * Get the cookie content from the client.
 *
 * Returns a pointer to the cookies content, NULL otherwise.
 */

char *api_client_get_cookie_content(struct t_api_client *client)
{
	if (!client)
		return NULL;

	return client->cookie->content->string;
}

/*
 * Get the cookie expiration date from the client.
 *
 * Returns a pointer to the cookies expiration date, NULL otherwise.
 */

char *api_client_get_cookie_expire_date(struct t_api_client *client)
{
	if (!client)
		return NULL;

	return client->cookie->expires;
}

/*
 * Set a cookie for the client.
 */

void api_client_set_cookie(struct t_api_client *client,
			   struct t_api_client_cookie *cookie)
{
	if (!client)
		return;

	if (client->cookie)
		api_client_cookie_free(cookie);

	client->cookie = cookie;
}

/*
 * Search for a specific header in the request.
 *
 * NOTE: The pointer returned by this function needs to be cleaned up afterwards.
 *
 * Returns a new char pointer to the header, or NULL if the header could not be
 * found.
 */

char *api_client_request_get_header(struct t_api_client_request *request,
				    const char *header)
{
	char *headers;
	char *result;
	int index;

	headers = api_client_request_get_response_headers(request);
	if (!headers) {
		pilab_log(LOG_DEBUG, "The request, has no headers set.");
		return NULL;
	}

	index = string_find_first(headers, header);
	if (index < 0) {
		pilab_log(LOG_DEBUG, "Header: %s, could not be found.", header);
		return NULL;
	}

	result = string_read_until(headers + index, "\r\n");
	if (!result)
		return NULL;

	return result;
}

/*
 * Execute the request.
 *
 * NOTE: This function is blocking, as it will only proceed when the request has
 * either failed, timed-out, or succeeded.
 *
 * Returns:
 * -1: invalid arguments.
 *  0: something went wrong while performing the execution.
 *  1: successfully executed the request.
 */

int api_client_request_execute(struct t_api_client_request *request)
{
	/* curl result code */
	CURLcode rc;
	int http_code;

	if (!request)
		return -1;

	rc = curl_easy_perform(request->handle);

	if (rc != CURLE_OK || request->response->response_body->length < 1) {
		http_code = api_client_get_http_status_code_request(request);
		pilab_log(
			LOG_DEBUG,
			"Failed to fetch the url, server returned a %d status code:",
			http_code);

		return 0;
	}

	return 1;
}

/*
 * Execute all the requests of the client.
 *
 * NOTE: This function is blocking, as it will only proceed when the request has
 * either failed, timed-out, or succeeded.
 *
 * Returns:
 * -1: invalid arguments.
 *  0: something went wrong while performing the execution.
 *  1: successfully executed the request.
 */

int api_client_execute_all_requests(struct t_api_client *client)
{
	struct t_pilist *key_list;
	struct t_api_client_request *request;
	int i, f;
	char *key;

	if (!client)
		return -1;

	key_list = hashtable_get_key_list(client->request_table);

	f = 0;
	for (i = 0; i < key_list->size; ++i) {
		key = pilist_get_data(key_list, i);
		if ((request = api_client_get_request(client, key)))
			api_client_request_execute(request);
		else
			f++;
	}

	return (f > 0) ? 0 : 1;
}

/*
 * Get the http status code of a request.
 *
 * NOTE: This functions has to be called right after a
 * api_client_execute_request call.
 *
 * Returns:
 *  -1: invalid parameters.
 *   0: request not initialised.
 *  >0: the http status code.
 */

int api_client_get_http_status_code_request(struct t_api_client_request *request)
{
	int http_code;
	CURLcode rc;

	if (!request)
		return -1;

	if (api_client_request_is_initialized(request) < 1)
		return 0;

	http_code = 0;
	rc = curl_easy_getinfo(request->handle, CURLINFO_RESPONSE_CODE,
			       &http_code);

	if (rc == CURLE_OK && http_code > 0)
		return http_code;

	return http_code;
}

/*
 * Retrieve the response body of an api client request.
 *
 * Returns a pointer to the internal response buffer, NULL otherwise.
 */

char *api_client_request_get_response_body(struct t_api_client_request *request)
{
	return (request) ? request->response->response_body->string : NULL;
}

/*
 * Retrieve the response body of an api client request in json.
 *
 * Returns a pointer to the the json response buffer, NULL otherwise.
 */

json_object *api_client_request_get_response_body_json(
	struct t_api_client_request *request)
{
	char *response;
	json_object *json_response;

	if (!request)
		return NULL;

	response = api_client_request_get_response_body(request);

	json_response = json_parser_to_json(response);

	return json_response;
}

/*
 * Retrieve the response headers of an api client request.
 *
 * Returns a pointer to the internal response buffer, NULL otherwise.
 */

char *api_client_request_get_response_headers(
	struct t_api_client_request *request)
{
	return (request) ? request->response->response_headers->string : NULL;
}

/*
 * Write the received data to a provided response header (default callback).
 *
 * This is basically a copy of the reponse body, but for the header.
 *
 * NOTE: When implementing your own callback, you must return the number of
 * bytes actually taken care of, as the library using the heavy lifting
 * (libcurl) will use this number to signal an error condition if this number
 * differs from the amount passed.
 *
 * Return the size written.
 */

size_t api_client_write_response_headers_default_cb(char *contents, size_t size,
						    size_t nmemb,
						    void *response_buffer)
{
	struct t_stringbuilder *sb_ptr;
	size_t realsize;

	/* calculate buffer size */
	realsize = nmemb * size;

	sb_ptr = (struct t_stringbuilder *)response_buffer;

	if (!sb_ptr)
		return -1;

	/*
	 * dunno why we can't pass in realsize for the bytes, but it add random
	 * characters and differs from the length of the chunk (contents).
	 */
	stringbuilder_append_nbytes(sb_ptr, contents, strlen(contents));

	/* return size */
	return realsize;
}

/*
 * Write the received data to a provided response body (default callback).
 *
 * NOTE: When implementing your own callback, you must return the number of
 * bytes actually taken care of, as the library using the heavy lifting
 * (libcurl) will use this number to signal an error condition if this number
 * differs from the amount passed.
 *
 * Return the size written.
 */

size_t api_client_write_response_body_default_cb(char *contents, size_t size,
						 size_t nmemb,
						 void *response_buffer)
{
	struct t_stringbuilder *sb_ptr;
	size_t realsize;

	/* calculate buffer size */
	realsize = size * nmemb;

	/* cast pointer to stringbuilder */
	sb_ptr = (struct t_stringbuilder *)response_buffer;

	if (!sb_ptr)
		return -1;

	stringbuilder_append_nbytes(sb_ptr, contents, strlen(contents));

	/* return size */
	return realsize;
}

/*
 * Set a api client request property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_free_request_fields
 */

void api_client_request_set_pointer(struct t_api_client_request *request,
				    const char *property, void *pointer)
{
	if (!request || !property)
		return;

	if (string_strcasecmp(property, "callback_free_request_fields") == 0)
		request->callback_free_request_fields = pointer;
}

/*
 * Get a request with the provided fields to the url.
 *
 * Returns a request_pointer if success, otherwise NULL.
 */

struct t_api_client_request *
	api_client_request_json(struct t_api_client *client,
				json_object *fields, const char *url,
				const char *name, const char *request_type)
{
	struct t_api_client_request *request;
	char *request_fields;

	if (!client || !fields || !url)
		return NULL;

	request_fields = string_strdup(json_object_to_json_string(fields));
	if (!request_fields)
		return NULL;

	/* safe to free json object */
	json_object_put(fields);

	/* create a new request */
	request = api_client_request_create(client, url, request_type, name,
					    request_fields);

	/* add some default headers if post request */
	if (request->type_request == API_CLIENT_REQUEST_POST) {
		api_client_request_add_header(request,
					      "Accept: application/json");
		api_client_request_add_header(request,
					      "Content-Type: application/json");
	}

	/* init request, with headers */
	api_client_init_request(client, request, 1);

	return request;
}

/*
 * Get a POST request with the provided fields to the url.
 *
 * Returns a request_pointer if success, otherwise NULL.
 */

struct t_api_client_request *
	api_client_request_post_json(struct t_api_client *client,
				     json_object *fields, const char *url, const char *name)
{
	return api_client_request_json(client, fields, url, name, "POST");
}

/*
 * Get a GET request with the provided fields to the url.
 *
 * Returns a request_pointer if success, otherwise NULL.
 */

struct t_api_client_request *
	api_client_request_get_json(struct t_api_client *client,
				    json_object *fields, const char *url, const char *name)
{
	return api_client_request_json(client, fields, url, name, "GET");
}

/*
 * Close a request, effectively removing it from the lookup table and freeing
 * the memory.
 */

void api_client_close_request(struct t_api_client *client,
			      struct t_api_client_request *request)
{
	if (!client || !request)
		return;

	hashtable_remove(client->request_table, request->name);
}

/*
 * Free an api client response.
 */

void api_client_response_free(struct t_api_client_response *response)
{
	if (!response)
		return;

	if (response->response_headers)
		stringbuilder_free(response->response_headers);
	if (response->response_body)
		stringbuilder_free(response->response_body);

	free(response);
}

/*
 * Free an api client request.
 */

void api_client_request_free(struct t_api_client_request *request)
{
	if (!request)
		return;

	if (request->handle)
		curl_easy_cleanup(request->handle);

	if (request->headers)
		curl_slist_free_all(request->headers);

	if (request->response)
		api_client_response_free(request->response);

	if (request->url)
		free(request->url);

	if (request->callback_free_request_fields)
		(void)(request->callback_free_request_fields)(
			request->request_fields);

	free(request);
}

/*
 * Free the api client's cookie.
 */

void api_client_cookie_free(struct t_api_client_cookie *cookie)
{
	if (!cookie)
		return;

	free(cookie);
}

/*
 * Free the api client.
 */

void api_client_free(struct t_api_client *client)
{
	if (!client)
		return;

	if (client->config)
		config_free(client->config);

	if (client->cookie)
		api_client_cookie_free(client->cookie);

	if (client->request_table)
		hashtable_free(client->request_table);

	free(client);
}

void api_client_free_minimal(struct t_api_client *client)
{
	if (!client)
		return;

	free(client->request_table);
	free(client);
}
