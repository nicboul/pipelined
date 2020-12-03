/*
 * Dynamically add new route
 * system: Linux
 *
 * Copyright 2006, 2007  Electronic Kludge inc.
 * 
 * maintainer: Nicolas Bouliane
 *             nicboul@gmail.com
 *
 * to compile: 
 * gcc linux-newroute.c -lmysqlclient -L/usr/lib/mysql
 * 
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <linux/rtnetlink.h>
#include <linux/netlink.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <mysql/mysql.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define DB_IP 0x1
#define DB_USR 0x2
#define DB_PWD 0x4
#define DB_NAME 0x8
#define REMOTE_IP 0x16
#define IFACE 0x32


struct db_info {
	char *db_ip;
	char *db_usr;
	char *db_pwd;
	char *db_name;
	char *remote_ip;
	char *iface;
};

struct rtnl_handle
{
	int fd;
	struct sockaddr_nl local;
	struct sockaddr_nl peer;
	__u32 seq;
	__u32 dump;
};

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
		int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
		return -1;
       
	rta = (struct rtattr*)(((char*)n) + NLMSG_ALIGN(n->nlmsg_len));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

	return 0;
}

int rtnl_open(struct rtnl_handle *rth)
{
	int addr_len;
	memset(rth, 0, sizeof(rth));

	// Creating the netlink socket of family NETLINK_ROUTE
	rth->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if (rth->fd < 0) {
		perror("cannot open netlink socket");
		return -1;
	}

	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = 0;

	// Binding the netlink socket
	if (bind(rth->fd, (struct sockaddr*)&rth->local, sizeof(rth->local)) < 0)
	{
		perror("cannot bind netlink socket");
		return -1;
	}
	addr_len = sizeof(rth->local);

	if (getsockname(rth->fd, (struct sockaddr*)&rth->local, (socklen_t *)&addr_len) < 0)
	{
		perror("cannot getsockname");
		return -1;
	}

	if (addr_len != sizeof(rth->local)) {
		fprintf(stderr, "wrong address lenght %d\n", addr_len);
		return -1;
	}

	if (rth->local.nl_family != AF_NETLINK) {
		fprintf(stderr, "wrong address family %d\n", rth->local.nl_family);
		return -1;
	}
	rth->seq = time(NULL);
	return 0;
}


int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, pid_t peer,
		unsigned groups, struct nlmsghdr *answer)
{
	int status;
	struct sockaddr_nl nladdr;
	
	// Forming the iovector with the netlink packet.
	struct iovec iov = { (void*)n, n->nlmsg_len };

	// Forming the message to be sent.
	struct msghdr msg = {
		(void*)&nladdr, sizeof(nladdr),
		&iov, 1,
		NULL, 0,
		0
	};

	// Filling up the details of the netlink socket to be contacted in
	// the kernel.
	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;
	nladdr.nl_pid = peer;

	n->nlmsg_seq = ++rtnl->seq;
	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	// Actual sending of the message, status contains success/failure
	status = sendmsg(rtnl->fd, &msg, 0);

	if (status < 0)
		return -1;

	return 0;
}

int route_add(char *destination, u_int32_t bitmask, char *interface)
{
        struct rtnl_handle rth;
        struct {
                struct nlmsghdr     n;
                struct rtmsg        r;
                char                buf[1024];
        } req;

        memset(&req, 0, sizeof(req));

        req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
        req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
        req.n.nlmsg_type = RTM_NEWROUTE;
        req.r.rtm_family = AF_INET;
        req.r.rtm_table = RT_TABLE_MAIN;
        req.r.rtm_scope = RT_SCOPE_NOWHERE;
        req.r.rtm_protocol = RTPROT_BOOT;
        req.r.rtm_scope = RT_SCOPE_UNIVERSE;
        req.r.rtm_type = RTN_UNICAST;

        u_int32_t dst;
	dst = inet_addr(destination);
        req.r.rtm_dst_len = bitmask;
	
	addattr_l(&req.n, sizeof(req), RTA_DST, &dst, 4);

        int idx;
        if ((idx = if_nametoindex(interface)) == 0) {
		syslog(LOG_ALERT, "if_nametoindex() can't find device %s\n", interface);
                return -1;
        }
        addattr_l(&req.n, sizeof(req), RTA_OIF, &idx, 4);

	if (rtnl_open(&rth) < 0) {
		syslog(LOG_ALERT, "rtnl_open() failed\n");
		exit(-1);
	}

        if (rtnl_talk(&rth, &req.n, 0, 0, NULL) < 0) {
		syslog(LOG_ALERT, "rtnl_talk() failed\n");
                exit(-1);
        }

        return 0;
}

int fetch_route(struct db_info *opts)
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
	 "SELECT subnet, netmask "
	 "FROM subnets "
	 "WHERE id = (SELECT id FROM subnets WHERE subnet = '%s') "
	 "AND netmask <> 32", opts->remote_ip); // 32 stand for netmask 255.255.255.255
	
	if (mysql_query(&sql_con, query_buf) != 0) {
		syslog(LOG_ALERT, "mysql_query() %s\n", mysql_error(&sql_con));
		return -1;
	}

	query_result = mysql_store_result(&sql_con);
	if (query_result == NULL) {
		syslog(LOG_ALERT, "mysql_store_result() %s\n", mysql_error(&sql_con));
		return -1;
	}
	
	nrow = mysql_num_rows(query_result);

	if (nrow < 1) {
		syslog(LOG_ALERT, "no route to add to %s\n", opts->remote_ip);
		return -1;
	}

	/*
 	 * +---------------+---------+
	 * | subnet        | netmask |
	 * +---------------+---------+
	 * | 192.168.100.0 |      30 |
	 * +---------------+---------+
         *
	 * row_buf[0] is the subnet  [ MYSQL TYPE: STRING  ]
	 * row_buf[1] is the netmask [ MYSQL TYPE: INTEGER ]
	 */

	row_buf = mysql_fetch_row(query_result);
	if (row_buf == NULL) {
		syslog(LOG_ALERT, "mysql_fetch_row() %s\n", mysql_error(&sql_con));
		return -1;
	}
	       	
	do {
		syslog(LOG_ALERT, "ADD => %s/%s iface: %s\n", row_buf[0], row_buf[1], opts->iface);

		route_add(row_buf[0], strtoll(row_buf[1], NULL, 10), opts->iface);

	} while (row_buf = mysql_fetch_row(query_result));

        /*
		while (i < nrow) {
		row_buf = mysql_fetch_row(query_result);
		printf("subnet: %s\n", row_buf[0]);
		printf("mask: %s\n", row_buf[1]);
		route_add(row_buf[0], strtoll(row_buf[1], NULL, 10), opts->iface);
		i++;
	} */

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
			"	-a	remote ip address\n"
			"	-i	interface name\n");
}

int main(int argc, char *argv[])
{

	extern char *optarg;
	extern int optind;
	int ch;
	u_int32_t opt_f = 0;

	struct db_info opts;

	while ((ch = getopt(argc, argv, "h:u:p:n:i:a:")) != -1) {
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
	
		default:
			usage();
			return -1;
		}
	}
	
	if (opt_f == 0) {
		usage();
		return -1;
	}
	
	openlog("NEWROUTE: ", LOG_USER, LOG_PID);

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

	syslog(LOG_ALERT, "db_ip: %s\n", opts.db_ip);
	syslog(LOG_ALERT, "db_usr: %s\n", opts.db_usr);
	syslog(LOG_ALERT, "db_pwd: %s\n", opts.db_pwd);
	syslog(LOG_ALERT, "db_name: %s\n", opts.db_name);
	syslog(LOG_ALERT, "remote_ip: %s\n", opts.remote_ip);
	syslog(LOG_ALERT, "interface: %s\n", opts.iface);

	fetch_route(&opts);

	closelog();

	return 0;
}
