struct plugin_opt {

	struct plugin_opt *next;
	char *name;
	void (*exec)();
};

extern int register_plugin(struct plugin_opt *);
extern int unregister_plugin(struct plugin_opt *);



