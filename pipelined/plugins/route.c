
#include <stdio.h>
#include "../modules.h"

void init() __attribute__((constructor));
void fini() __attribute__((destructor));

void add_route()
{

	printf("im adding new route\n");
}

static struct plugin_opt route_opt = {
	.name	= "route",
	.exec	= &add_route,
};

void init()
{
	register_plugin(&route_opt);

}

void fini()
{
	unregister_plugin(&route_opt);
}


