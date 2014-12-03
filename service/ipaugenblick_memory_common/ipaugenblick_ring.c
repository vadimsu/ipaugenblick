#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <sys/errno.h>
#include "ipaugenblick_ring.h"

#define POWEROF2(x) ((((x)-1) & (x)) == 0)
#define IPAUGENBLICK_ALIGN_FLOOR(val, align) \
	(typeof(val))((val) & (~((typeof(val))((align) - 1))))
#define IPAUGENBLICK_ALIGN_CEIL(val, align) \
	IPAUGENBLICK_ALIGN_FLOOR(((val) + ((typeof(val)) (align) - 1)), align)
#define IPAUGENBLICK_ALIGN(val, align) IPAUGENBLICK_ALIGN_CEIL(val, align)

/* return the size of memory occupied by a ring */
ssize_t ipaugenblick_ring_get_memsize(unsigned count)
{
	ssize_t sz;

	/* count must be a power of 2 */
	if ((!POWEROF2(count)) || (count > IPAUGENBLICK_RING_SZ_MASK )) {
		printf("Requested size is invalid, must be power of 2, and do not exceed the size limit %u %u\n", 
                                IPAUGENBLICK_RING_SZ_MASK,count);
		return -EINVAL;
	}

	sz = sizeof(struct ipaugenblick_ring) + count * sizeof(void *);
	sz = IPAUGENBLICK_ALIGN(sz, 32/*L1_CACHE_BYTES*/);
	return sz;
}

int ipaugenblick_ring_init(struct ipaugenblick_ring *r, unsigned count,	unsigned flags)
{
	/* init the ring structure */
	memset(r, 0, sizeof(*r));
	r->flags = flags;
	r->prod.watermark = count;
        r->prod.sp_enqueue = !!(flags & RING_F_SP_ENQ);
	r->cons.sc_dequeue = !!(flags & RING_F_SC_DEQ);
	r->prod.size = r->cons.size = count;
	r->prod.mask = r->cons.mask = count-1;
	r->prod.head = r->cons.head = 0;
	r->prod.tail = r->cons.tail = 0;

	return 0;
}

