#include <stdlib.h>
#include "pilab-list.h"
#include "pilab-string.h"
#include "pilab-api-client.h"

char *api_client_request_type_string[API_CLIENT_REQUEST_NUM_TYPES] = {
	PILAB_API_CLIENT_REQUEST_GET, PILAB_API_CLIENT_REQUEST_POST
};

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
		if (string_charcasecmp(api_client_request_type_string[i],
				       type) == 0)
			return i;

	/* type not found */
	return -1;
}

/*
 * Create a new api client.
 *
 * Returns a pointer to the newly created api client, NULL otherwise.
 */

struct t_api_client *api_client_create(
	t_api_client_write_response_buffer *callback_write_response_buffer)
{
	struct t_api_client *new_client;
	struct t_hashtable *new_hashtable;

	new_client = malloc(sizeof(*new_client));

	if (!new_client)
		return NULL;

	new_hashtable = hashtable_create(8, PILAB_HASHTABLE_STRING,
					 PILAB_HASHTABLE_POINTER, NULL, NULL);
	if (!new_hashtable)
		return NULL;

	new_hashtable->callback_free_value =
		&api_client_request_free_default_cb;

	new_client->request_table = new_hashtable;
	new_client->callback_write =
		(callback_write_response_buffer) ?
			callback_write_response_buffer :
			&api_client_write_response_buffer_default_cb;

	return new_client;
}

/*
 * Conjure up a new request.
 *
 * Returns a pointer to the newly created request, NULL otherwise.
 */

struct t_api_client_request *api_client_request_create(const char *url,
						       const char *type_request,
						       char *post_fields)
{
	struct t_api_client_request *new_request;
	struct t_stringbuilder *new_reponse_buffer;
	int type_request_int;

	new_request = malloc(sizeof(*new_request));

	if (!new_request)
		return NULL;

	type_request_int = api_client_get_request_type(type_request);
	if (type_request_int < 0)
		return NULL;

	new_reponse_buffer = stringbuilder_create();

	if (!new_reponse_buffer) {
		free(new_request);
		return NULL;
	}

	new_request->response = new_reponse_buffer;
	new_request->headers = NULL;
	new_request->url = url;
	new_request->type_request = type_request_int;
	new_request->post_fields = (post_fields) ? post_fields : "";
	new_request->handle = NULL;

	return new_request;
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
 * Add a new request to the client.
 */

void api_client_add_request(struct t_api_client *client,
			    struct t_api_client_request *request)
{
	if (!client || !request)
		return;

	hashtable_set(client->request_table, request->url, request);
}

/*
 * Add post fields to a request.
 */

void api_client_request_add_post_fields(struct t_api_client_request *request,
					char *post_fields)
{
	if (!request || !post_fields)
		return;

	request->post_fields = post_fields;
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
 * Get a request based on the provided URL.
 *
 * Returns a pointer to the request for the provided URL, or NULL if the URL is
 * not found.
 */

struct t_api_client_request *api_client_get_request(struct t_api_client *client,
						    const char *url)
{
	struct t_api_client_request *request;

	if (!client)
		return NULL;

	request = (struct t_api_client_request *)hashtable_get(
		client->request_table, url);

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
			    struct t_api_client_request *request)
{
	char *request_type;

	if (!client || !request)
		return -1;

	if (api_client_get_request(client, request->url)) {
		/* TODO: LOG entry: failed to initialise request */
		if ((request->handle = curl_easy_init()) == NULL)
			return 0;
		request_type =
			api_client_request_type_string[request->type_request];

		/* make curl aware of the url we want to fetch */
		curl_easy_setopt(request->handle, CURLOPT_URL, request->url);

		/* make curl aware of our write callback */
		curl_easy_setopt(request->handle, CURLOPT_WRITEFUNCTION,
				 client->callback_write);

		/* feed the fetched body to the struct pointer */
		curl_easy_setopt(request->handle, CURLOPT_WRITEDATA,
				 request->response);

		/* provide a default user agent */
		curl_easy_setopt(request->handle, CURLOPT_USERAGENT,
				 PILAB_API_CLIENT_USER_AGENT);

		/* follow redirects */
		curl_easy_setopt(request->handle, CURLOPT_FOLLOWLOCATION, 1);

		/* only one redirect though */
		curl_easy_setopt(request->handle, CURLOPT_MAXREDIRS, 1);

		/* custom string for request */
		curl_easy_setopt(request->handle, CURLOPT_CUSTOMREQUEST,
				 request_type);

		/* provide the headers */
		curl_easy_setopt(request->handle, CURLOPT_HTTPHEADER,
				 request->headers);

		/* provide the postfields */
		if (request->type_request == API_CLIENT_REQUEST_POST)
			curl_easy_setopt(request->handle, CURLOPT_POSTFIELDS,
					 request->post_fields);
	}

	return 1;
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

	if (!request)
		return -1;

	rc = curl_easy_perform(request->handle);

	/* TODO: Log entry: Failed to fetch url */
	if (rc != CURLE_OK || request->response->length < 1)
		return 0;

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

int api_client_get_http_status_code_request(
	struct t_api_client *client, struct t_api_client_request *request)
{
	int http_code;
	CURLcode rc;

	if (!client || !request)
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
 * Retrieve the response of a api client request.
 *
 * Returns a pointer to the internal response buffer, NULL otherwise.
 */

char *api_client_request_get_response(struct t_api_client_request *request)
{
	return (request) ? request->response->string : NULL;
}

/*
 * Write the received data to a provided response buffer (default callback).
 *
 * NOTE: When implementing your own callback, you must return the number of
 * bytes actually taken care of, as the library using the heavy lifting
 * (libcurl) will use this number to signal an error condition if this number
 * differs from the amount passed.
 *
 * Return the hash of the key, depending on the type of the key.
 */

size_t api_client_write_response_buffer_default_cb(void *contents, size_t size,
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

	stringbuilder_append_nbytes(sb_ptr, contents, realsize);

	/* return size */
	return realsize;
}

/*
 * Set a api client request property (pointer)
 *
 * NOTE: Currently the properties that are allowed to be set:
 * - callback_free_post_fields
 */

void api_client_request_set_pointer(struct t_api_client_request *request,
				    const char *property, void *pointer)
{
	if (!request || !property)
		return;

	if (string_strcasecmp(property, "callback_free_post_fields") == 0)
		request->callback_free_post_fields = pointer;
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

	hashtable_remove(client->request_table, request->url);
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
		stringbuilder_free(request->response);

	if (request->callback_free_post_fields)
		(void)(request->callback_free_post_fields)(
			request->post_fields);

	free(request);
}

/*
 * Free the api client.
 */

void api_client_free(struct t_api_client *client)
{
	if (!client)
		return;

	if (client->request_table)
		hashtable_free(client->request_table);

	free(client);
}
