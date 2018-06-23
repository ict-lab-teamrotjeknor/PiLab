#ifndef _PILAB_API_CLIENT
#define _PILAB_API_CLIENT
#include <curl/curl.h>
#include <json-c/json.h>
#include "pilab-hashtable.h"
#include "pilab-config.h"

#define PILAB_API_CLIENT_USER_AGENT "libcurl-agent/1.0"

#define PILAB_API_CLIENT_REQUEST_GET "GET"
#define PILAB_API_CLIENT_REQUEST_POST "POST"

typedef size_t(t_api_client_write_response_body)(char *contents, size_t size,
						 size_t nmemb,
						 void *response_buffer);
typedef size_t(t_api_client_write_response_headers)(char *contents, size_t size,
						    size_t nmemb,
						    void *response_buffer);

typedef void(t_api_client_request_free_request_fields)(char *request_fields);

enum t_api_client_request_type {
	API_CLIENT_REQUEST_GET = 0,
	API_CLIENT_REQUEST_POST,
	/*
	 * Number of request types
	 */
	API_CLIENT_REQUEST_NUM_TYPES,
};

struct t_api_client_cookie {
	/*
	 * The cookie content.
	 */
	struct t_stringbuilder *content;
	/*
	 * When the cookie expires, extracted from the content.
	 */
	char *expires;
};

struct t_api_client_response {
	/*
	 * Response body of the last executed call.
	 */
	struct t_stringbuilder *response_body;
	/*
	 * Response headers of the last executed call.
	 */
	struct t_stringbuilder *response_headers;
};

struct t_api_client_request {
	/*
	 * Name of the request.
	 */
	const char *name;
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
	char *url;
	/*
	 * Request fields, that points to the full data to send in an HTTP POST
	 * operation.
	 *
	 * NOTE: This should be NULL if the type_request is not equal to
	 * API_CLIENT_REQUEST_POST.
	 */
	char *request_fields;
	/*
	 * The handle provided by the curl library.
	 */
	CURL *handle;
	/*
	 * Response of the request.
	 */
	struct t_api_client_response *response;

	/* Callbacks */

	/*
	 * Callback to free request fields.
	 */
	t_api_client_request_free_request_fields *callback_free_request_fields;
};

struct t_api_client {
	/*
	 * Settings for pilab.
	 */
	struct t_pilab_config *config;
	/*
	 * Hashtable containing, all the active and in-active requests.
	 */
	struct t_hashtable *request_table;
	/*
	 * A client can have a login cookie.
	 */
	struct t_api_client_cookie *cookie;

	/* Callbacks */

	/*
	 * The callback used for writing the body data.
	 */
	t_api_client_write_response_body *callback_write_response_body;
	/*
	 * The callback used for writing the header data.
	 */
	t_api_client_write_response_headers *callback_write_response_headers;
};

extern int api_client_get_request_type(const char *type);
extern struct t_api_client *api_client_create_custom(
	struct t_pilab_config *config,
	t_api_client_write_response_body *callback_write_response_body,
	t_api_client_write_response_headers *callback_write_response_headers);
extern struct t_api_client *api_client_create(struct t_pilab_config *config);
extern struct t_api_client_cookie *api_client_cookie_create(char *content);
extern struct t_api_client_request *api_client_request_create_custom(
	struct t_api_client *client, const char *url, const char *type_request,
	const char *name, char *request_fields,
	t_api_client_request_free_request_fields *callback_free_request_fields);
extern struct t_api_client_request *
	api_client_request_create(struct t_api_client *client, const char *url,
				  const char *name, const char *type_request,
				  char *request_fields);
extern struct t_api_client_response *api_client_response_create(void);
extern int
	api_client_request_is_initialized(struct t_api_client_request *request);
extern void api_client_request_add_request_fields(
	struct t_api_client_request *request, char *request_fields);
extern void api_client_request_add_header(struct t_api_client_request *request,
					  const char *header);
extern struct t_api_client_request *
	api_client_get_request(struct t_api_client *client, const char *name);
extern int api_client_init_request(struct t_api_client *client,
				   struct t_api_client_request *request,
				   int with_headers);
extern int api_client_is_valid_cookie(struct t_api_client_cookie *cookie);
extern char *api_client_get_cookie_content(struct t_api_client *client);
extern char *api_client_get_cookie_expire_date(struct t_api_client *client);
extern void api_client_set_cookie(struct t_api_client *client,
				  struct t_api_client_cookie *cookie);
extern char *api_client_request_get_header(struct t_api_client_request *request,
					   const char *header);
extern int api_client_request_execute(struct t_api_client_request *request);
extern int api_client_execute_all_requests(struct t_api_client *client);
extern int api_client_get_http_status_code_request(
	struct t_api_client_request *request);
extern char *api_client_request_get_response_body(
	struct t_api_client_request *request);
extern json_object *api_client_request_get_response_body_json(
	struct t_api_client_request *request);
extern char *api_client_request_get_response_headers(
	struct t_api_client_request *request);
extern size_t api_client_write_response_headers_default_cb(
	char *contents, size_t size, size_t nmemb, void *response_buffer);
extern size_t api_client_write_response_body_default_cb(char *contents,
							size_t size,
							size_t nmemb,
							void *response_buffer);
extern void api_client_request_set_pointer(struct t_api_client_request *request,
					   const char *property, void *pointer);
struct t_api_client_request *
	api_client_request_json(struct t_api_client *client,
				json_object *fields, const char *url,
				const char *name, const char *request_type);
struct t_api_client_request *
	api_client_request_post_json(struct t_api_client *client,
				     json_object *fields, const char *url,
				     const char *name);
extern struct t_api_client_request *
	api_client_request_get_json(struct t_api_client *client,
				    json_object *fields, const char *url,
				    const char *name);
extern void api_client_close_request(struct t_api_client *client,
				     struct t_api_client_request *request);
extern void api_client_response_free(struct t_api_client_response *response);
extern void api_client_request_free(struct t_api_client_request *request);
extern void api_client_cookie_free(struct t_api_client_cookie *cookie);
extern void api_client_free(struct t_api_client *client);
extern void api_client_free_minimal(struct t_api_client *client);
#endif
