#include <ipaugenblick_log.h>
#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>

static int g_ipaugenblick_log_dest = 0; /* 0 - stdio, 1 - syslog */

void ipaugenblick_log_init(int dest)
{
	g_ipaugenblick_log_dest = dest;
	switch(g_ipaugenblick_log_dest) {
		case 0:
		break;
		case 1:
			openlog(NULL, 0, LOG_DAEMON);
		break;
	}
}

void ipaugenblick_log(int level, const char* format, ...)
{
	va_list argptr;

	switch(g_ipaugenblick_log_dest) {
		case 0:			
    			va_start(argptr, format);
			vfprintf(stdout, format, argptr);
    			va_end(argptr);
		break;
			switch(level) {
				case IPAUGENBLICK_LOG_DEBUG:
					syslog(LOG_DEBUG, format, argptr);
				break;
				case IPAUGENBLICK_LOG_INFO:
					syslog(LOG_INFO, format, argptr);
				break;
				case IPAUGENBLICK_LOG_WARNING:
					syslog(LOG_WARNING, format, argptr);
				case IPAUGENBLICK_LOG_ERR:
					syslog(LOG_ERR,format, argptr);
				break;
				case IPAUGENBLICK_LOG_CRIT:
					syslog(LOG_CRIT,format, argptr);
				break;
			}
		case 1:
		break;
	}
}
