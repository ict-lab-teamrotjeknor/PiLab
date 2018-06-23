#ifndef _PILAB_API_CALLS_H
#define _PILAB_API_CALLS_H
#include "pilab-api-client.h"

extern void pilab_login(struct t_api_client *client);
extern void pilab_add_pi(struct t_api_client *client);
extern void pilab_add_sensor(struct t_api_client *client, const char *name,
			     const char *type_value);
extern void pilab_add_data(struct t_api_client *client, const char *value);

#endif
