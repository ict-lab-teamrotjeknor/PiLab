#include <stdlib.h>
#include "pilab-api-request.h"

struct t_api_request *api_request_create()
{
	struct t_api_request *new_request;

	new_request = malloc(sizeof(*new_request));

	/* TODO: Log error: Could not allocate memory for new api_request */
	if (!new_request)
		return NULL;

	api_request_transition_to_stop(&new_request->state);

	return new_request;
}

void api_request_free(struct t_api_request *api_request)
{
	if (!api_request)
		return;

	free(api_request);
}

void api_request_do(struct t_api_request *api_request)
{
	if (!api_request)
		return;

	api_request->state.do_it(&api_request->state);
}

void api_request_stop(struct t_api_request *api_request)
{
	if (!api_request)
		return;

	api_request->state.stop_it(&api_request->state);
}

