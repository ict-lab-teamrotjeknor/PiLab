#ifndef _PILAB_API_CLIENT
#define _PILAB_API_CLIENT
#include "pilab-hashtable.h"

typedef size_t(t_api_client_write_response_buffer)(void *contents, size_t size,
						   size_t nmemb,
						   void *response_buffer);

#define MAX_RESPONSE_BUFFER 2048

/* TODO: Keep track of outgoing requests and create stop start implementation */
struct t_api_client {
	/*
	 * Hashtable containing the request headers, according to rfc2616 the order does
	 * not matter.
	 */
	struct t_hashtable *headers;
	/*
	 * Max number of bytes to buffer when reading the response content.
	 */
	size_t max_size_reponse_buffer;
	/*
	 * The callback used for writing the data.
	 */
	t_api_client_write_response_buffer *write_callback;
};

extern struct t_api_client *
	api_client_create_with_max_response_size(size_t max_size);
extern struct t_api_client *api_client_create(void);
extern void api_client_free(struct t_api_client *client);
extern void api_client_header_add(struct t_api_client *client, const char *key,
				  const char *value);
#endif
