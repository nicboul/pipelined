/*
 * Dynamically add new route
 * system: netBSD
 *
 * Copyright 2006, 2007  Electronic Kludge inc.
 * 
 * maintainer: Nicolas Bouliane
 *             nicboul@gmail.com
 *
 * to compile: 
 * gcc bsd-newroute.c -lmysqlclient -I/usr/pkg/include
 *
 */

#include <sys/ioctl.h>

#include <sys/param.h>
#include <sys/socket.h>

#include <net/if.h>
#include <net/route.h>
#include <net/if_dl.h>

#include <arpa/inet.h>

#include <mysql/mysql.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define MAX_INT 4294967295

#define DB_IP 0x1
#define DB_USR 0x2
#define DB_PWD 0x4
#define DB_NAME 0x8
#define REMOTE_IP 0x10

struct db_info {
	
	char *db_ip;
	char *db_usr;
	char *db_pwd;
	char *db_name;
	char *remote_ip;
}; 

union sockunion {

	struct sockaddr sa;
	struct sockaddr_in sin;

} so_dst, so_mask, so_gate;


struct {
	struct rt_msghdr m_rtm;
	char m_space[512];
} m_rtmsg;


#define ROUNDUP(a) \
	((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

#define NEXTADDR(w, u) \
	if (rtm_addrs & (w)) { \
		sz = ROUNDUP(u.sa.sa_len); memmove(cp, &(u), sz); cp += sz; \
	}

int newroute(u_int32_t subnet, u_int32_t subnetmask, u_int32_t gateway)
{

	char *cp = m_rtmsg.m_space;
	int nbytes = 0;
	int sz = 0;

	int rtm_addrs = RTA_DST | RTA_NETMASK | RTA_GATEWAY; 

	int sock = 0;
	
	errno = 0;

	memset(&m_rtmsg, 0, sizeof(m_rtmsg));

	
	so_dst.sa.sa_len = sizeof(so_dst);
	so_dst.sa.sa_family = AF_INET;
	so_dst.sin.sin_addr.s_addr = subnet;

	syslog(LOG_ALERT, "subnet: %s\n", inet_ntoa(so_dst.sin.sin_addr));
	
	so_mask.sa.sa_len = sizeof(so_mask);
	so_mask.sa.sa_family = AF_INET;
	so_mask.sin.sin_addr.s_addr = subnetmask;

	syslog(LOG_ALERT, "subnetmask: %s\n", inet_ntoa(so_mask.sin.sin_addr));

	so_gate.sa.sa_len = sizeof(so_gate);
	so_gate.sa.sa_family = AF_INET;
	so_gate.sin.sin_addr.s_addr = gateway;

	syslog(LOG_ALERT, "gateway: %s\n", inet_ntoa(so_gate.sin.sin_addr));


#define rtm m_rtmsg.m_rtm
	rtm.rtm_type = RTM_ADD;
	rtm.rtm_flags = RTF_UP | RTF_GATEWAY;
	rtm.rtm_version = RTM_VERSION;
	rtm.rtm_addrs = rtm_addrs;
	
	NEXTADDR(RTA_DST, so_dst);
	NEXTADDR(RTA_GATEWAY, so_gate);
	NEXTADDR(RTA_NETMASK, so_mask);

	rtm.rtm_msglen = cp - (char *)&m_rtmsg;

	sock = socket(PF_ROUTE, SOCK_RAW, 0);
	if ((nbytes = write(sock, (char *)&m_rtmsg, rtm.rtm_msglen)) < 0) {
		syslog(LOG_ALERT, "writing to routing socket: %s", strerror(errno));
		return -1;
	}

	if (nbytes < 1) {
		syslog(LOG_ALERT, "write to routing socket, got %i for nbytes", nbytes);
		return -1;
	}

	close(sock);

	return 0;	
}

fetch_route(struct db_info *opts)
{

	char query_buf[512];
	int nrow = 0, i = 0;

	u_int32_t subnet;
	u_int32_t subnetmask;
	u_int32_t gateway;

	MYSQL sql_con;
	MYSQL_RES *query_result = NULL;
	MYSQL_ROW row_buf = NULL;

	mysql_init(&sql_con);

	inet_aton(opts->remote_ip, (struct in_addr *)&gateway);
	if (!mysql_real_connect(&sql_con, opts->db_ip, opts->db_usr, opts->db_pwd, opts->db_name, 0, NULL, 0)) {
		syslog(LOG_ALERT, "mysql_real_connect: %s\n", mysql_error(&sql_con));
	}

	snprintf(query_buf, 512,
	 "SELECT subnet, netmask "
	 "FROM subnets "
	 "WHERE id = (SELECT id FROM subnets WHERE subnet = '%lu') "
	 "AND netmask <> '%lu'", gateway, (unsigned long)MAX_INT); // MAX_INT stand for /32 netmask

	if (mysql_query(&sql_con, query_buf) != 0) {
		syslog(LOG_ALERT, "mysql_query: %s\n", mysql_error(&sql_con));
		return -1;
	}

	query_result = mysql_store_result(&sql_con);
	nrow = mysql_num_rows(query_result);

	if (nrow < 1) {
		syslog(LOG_ALERT, "no route to add to : %s\n", opts->remote_ip);
		return -1;
	}
	
	while (i < nrow) {
	
		row_buf = mysql_fetch_row(query_result);

		subnet = strtoll(row_buf[0], NULL, 10);
		subnetmask = strtoll(row_buf[1], NULL, 10);
		newroute(subnet, subnetmask, gateway);

		i++;
	}

	mysql_close(&sql_con);	

	return 0;
}

void usage()
{

	fprintf(stdout, "Usage:\n"
			"	-h	database's ip\n"
			"	-u	database's user\n"
			"	-p	database's password\n"
			"	-n	database name to use\n"
			"	-i	remote ip address\n");
}


int main(int argc, char *argv[])
{

	extern char *optarg;
	extern int optind;
	int ch;
	u_int16_t opt_f = 0;

	struct db_info opts;
	
	while ((ch = getopt(argc, argv, "h:u:p:n:i:")) != -1) {
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

		case 'i':
			opts.remote_ip = strdup(optarg);
			opt_f |= REMOTE_IP;
			break;
		default:
			usage();
			return -1;
		}
	}

	openlog("NEWROUTE: ", LOG_USER, LOG_PID); 

	if (~opt_f & DB_IP) {
		syslog(LOG_ALERT, "-h database ip, not specified \n");
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
		syslog(LOG_ALERT, "-i remote ip, not specified\n");
		return -1;
	}

	syslog(LOG_ALERT, "db_ip: %s\n", opts.db_ip);
	syslog(LOG_ALERT, "db_usr: %s\n", opts.db_usr);
	syslog(LOG_ALERT, "db_pwd: %s\n", opts.db_pwd);
	syslog(LOG_ALERT, "db_name: %s\n", opts.db_name);
	syslog(LOG_ALERT, "remote_ip: %s\n", opts.remote_ip);

	fetch_route(&opts);

	closelog();

	return 0;
}
