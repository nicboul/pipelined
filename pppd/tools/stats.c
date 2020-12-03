/*
 * Dynamically add statistics
 * system: BSD/Linux
 *
 * Copyright 2007 Electronic Kludge inc.
 *
 * maintainer:	Nicolas Bouliane
 * 		nicboul@gmail.com
 *
 * to compile:
 * gcc stats.c -lmysqlclient -L/usr/lib/mysql
 * 
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <mysql/mysql.h>

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>


#define DB_IP 0x1
#define DB_USR 0x2
#define DB_PWD 0x4
#define DB_NAME 0x8
#define REMOTE_IP 0x16
#define IFACE 0x32
#define ACTION 0x64


struct db_info {
	char *db_ip;
	char *db_usr;
	char *db_pwd;
	char *db_name;
	char *remote_ip;
	char *iface;
	char *action;
};

int update_stats(struct db_info *opts)
{
	MYSQL sql_con;
	MYSQL_RES *query_result = NULL;
	MYSQL_ROW row_buf = NULL;

	char query_buf[512];
	int nrow = 0;
       
	if (mysql_init(&sql_con) == NULL) {
		syslog(LOG_ALERT, "mysql_init() %s\n", mysql_error(&sql_con));
		return -1;
	}
	
	if (!mysql_real_connect(&sql_con, opts->db_ip, opts->db_usr, opts->db_pwd, opts->db_name, 0, NULL, 0)) {
		syslog(LOG_ALERT, "mysql_real_connect() %s\n", mysql_error(&sql_con));
		return -1;
	}
	
	snprintf(query_buf, 512,
	 "UPDATE tunnel_status "
	 "SET is_online = 0 "
	 "WHERE remote_ip = '%s' "
	 "AND timestamp < NOW() ",
	 opts->remote_ip);

	if (mysql_query(&sql_con, query_buf) != 0) {
		syslog(LOG_ALERT, "mysql_query() %s\n", mysql_error(&sql_con));
		return -1;
	}

	query_result = mysql_store_result(&sql_con);
	if (query_result == NULL) {
		syslog(LOG_ALERT, "mysql_store_result() %s\n", mysql_error(&sql_con));
		return -1;
	}
        
	mysql_close(&sql_con);	

	return 0;
}

int add_stats(struct db_info *opts)
{
	MYSQL sql_con;
	MYSQL_RES *query_result = NULL;
	MYSQL_ROW row_buf = NULL;

	char query_buf[512];
	int nrow = 0;
       
	if (mysql_init(&sql_con) == NULL) {
		syslog(LOG_ALERT, "mysql_init() %s\n", mysql_error(&sql_con));
		return -1;
	}
	
	if (!mysql_real_connect(&sql_con, opts->db_ip, opts->db_usr, opts->db_pwd, opts->db_name, 0, NULL, 0)) {
		syslog(LOG_ALERT, "mysql_real_connect() %s\n", mysql_error(&sql_con));
		return -1;
	}
	
	/*
	INSERT INTO tunnel_status
	VALUES ((SELECT subnets.id FROM subnets where subnet = '255.255.255.255'), 'ppp0', '255.255.255.255', 1, NOW());
      	*/

	snprintf(query_buf, 512,
	 "INSERT INTO tunnel_status "
	 "VALUES ((SELECT subnets.id FROM subnets where subnet = '%s'), '%s', '%s', 1, NOW()); ",
	 opts->remote_ip, opts->iface, opts->remote_ip);

	if (mysql_query(&sql_con, query_buf) != 0) {
		syslog(LOG_ALERT, "mysql_query() %s\n", mysql_error(&sql_con));
		return -1;
	}

	query_result = mysql_store_result(&sql_con);
	if (query_result == NULL) {
		syslog(LOG_ALERT, "mysql_store_result() %s\n", mysql_error(&sql_con));
		return -1;
	}
        
	mysql_close(&sql_con);	

	return 0;
}

void usage()
{
	fprintf(stdout, "Usage:\n"
			"	-h	database hostname or ip\n"
			"	-u	database user\n"
			"	-p	database password\n"
			"	-n	database name\n"
			"	-a	remote ip address\n"
			"	-i	interface name\n"
			"	-x	action add/update\n"
	       );

}


int main(int argc, char *argv[])
{

	extern char *optarg;
	extern int optind;
	int ch;
	u_int32_t opt_f = 0;

	struct db_info opts;

	while ((ch = getopt(argc, argv, "h:u:p:n:i:a:x:")) != -1) {
		switch (ch) {

		case 'h':
			opts.db_ip = strdup(optarg);
			opt_f |= DB_IP;
			break;

		case 'u':
			opts.db_usr = strdup(optarg);
			opt_f |= DB_USR;
			break;

		case 'p':
			opts.db_pwd = strdup(optarg);
			opt_f |= DB_PWD;
			break;

		case 'n':
			opts.db_name = strdup(optarg);
			opt_f |= DB_NAME;
			break;

		case 'a':
			opts.remote_ip = strdup(optarg);
			opt_f |= REMOTE_IP;
			break;

		case 'i':
			opts.iface = strdup(optarg);
			opt_f |= IFACE;
			break;

		case 'x':
			opts.action = strdup(optarg);
			opt_f |= ACTION;
			break;
	
		default:
			usage();
			return -1;
		}
	}
	
	if (opt_f == 0) {
		usage();
		return -1;
	}
	
	openlog("STATS: ", LOG_USER, LOG_PID);

	if (~opt_f & DB_IP) {
		syslog(LOG_ALERT, "-h database ip, not specified\n");
		return -1;
	}

	if (~opt_f & DB_USR) {
		syslog(LOG_ALERT, "-u database user, not specified\n");
		return -1;
	}

	if (~opt_f & DB_PWD) {
		syslog(LOG_ALERT, "-p database password, not specified\n");
		return -1;
	}

	if (~opt_f & DB_NAME) {
		syslog(LOG_ALERT, "-n database name, not specified\n");
		return -1;
	}

	if (~opt_f & REMOTE_IP) {
		syslog(LOG_ALERT, "-a remote ip, not specified\n");
		return -1;
	}

	if (~opt_f & IFACE) {
		syslog(LOG_ALERT, "-i interface, not specified\n");
		return -1;
	}

	if (~opt_f & ACTION) {
		syslog(LOG_ALERT, "-x action, not specified\n");
		return -1;
	}

	syslog(LOG_ALERT, "db_ip: %s\n", opts.db_ip);
	syslog(LOG_ALERT, "db_usr: %s\n", opts.db_usr);
	syslog(LOG_ALERT, "db_pwd: %s\n", opts.db_pwd);
	syslog(LOG_ALERT, "db_name: %s\n", opts.db_name);
	syslog(LOG_ALERT, "remote_ip: %s\n", opts.remote_ip);
	syslog(LOG_ALERT, "interface: %s\n", opts.iface);
	syslog(LOG_ALERT, "action: %s\n", opts.action);

	if (*opts.action == 'a')  {
		add_stats(&opts);
	}
	if (*opts.action == 'u') {
		update_stats(&opts);
	}

	closelog();	

	return 0;
}
