
#include <stdio.h>

#include "pipelined.h"
#include "sql.h"
#include "modules.h"

void *db_name = NULL;
void *db_usr = NULL;
void *db_pwd = NULL;
void *db_host = NULL;
void *plugins = NULL;

struct options opts[] = {

	{ "db_name", &db_name },
	{ "db_usr", &db_usr },
	{ "db_pwd", &db_pwd },
	{ "db_host", &db_host },
	{ "plugins", &plugins },

	{ NULL }
};


int main(int argc, char *argv[])
{

	pipelined_state = INIT_STATE;

	syslog_init();

	/*
	 * are we running as root ?
	 */
	if (getuid() != 0) {
		fprintf(stderr, "%s must be run as root\n", argv[0]);
		_exit(EXIT_NOT_ROOT);
	}

	option_parse(opts);
	option_dump(opts);

//	sql_init(db_host, db_usr, db_pwd, db_name);

	modules_init(plugins);

	muxcom_init();
	muxcom_sched();

	return 0;
}

