#include <stdlib.h>
#include <curl/curl.h>
#include "pilab-api-client.h"

/*
 * Create a new api client.
 *
 * Returns a pointer to the newly created api client, NULL otherwise.
 */

struct t_api_client *api_client_create()
{
	struct t_api_client *new_client;
	struct t_hashtable *new_hashtable;

	new_client = malloc(sizeof(*new_client));
	new_hashtable = hashtable_create(8, PILAB_HASHTABLE_STRING,
					 PILAB_HASHTABLE_STRING, NULL, NULL);
	if (!new_client)
		return NULL;

	if (!new_hashtable)
		return NULL;

	new_client->headers = new_hashtable;
	new_client->write_callback = NULL;

	return new_client;
}

/*
 * Every beginning has an end!
 */

void api_client_free(struct t_api_client *client)
{
	if (!client)
		return;

	if (client->headers)
		hashtable_free(client->headers);

	free(client);
	client = NULL;
}

/*
 * If you need to add headers.
 */

void api_client_header_add(struct t_api_client *client, const char *key,
			   const char *value)
{
	if (!client)
		return;

	hashtable_set(client->headers, key, value);
}

/*
 * If you need to remove some headers!
 */

void api_client_header_remove(struct t_api_client *client, const char *key)
{
	if (!client)
		return;

	hashtable_remove(client->headers, key);
}

/* size_t t_api_client_write_response_buffer_default_cb(void *contents, size_t size, size_t nmemb, void *response_buffer) */
/* { */
/* 	size_t rsize; */

/* 	/\* calculate the actual number of bytes written *\/ */
/* 	rsize = size * nmemb; */
/* } */
