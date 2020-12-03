
#include <stdio.h>
#include <unistd.h>
#include <mysql/mysql.h>

#include "pipelined.h"

MYSQL sql_con;

int sql_init(char *host, char *usr, char *pwd, char *name)
{
	static const unsigned timeout = 1;

	if (mysql_init(&sql_con) == NULL) {
		fprintf(stdout, "mysql_init() %s\n", mysql_error(&sql_con));
		_exit(EXIT_NO_MYSQL);
	}

	if (mysql_options(&sql_con, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&timeout) != 0) {
		fprintf(stdout, "mysql_options() %s\n", mysql_error(&sql_con));
		_exit(EXIT_NO_MYSQL);
	}

	if (!mysql_real_connect(&sql_con, host, usr, pwd, name, 0, NULL, 0)) {
		fprintf(stdout, "mysql_real_connect() %s\n", mysql_error(&sql_con));
		_exit(EXIT_NO_MYSQL);
	}

	return 0;
}
