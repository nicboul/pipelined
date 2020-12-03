
/*
 * Pipeline authentication plugin
 * Copyright 2006,2007 Electronic Kludge inc.
 *
 * maintainer: Nicolas Bouliane
 *             nicboul@gmail.com
 */

#define MAX_STR_LEN 32
#define MAX_USR_LEN 64
#define BUF_SIZE 512

#define NOT_AUTHENTICATED 0
#define AUTHENTICATED 1

#include "pppd.h"
#include "chap-new.h"
#include "fsm.h"
#include "ccp.h"

#include <mysql/mysql.h>

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <net/ppp-comp.h>

char pppd_version[] = VERSION;

char VALID_CHAR[] = "[0-9a-zA-Z._@]";

char *db_name = NULL;
char *db_usr = NULL;
char *db_pwd = NULL;
char *db_host = NULL;

char *t_cred = NULL;
char *f_cred_id = NULL;
char *f_cred_usr = NULL;
char *f_cred_pwd = NULL;
char *f_cred_ena = NULL;

char *t_usr_srv_m = NULL;
char *f_usr_srv_m_id = NULL;
char *f_usr_srv_m_name = NULL;
char *f_usr_srv_m_ena = NULL;

char *t_usr_srv = NULL;
char *f_usr_srv_name = NULL;

char *t_sn = NULL;
char *f_sn_id = NULL;
char *f_sn_sn = NULL;
char *f_sn_nm = NULL;

char *t_stats = NULL;

struct user_info_s {

	u_int8_t pipeline;
	u_int8_t mppe;

	int id;
	char pwd[MAX_STR_LEN];
	int pwd_len;
	u_int32_t ip;
	MYSQL sql_con;

} user_info = {0, 0};

static option_t opts[] = {

	{ "db_name", o_string, &db_name },
	{ "db_usr", o_string, &db_usr },
	{ "db_pwd", o_string, &db_pwd },
	{ "db_host", o_string, &db_host },

	{ "t_cred", o_string, &t_cred },
	{ "f_cred_id", o_string, &f_cred_id },
	{ "f_cred_usr", o_string, &f_cred_usr },
	{ "f_cred_pwd", o_string, &f_cred_pwd },
	{ "f_cred_ena", o_string, &f_cred_ena },

	{ "t_usr_srv_m", o_string, &t_usr_srv_m },
	{ "f_usr_srv_m_id", o_string, &f_usr_srv_m_id },
	{ "f_usr_srv_m_name", o_string, &f_usr_srv_m_name },
	{ "f_usr_srv_m_ena", o_string, &f_usr_srv_m_ena },

	{ "t_usr_srv", o_string, &t_usr_srv },
	{ "f_usr_srv_name", o_string, &f_usr_srv_name },

	{ "t_sn", o_string, &t_sn },
	{ "f_sn_id", o_string, &f_sn_id },
	{ "f_sn_sn", o_string, &f_sn_sn },
	{ "f_sn_nm", o_string, &f_sn_nm },

	{ "t_stats", o_string, &t_stats },

	{ NULL }
};


void ip_choose(u_int32_t *address)
{
	info("PIPELINE: Entering ip_choose()");

	*address = user_info.ip;
	return;
}

int allowed_address(u_int32_t address)
{
	info("PIPELINE: Entering allowed_address()");

	return (address == user_info.ip);
}

int valid_str(char *p)
{
	/* regex() is a kludge,
	 * anyway, our needs are pretty easy to fit
         * valid char are only [0-9a-zA-Z._]
	 */
	while ((*p >= '0' && *p <= '9') ||
	       (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
               (*p == '.') || (*p == '_') || (*p == '@') && *p != '\0') {

		p++;
	}

	if (*p == '\0')
		return 0;
	else
		return -1;
}

int valid_opts()
{
	int err = 0;
	int i = 0;

	/* we perform some sanity check on the
	 * options provided through the config file
	 */
	while (opts[i].name && !err) {
		if (*(char **)opts[i].addr == NULL) {
			error("PIPELINE CONF: option [%s] is not set\n", opts[i].name);
			err = -1;
		}
		else if (strlen(*(char **)opts[i].addr) > MAX_STR_LEN) {
			error("PIPELINE CONF: option [%s] is too long: \"%s\"\n", opts[i].name, *(char **)opts[i].addr);
			err = -1;
		}
		else if (!!valid_str(*(char **)opts[i].addr)) {
			error("PIPELINE CONF: option [%s] contain invalid char: \"%s\"\n", opts[i].name, *(char **)opts[i].addr);
			err = -1;
		}
		i++;
	}

	return err;
}

void init_mppe()
{
	info("PIPELINE: Entering init_mppe()");

	/* ccp.c: 124
	 * "We use ccp_allowoptions[0].mppe as a junk var ... it is reset later"
	 *
	 * So we _must_ use ccp_wantoptions[0].mppe
	 * to switch on/off mppe.
	 */

	if (user_info.mppe == 1) {
		ccp_allowoptions[0].mppe = 1;
		ccp_wantoptions[0].mppe = MPPE_OPT_128;
	}
	else {
		ccp_allowoptions[0].mppe = 0;
		ccp_wantoptions[0].mppe = 0;
	}
}

int init_pipeline(char *name)
{
	info("PIPELINE: Entering init_pipeline()");

	char query_buf[BUF_SIZE];
	int size = 0;
	unsigned long *len = NULL;
	int nrow = 0;

	MYSQL_RES *query_result = NULL;
	MYSQL_ROW row_buf = NULL;

	/*
	 * select users_services.name, password, subnet, credentials.id, users_services_map.enabled
	 * FROM users_services, users_services_map, subnets, credentials
	 * WHERE username = 'user'
	 * AND credentials.id = users_services_map.id
	 * AND credentials.id = subnets.id
	 * AND users_services_map.name = users_services.name
	 * AND credentials.enabled = 1
	 * AND netmask = 32;
	 */

	size = snprintf(query_buf, BUF_SIZE,
	"SELECT %s.%s, %s, %s, %s.%s, %s.%s  "
	"FROM %s, %s, %s, %s "
	"WHERE %s = '%s' "
	"AND %s.%s = %s.%s "
	"AND %s.%s = %s.%s "
	"AND %s.%s = %s.%s "
	"AND %s.%s = 1 "
	"AND %s = 32",  // netmask 255.255.255.255

	t_usr_srv, f_usr_srv_name, f_cred_pwd, f_sn_sn, t_cred, f_cred_id, t_usr_srv_m, f_usr_srv_m_ena,
	t_usr_srv, t_usr_srv_m, t_sn, t_cred,
        f_cred_usr, name,
	t_cred, f_cred_id, t_usr_srv_m, f_usr_srv_m_id,
	t_cred, f_cred_id, t_sn, f_sn_id,
	t_usr_srv_m, f_usr_srv_m_name, t_usr_srv, f_usr_srv_name,
	t_cred, f_cred_ena,
	f_sn_nm);

	if (size >= BUF_SIZE) {
		error("PIPELINE MYSQL: query_buf[] is too short. Line: %d File: %s\n", __LINE__, __FILE__);
		return -1;
	}

	if (mysql_query(&user_info.sql_con, query_buf) != 0) {
		error("PIPELINE MYSQL: mysql_query() %s\n", mysql_error(&user_info.sql_con));
		return -1;
	}

	query_result = mysql_store_result(&user_info.sql_con);
	if (query_result == NULL) {
		error("PIPELINE MYSQL: mysql_store_result() %s\n", mysql_error(&user_info.sql_con));
		return -1;
	}

	nrow = mysql_num_rows(query_result);

	if (nrow < 1) {
		error("PIPELINE MYSQL: user '%s' is not allowed to connect\n", name);
		return -1;
	}

	row_buf = mysql_fetch_row(query_result);
	if (row_buf == NULL) {
		error("PIPELINE MYSQL: mysql_fetch_row() %s\n", mysql_error(&user_info.sql_con));
		return -1;
	}

	len = mysql_fetch_lengths(query_result);

	/*
	 * +----------+-------------+------------+----+---------+
	 * | name     | password    | subnet     | id | enabled |
	 * +----------+-------------+------------+----+---------+
	 * | pipeline | user pass   | 16820416   |  3 |       1 |
	 * | webadmin | user pass   | 16820416   |  3 |       1 |
	 * | mppe     | user pass   | 16820416   |  3 |       1 |
	 * +----------+-------------+------------+----+---------+
         *
	 * row_buf[0] is the name field
	 * row_buf[1] is the password field
	 * row_buf[2] is the subnet field
	 * row_buf[3] is the id field
	 * row_buf[4] is the enabled flag
	 */

	user_info.pwd_len = len[1];
	strncpy((char*)&user_info.pwd, row_buf[1], user_info.pwd_len);

	user_info.ip = inet_addr(row_buf[2]);
	sscanf(row_buf[3], "%d", &user_info.id);

	info("uname: %s\n", name);
	info("pwr: %s\n", user_info.pwd);
	info("ip : %d\n", user_info.ip);
	info("id : %d\n", user_info.id);

	do {
		if (strcmp(row_buf[0], "pipeline") == 0) {
			if (strcmp(row_buf[4], "1") == 0)
				user_info.pipeline = 1;
		}
		else if (strcmp(row_buf[0], "mppe") == 0) {
			if (strcmp(row_buf[4], "1") == 0)
				user_info.mppe = 1;
		}

	} while (row_buf = mysql_fetch_row(query_result));

	info("pipeline: %d\n", user_info.pipeline);
	info("mppe: %d\n", user_info.mppe);

	if (user_info.pipeline != 1) {
		return -1;
	}

	return 0;
}

int init_mysql()
{
	static const unsigned timeout = 1;

	info("PIPELINE: Entering init_mysql()");

	if (mysql_init(&user_info.sql_con) == NULL) {
		error("PIPELINE MYSQL: mysql_init() %s\n", mysql_error(&user_info.sql_con));
		return -1;
	}

	if (mysql_options(&user_info.sql_con, MYSQL_OPT_CONNECT_TIMEOUT, &timeout) != 0) {
		info("PIPELINE MYSQL: mysql_options() unknown option");
	}

	if (!mysql_real_connect(&user_info.sql_con, db_host, db_usr, db_pwd, db_name, 0, NULL, 0)) {
		error("PIPELINE MYSQL: mysql_real_connect() %s\n", mysql_error(&user_info.sql_con));
		return -1;
	}

	return 0;
}

int sanity_check(char *name)
{
	info("PIPELINE: Entering sanity_check()");

	if (valid_opts() != 0) {
		return -1;
	}

	if (strlen(name) > MAX_USR_LEN) {
		error("PIPELINE AUTH: username is too long: \"%s\"\n", name);
		return -1;
	}

	if (!!valid_str(name)) {
		error("PIPELINE AUTH: username contain invalid char: \"%s\"\n", name);
		return -1;
	}

	return 0;
}

int chap_verify(char *name, char *ourname, int id,
		struct chap_digest_type *digest,
		unsigned char *challenge, unsigned char *response,
		char *message, int message_space)
{
	int err;

	info("PIPELINE: Entering chap_verify()");

	if (sanity_check(name) != 0) {

		slprintf(message, message_space, "E=691");
		return NOT_AUTHENTICATED;
	}

	if (init_mysql() != 0) {

		slprintf(message, message_space, "E=643");
		return NOT_AUTHENTICATED;
	}

	if (init_pipeline(name) != 0) {

		slprintf(message, message_space, "E=691");
		return NOT_AUTHENTICATED;
	}

	/*
	 * we MUST switch mppe on/off before authentication because:
         *
	 * The initial (MPPE) session key in both directions is derived from the
	 * credentials of the peer that initiated the call and the challenge
	 * used (if any) is the challenge from the first authentication.
	 *
	 * RFC(3079)
	 */

	init_mppe();

	err = digest->verify_response(id, name, user_info.pwd, user_info.pwd_len, challenge,
		                                          response, message, message_space);
	if (err != 0)
		return AUTHENTICATED;

	return NOT_AUTHENTICATED;
}

int idle_time(struct ppp_idle *idlep)
{
	info("PIPELINE: Entering idle_time()");

	// no idle timeout
	return 0;
}

void update_stats()
{
	info("PIPELINE: Entering update_stats()");

	char query_buf[BUF_SIZE];
	int size = 0;

	size = snprintf(query_buf, BUF_SIZE,
	"INSERT INTO %s VALUES(%d, %lu, %lu, %lu, %lu, %lu, NULL, NOW())",
		t_stats,
		user_info.id, link_connect_time,
		link_stats.bytes_out, link_stats.bytes_in,
		link_stats.pkts_out, link_stats.pkts_in);

	if (size >= BUF_SIZE) {
		error("PIPELINE MYSQL: query_buf[] is too short. Line: %d File: %s\n", __LINE__, __FILE__);
		return;
	}

	if (mysql_query(&user_info.sql_con, query_buf) != 0) {
		error("PIPELINE MYSQL: mysql_query() %s\n", mysql_error(&user_info.sql_con));
		return;
	}
}

void ip_down(void *opaque, int arg)
{
	info("PIPELINE: Entering ip_down()");

	update_stats();
	mysql_close(&user_info.sql_con);
}

void ip_up(void *opaque, int arg)
{
	info("PIPELINE: Entering ip_up()");
}

void plugin_init(void)
{
	info("PIPELINE: Entering plugin_init()");

	add_options(opts);

	chap_verify_hook = chap_verify;
	ip_choose_hook = ip_choose;
	allowed_address_hook = allowed_address;
	idle_time_hook = idle_time;

	add_notifier(&ip_down_notifier, ip_down, NULL);
	add_notifier(&ip_up_notifier, ip_up, NULL);

	return;
}

