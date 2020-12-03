
#include <unistd.h> // _exit()


#define OPTIONS_FILE "./pipelined.ini"


#define INIT_STATE 0x1
#define DAEM_STATE 0x2

 int pipelined_state;

#define EXIT_NOT_ROOT 1
#define EXIT_NO_MYSQL 2
#define EXIT_ERR_PARS 3

struct options {
	char *tag;
	void *value;
};



/*
 * procedures
 */

/* exported from options.c */
int option_parse();
void option_error(char *, ...);
void option_dump(struct options *);

/* exported from syslog.c */
void syslog_init();

/* exported from modules.c */
int modules_init(char *);

/* exported from muxcom.c */
int muxcom_init();


