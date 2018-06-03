#include "pilab-api-request-state.h"

/*
 * Start doing a specified request.
 */

void api_request_default_do_request_cb()
{
}

/*
 * Stop doing a specified request.
 */

void api_request_default_stop_request_cb()
{
}

/*
 * Default implementation.
 */

void api_request_default_implementation(
	struct t_api_request_state *request_state)
{
	if (!request_state)
		return;

	request_state->do_it = api_request_default_do_request_cb;
	request_state->stop_it = api_request_default_stop_request_cb;
}

/*
 * Does the request.
 */

static void api_request_do(struct t_api_request_state *request_state)
{
	api_request_transition_to_do(request_state);
}

/*
 * Stop doing the request, effectively changing the state from do to stop.
 */

void api_request_transition_to_stop(struct t_api_request_state *request_state)
{
	if (!request_state)
		return;

	/* Initialise with the default implementation */
	api_request_default_implementation(request_state);
	request_state->do_it = api_request_do;
}

/*
 * Stops the request.
 */

static void api_request_stop(struct t_api_request_state *request_state)
{
	api_request_transition_to_stop(request_state);
}

/*
 * Start doing the request, effectively changing the state from stop_it
 * to do_it.
 */

void api_request_transition_to_do(struct t_api_request_state *request_state)
{
	if (!request_state)
		return;

	/* Initialise with the default implementation */
	api_request_default_implementation(request_state);
	request_state->stop_it = api_request_stop;
}
