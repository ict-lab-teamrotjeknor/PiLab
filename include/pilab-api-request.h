#ifndef _PILAB_API_REQUEST_H
#define _PILAB_API_REQUEST_H
#include "pilab-api-request-state.h"

struct t_api_request {
	struct t_api_request_state state;
};

extern struct t_api_request *api_request_create(void);
extern void api_request_free(struct t_api_request *api_request);
extern void api_request_do(struct t_api_request *api_request);
extern void api_request_stop(struct t_api_request *api_request);

#endif
