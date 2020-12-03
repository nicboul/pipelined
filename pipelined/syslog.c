
#include <stdarg.h>
#include <syslog.h>

#include "pipelined.h"

/**
 * pipelined logging facilities
 * @ident: The string pointed to by ident is prepending to every message  
 * @priority: This determines the importance of the message
 * @format: This is the payload format
 * @...: This is the payload message
*/ 
void plog(char *ident, int priority, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	openlog(ident, LOG_PID, LOG_DAEMON);
	vsyslog(priority, format, ap);

	va_end(ap);
	closelog();

	return;
}


void syslog_init() {
	
	openlog("pipelined", LOG_PID | LOG_NDELAY, LOG_DAEMON);	
	setlogmask(LOG_UPTO(LOG_INFO));

	return;
}


