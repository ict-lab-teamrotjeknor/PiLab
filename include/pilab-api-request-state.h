#ifndef _PILAB_API_REQUEST_STATE_H
#define _PILAB_API_REQUEST_STATE_H

struct t_api_request_state;

/* TODO:  Maybe add an additional pause state */
typedef void(t_api_request_do_request)(
	struct t_api_request_state *request_state);
typedef void(t_api_request_stop_request)(
	struct t_api_request_state *request_state);

struct t_api_request_state {
	t_api_request_do_request *do_it;
	t_api_request_stop_request *stop_it;
};

extern void api_request_default_implementation(
	struct t_api_request_state *request_state);
extern void api_request_transition_to_stop(
	struct t_api_request_state *request_state);
extern void api_request_transition_to_do(
	struct t_api_request_state *request_state);

#endif
