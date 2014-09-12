#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

#include <specific_includes/uapi/linux/stddef.h>


#undef NULL
#define NULL ((void *)0)

/*enum {
	false	= 0,
	true	= 1
};*/

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})
#endif
