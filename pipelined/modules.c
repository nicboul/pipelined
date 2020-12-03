
#include <stdio.h>

#include <dlfcn.h>
#include "pipelined.h"
#include "modules.h"


static struct plugin_opt *plugins_opt_list = NULL;

static void append_list(struct plugin_opt *p_opt)
{
	struct plugin_opt *i;

	if (plugins_opt_list != NULL) {
		i = plugins_opt_list;

		while (i->next != NULL) {
			i = i->next;
		}

		i->next = p_opt;
		i->next->next = NULL;

	}

	else {
		plugins_opt_list = p_opt;
		plugins_opt_list->next = NULL;
	}
}

static void dump_list()
{
	struct plugin_opt *i;
	i = plugins_opt_list;

	while (i != NULL)
	{
		printf("name: %s\n", i->name);
		i = i->next;
	}
}


int modules_init(char *plugins)
{
	void *handle;

	handle = dlopen("./plugins/route", RTLD_GLOBAL | RTLD_NOW);
	if (handle == NULL)
		printf("prout\n");


//	dlclose(handle);


}

int register_plugin(struct plugin_opt *p_opt)
{
	append_list(p_opt);
	return 0;
}

int unregister_plugin(struct plugin_opt *p_opt)
{
	dump_list();
	return 0;
}
