
#ifndef __IPAUGENBLICK_LOG_H__
#define __IPAUGENBLICK_LOG_H__

enum
{
	IPAUGENBLICK_LOG_DEBUG,
	IPAUGENBLICK_LOG_INFO,
	IPAUGENBLICK_LOG_WARNING,
	IPAUGENBLICK_LOG_ERR,
	IPAUGENBLICK_LOG_CRIT,
	IPAUGENBLICK_LOG_NONE
};

void ipaugenblick_log_init(int);
void ipaugenblick_log(int, const char *, ...);
void ipaugenblick_set_log_level(int);

#endif
