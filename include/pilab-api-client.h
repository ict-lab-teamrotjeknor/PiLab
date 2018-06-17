#ifndef _PILAB_API_CLIENT
#define _PILAB_API_CLIENT
#include <curl/curl.h>
#include "pilab-hashtable.h"

#define PILAB_API_CLIENT_USER_AGENT "libcurl-agent/1.0"

#define PILAB_API_CLIENT_REQUEST_GET "GET"
#define PILAB_API_CLIENT_REQUEST_POST "POST"

typedef size_t(t_api_client_write_response_buffer)(void *contents, size_t size,
						   size_t nmemb,
						   void *response_buffer);
typedef void(t_api_client_request_free_post_fields)(void *postfield);

enum t_api_client_request_type {
	API_CLIENT_REQUEST_GET = 0,
	API_CLIENT_REQUEST_POST,
	/*
	 * Number of request types
	 */
	API_CLIENT_REQUEST_NUM_TYPES,
};

struct t_api_client_request {
	/*
	 * list containing the request headers, according to rfc2616 the order does
	 * not matter.
	 */
	struct curl_slist *headers;
	/*
	 * Request type.
	 */
	enum t_api_client_request_type type_request;
	/*
	 * Pointer to a URL encoded zero terminated string.
	 *
	 * For a greater explanation of the format please see RFC 3986
	 * (https://www.ietf.org/rfc/rfc3986.txt).
	 */
	const char *url;
	/*
	 * Request fields, that points to the full data to send in an HTTP POST
	 * operation.
	 *
	 * NOTE: This should be NULL if the type_request is not equal to
	 * API_CLIENT_REQUEST_POST.
	 */
	char *post_fields;
	/*
	 * The handle provided by the curl library.
	 */
	CURL *handle;
	/*
	 * Response of the last executed call.
	 */
	struct t_stringbuilder *response;

	/* Callbacks */

	/*
	 * Callback to free post fields.
	 */
	t_api_client_request_free_post_fields *callback_free_post_fields;
};

struct t_api_client {
	/*
	 * Hashtable containing, all the active and in-active requests.
	 */
	struct t_hashtable *request_table;
	/*
	 * The callback used for writing the data.
	 */
	t_api_client_write_response_buffer *callback_write;
};

extern int api_client_get_request_type(const char *type);
extern struct t_api_client *api_client_create(
	t_api_client_write_response_buffer *callback_write_response_buffer);
extern struct t_api_client_request *
	api_client_request_create(const char *url, const char *type_request,
				  char *post_fields);
extern int
	api_client_request_is_initialized(struct t_api_client_request *request);

extern void api_client_add_request(struct t_api_client *client,
				   struct t_api_client_request *request);
extern void
	api_client_request_add_post_fields(struct t_api_client_request *request,
					   char *post_fields);
extern void api_client_request_add_header(struct t_api_client_request *request,
					  const char *header);
extern struct t_api_client_request *
	api_client_get_request(struct t_api_client *client, const char *url);
extern int api_client_init_request(struct t_api_client *client,
				   struct t_api_client_request *request);
extern int api_client_request_execute(struct t_api_client_request *request);
extern int api_client_get_http_status_code_request(
	struct t_api_client *client, struct t_api_client_request *request);
extern char *
	api_client_request_get_response(struct t_api_client_request *request);
extern size_t api_client_write_response_buffer_default_cb(
	void *contents, size_t size, size_t nmemb, void *response_buffer);
extern void api_client_request_set_pointer(struct t_api_client_request *request,
					   const char *property, void *pointer);
extern void api_client_close_request(struct t_api_client *client,
				     struct t_api_client_request *request);
extern void api_client_request_free(struct t_api_client_request *request);
extern void api_client_free(struct t_api_client *client);

#endif
