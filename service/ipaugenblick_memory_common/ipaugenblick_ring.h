
#ifndef __IPAUGENBLICK_RING_H__
#define __IPAUGENBLICK_RING_H__
#define IPAUGENBLICK_FORCE_INTRINSICS
#ifndef __KERNEL__
#define uint8_t unsigned char
#define uint32_t unsigned int
#endif

#define __ipaugenblick_cache_aligned

#ifdef IPAUGENBLICK_LIBRTE_RING_DEBUG
#else
#define __RING_STAT_ADD(r, name, n) do {} while(0)
#endif

#define MPLOCKED        "lock ; "       /**< Insert MP lock prefix. */

#define	ipaugenblick_compiler_barrier() do {		\
	asm volatile ("" : : : "memory");	\
} while(0)
#ifndef __KERNEL__
#define likely(x)	__builtin_expect((x), 1)
#define unlikely(x)	__builtin_expect((x), 0)
#endif
/**
 * Atomic compare and set.
 *
 * (atomic) equivalent to:
 *   if (*dst == exp)
 *     *dst = src (all 32-bit words)
 *
 * @param dst
 *   The destination location into which the value will be written.
 * @param exp
 *   The expected value.
 * @param src
 *   The new value.
 * @return
 *   Non-zero on success; 0 on failure.
 */
static inline int ipaugenblick_atomic32_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src)
{
#ifndef IPAUGENBLICK_FORCE_INTRINSICS
	uint8_t res;

	asm volatile(
			MPLOCKED
			"cmpxchgl %[src], %[dst];"
			"sete %[res];"
			: [res] "=a" (res),     /* output */
			  [dst] "=m" (*dst)
			: [src] "r" (src),      /* input */
			  "a" (exp),
			  "m" (*dst)
			: "memory");            /* no-clobber list */
	return res;
#else
	return __sync_bool_compare_and_swap(dst, exp, src);
#endif
}

static inline void ipaugenblick_pause(void) {}
#define RING_F_SP_ENQ 0x0001 /**< The default enqueue is "single-producer". */
#define RING_F_SC_DEQ 0x0002 /**< The default dequeue is "single-consumer". */
#define IPAUGENBLICK_RING_QUOT_EXCEED (1 << 31)  /**< Quota exceed for burst ops */
#define IPAUGENBLICK_RING_SZ_MASK  (unsigned)(0x0fffffff) /**< Ring size mask */

enum ipaugenblick_ring_queue_behavior {
	IPAUGENBLICK_RING_QUEUE_FIXED = 0, /* Enq/Deq a fixed number of items from a ring */
	IPAUGENBLICK_RING_QUEUE_VARIABLE   /* Enq/Deq as many items a possible from ring */
};

struct ipaugenblick_ring {
        int flags;                       /**< Flags supplied at creation. */
	/** Ring producer status. */
	struct ipaugenblick_prod {
                uint32_t watermark;      /**< Maximum items before EDQUOT. */
                uint32_t sp_enqueue;     /**< True, if single producer. */
		uint32_t size;           /**< Size of ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Producer head. */
		volatile uint32_t tail;  /**< Producer tail. */
	} prod __ipaugenblick_cache_aligned;

	/** Ring consumer status. */
	struct ipaugenblick_cons {
                uint32_t sc_dequeue;     /**< True, if single consumer. */
                uint32_t size;           /**< Size of the ring. */
		uint32_t mask;           /**< Mask (size-1) of ring. */
		volatile uint32_t head;  /**< Consumer head. */
		volatile uint32_t tail;  /**< Consumer tail. */
	} cons __ipaugenblick_cache_aligned;

	void * ring[0] __ipaugenblick_cache_aligned; /**< Memory space of ring starts here.
	                                     * not volatile so need to be careful
	                                     * about compiler re-ordering */
}__attribute__((packed));

/* the actual enqueue of pointers on the ring.
 * Placed here since identical code needed in both
 * single and multi producer enqueue functions */
#define ENQUEUE_PTRS() do { \
	const uint32_t size = r->prod.size; \
	uint32_t idx = prod_head & mask; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) { \
			r->ring[idx] = obj_table[i]; \
			r->ring[idx+1] = obj_table[i+1]; \
			r->ring[idx+2] = obj_table[i+2]; \
			r->ring[idx+3] = obj_table[i+3]; \
		} \
		switch (n & 0x3) { \
			case 3: r->ring[idx++] = obj_table[i++]; \
			case 2: r->ring[idx++] = obj_table[i++]; \
			case 1: r->ring[idx++] = obj_table[i++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++)\
			r->ring[idx] = obj_table[i]; \
		for (idx = 0; i < n; i++, idx++) \
			r->ring[idx] = obj_table[i]; \
	} \
} while(0)

/* the actual copy of pointers on the ring to obj_table.
 * Placed here since identical code needed in both
 * single and multi consumer dequeue functions */
#define DEQUEUE_PTRS() do { \
	uint32_t idx = cons_head & mask; \
	const uint32_t size = r->cons.size; \
	if (likely(idx + n < size)) { \
		for (i = 0; i < (n & (~(unsigned)0x3)); i+=4, idx+=4) {\
			obj_table[i] = r->ring[idx]; \
			obj_table[i+1] = r->ring[idx+1]; \
			obj_table[i+2] = r->ring[idx+2]; \
			obj_table[i+3] = r->ring[idx+3]; \
		} \
		switch (n & 0x3) { \
			case 3: obj_table[i++] = r->ring[idx++]; \
			case 2: obj_table[i++] = r->ring[idx++]; \
			case 1: obj_table[i++] = r->ring[idx++]; \
		} \
	} else { \
		for (i = 0; idx < size; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
		for (idx = 0; i < n; i++, idx++) \
			obj_table[i] = r->ring[idx]; \
	} \
} while (0)

/**
 * @internal Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   IPAUGENBLICK_RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   IPAUGENBLICK_RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = IPAUGENBLICK_RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = IPAUGENBLICK_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __ipaugenblick_ring_mp_enqueue(struct ipaugenblick_ring *r, void * const *obj_table,
			 unsigned n, enum ipaugenblick_ring_queue_behavior behavior)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail, free_entries;
	const unsigned max = n;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	/* move prod.head atomically */
	do {
		/* Reset n to the initial burst count */
		n = max;

		prod_head = r->prod.head;
		cons_tail = r->cons.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * prod_head > cons_tail). So 'free_entries' is always between 0
		 * and size(ring)-1. */
		free_entries = (mask + cons_tail - prod_head);

		/* check that we have enough room in ring */
		if (unlikely(n > free_entries)) {
			if (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) {
				__RING_STAT_ADD(r, enq_fail, n);
				return -ENOBUFS;
			}
			else {
				/* No free entry available */
				if (unlikely(free_entries == 0)) {
					__RING_STAT_ADD(r, enq_fail, n);
					return 0;
				}

				n = free_entries;
			}
		}

		prod_next = prod_head + n;
		success = ipaugenblick_atomic32_cmpset(&r->prod.head, prod_head,
					      prod_next);
	} while (unlikely(success == 0));

	/* write entries in ring */
	ENQUEUE_PTRS();
	ipaugenblick_compiler_barrier();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) ? -EDQUOT :
				(int)(n | IPAUGENBLICK_RING_QUOT_EXCEED);
		__RING_STAT_ADD(r, enq_quota, n);
	}
	else {
		ret = (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) ? 0 : n;
		__RING_STAT_ADD(r, enq_success, n);
	}

	/*
	 * If there are other enqueues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->prod.tail != prod_head))
		ipaugenblick_pause();

	r->prod.tail = prod_next;
	return ret;
}
/**
 * @internal Enqueue several objects on a ring (NOT multi-producers safe).
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @param behavior
 *   IPAUGENBLICK_RING_QUEUE_FIXED:    Enqueue a fixed number of items from a ring
 *   IPAUGENBLICK_RING_QUEUE_VARIABLE: Enqueue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = IPAUGENBLICK_RING_QUEUE_FIXED
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 *   if behavior = IPAUGENBLICK_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects enqueued.
 */
static inline int __ipaugenblick_ring_sp_enqueue(struct ipaugenblick_ring *r, void * const *obj_table,
			 unsigned n, enum ipaugenblick_ring_queue_behavior behavior)
{
	uint32_t prod_head, cons_tail;
	uint32_t prod_next, free_entries;
	unsigned i;
	uint32_t mask = r->prod.mask;
	int ret;

	prod_head = r->prod.head;
	cons_tail = r->cons.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * prod_head > cons_tail). So 'free_entries' is always between 0
	 * and size(ring)-1. */
	free_entries = mask + cons_tail - prod_head;

	/* check that we have enough room in ring */
	if (unlikely(n > free_entries)) {
		if (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) {
			__RING_STAT_ADD(r, enq_fail, n);
			return -ENOBUFS;
		}
		else {
			/* No free entry available */
			if (unlikely(free_entries == 0)) {
				__RING_STAT_ADD(r, enq_fail, n);
				return 0;
			}

			n = free_entries;
		}
	}

	prod_next = prod_head + n;
	r->prod.head = prod_next;

	/* write entries in ring */
	ENQUEUE_PTRS();
	ipaugenblick_compiler_barrier();

	/* if we exceed the watermark */
	if (unlikely(((mask + 1) - free_entries + n) > r->prod.watermark)) {
		ret = (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) ? -EDQUOT :
			(int)(n | IPAUGENBLICK_RING_QUOT_EXCEED);
		__RING_STAT_ADD(r, enq_quota, n);
	}
	else {
		ret = (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) ? 0 : n;
		__RING_STAT_ADD(r, enq_success, n);
	}

	r->prod.tail = prod_next;
	return ret;
}
/**
 * @internal Dequeue several objects from a ring (multi-consumers safe). When
 * the request objects are more than the available objects, only dequeue the
 * actual number of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   IPAUGENBLICK_RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   IPAUGENBLICK_RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = IPAUGENBLICK_RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = IPAUGENBLICK_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */

static inline int __ipaugenblick_ring_mc_dequeue(struct ipaugenblick_ring *r, void **obj_table,
		 unsigned n, enum ipaugenblick_ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	const unsigned max = n;
	int success;
	unsigned i;
	uint32_t mask = r->prod.mask;

	/* move cons.head atomically */
	do {
		/* Restore n as it may change every loop */
		n = max;

		cons_head = r->cons.head;
		prod_tail = r->prod.tail;
		/* The subtraction is done between two unsigned 32bits value
		 * (the result is always modulo 32 bits even if we have
		 * cons_head > prod_tail). So 'entries' is always between 0
		 * and size(ring)-1. */
		entries = (prod_tail - cons_head);

		/* Set the actual entries for dequeue */
		if (n > entries) {
			if (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) {
				__RING_STAT_ADD(r, deq_fail, n);
				return -ENOENT;
			}
			else {
				if (unlikely(entries == 0)){
					__RING_STAT_ADD(r, deq_fail, n);
					return 0;
				}

				n = entries;
			}
		}

		cons_next = cons_head + n;
		success = ipaugenblick_atomic32_cmpset(&r->cons.head, cons_head,
					      cons_next);
	} while (unlikely(success == 0));

	/* copy in table */
	DEQUEUE_PTRS();
	ipaugenblick_compiler_barrier();

	/*
	 * If there are other dequeues in progress that preceded us,
	 * we need to wait for them to complete
	 */
	while (unlikely(r->cons.tail != cons_head))
		ipaugenblick_pause();

	__RING_STAT_ADD(r, deq_success, n);
	r->cons.tail = cons_next;

	return behavior == IPAUGENBLICK_RING_QUEUE_FIXED ? 0 : n;
}
/**
 * @internal Dequeue several objects from a ring (NOT multi-consumers safe).
 * When the request objects are more than the available objects, only dequeue
 * the actual number of objects
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @param behavior
 *   IPAUGENBLICK_RING_QUEUE_FIXED:    Dequeue a fixed number of items from a ring
 *   IPAUGENBLICK_RING_QUEUE_VARIABLE: Dequeue as many items a possible from ring
 * @return
 *   Depend on the behavior value
 *   if behavior = RTE_RING_QUEUE_FIXED
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 *   if behavior = RTE_RING_QUEUE_VARIABLE
 *   - n: Actual number of objects dequeued.
 */
static inline int __ipaugenblick_ring_sc_dequeue(struct ipaugenblick_ring *r, void **obj_table,
		 unsigned n, enum ipaugenblick_ring_queue_behavior behavior)
{
	uint32_t cons_head, prod_tail;
	uint32_t cons_next, entries;
	unsigned i;
	uint32_t mask = r->prod.mask;

	cons_head = r->cons.head;
	prod_tail = r->prod.tail;
	/* The subtraction is done between two unsigned 32bits value
	 * (the result is always modulo 32 bits even if we have
	 * cons_head > prod_tail). So 'entries' is always between 0
	 * and size(ring)-1. */
	entries = prod_tail - cons_head;

	if (n > entries) {
		if (behavior == IPAUGENBLICK_RING_QUEUE_FIXED) {
			__RING_STAT_ADD(r, deq_fail, n);
			return -ENOENT;
		}
		else {
			if (unlikely(entries == 0)){
				__RING_STAT_ADD(r, deq_fail, n);
				return 0;
			}

			n = entries;
		}
	}

	cons_next = cons_head + n;
	r->cons.head = cons_next;

	/* copy in table */
	DEQUEUE_PTRS();
	ipaugenblick_compiler_barrier();

	__RING_STAT_ADD(r, deq_success, n);
	r->cons.tail = cons_next;
	return behavior == IPAUGENBLICK_RING_QUEUE_FIXED ? 0 : n;
}
/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - 0: Success; objects enqueue.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue, no object is enqueued.
 */
static inline int ipaugenblick_ring_enqueue_bulk(struct ipaugenblick_ring *r, void * const *obj_table,
			 unsigned n)
{
        if (r->prod.sp_enqueue)
	    return __ipaugenblick_ring_sp_enqueue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_FIXED);
	else
	    return __ipaugenblick_ring_mp_enqueue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_FIXED);
}

/**
 * Enqueue one object on a ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj
 *   A pointer to the object to be added.
 * @return
 *   - 0: Success; objects enqueued.
 *   - -EDQUOT: Quota exceeded. The objects have been enqueued, but the
 *     high water mark is exceeded.
 *   - -ENOBUFS: Not enough room in the ring to enqueue; no object is enqueued.
 */
static inline int ipaugenblick_ring_enqueue(struct ipaugenblick_ring *r, void *obj)
{
	return ipaugenblick_ring_enqueue_bulk(r, &obj, 1);
}
/**
 * Enqueue several objects on the ring (multi-producers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * producer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects).
 * @param n
 *   The number of objects to add in the ring from the obj_table.
 * @return
 *   - n: Actual number of objects enqueued.
 */
static inline int ipaugenblick_ring_enqueue_burst(struct ipaugenblick_ring *r, void * const *obj_table,
			 unsigned n)
{
        if (r->prod.sp_enqueue)
	    return __ipaugenblick_ring_sp_enqueue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_VARIABLE);
	else
	    return __ipaugenblick_ring_mp_enqueue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_VARIABLE);
}
/**
 * Dequeue several objects from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int ipaugenblick_ring_dequeue_bulk(struct ipaugenblick_ring *r, void **obj_table, unsigned n)
{
        if (r->cons.sc_dequeue)
	    return __ipaugenblick_ring_sc_dequeue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_FIXED);
	else
	    return __ipaugenblick_ring_mc_dequeue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_FIXED);
}

/**
 * Dequeue one object from a ring (multi-consumers safe).
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_p
 *   A pointer to a void * pointer (object) that will be filled.
 * @return
 *   - 0: Success; objects dequeued.
 *   - -ENOENT: Not enough entries in the ring to dequeue; no object is
 *     dequeued.
 */
static inline int ipaugenblick_ring_dequeue(struct ipaugenblick_ring *r, void **obj_p)
{
	return ipaugenblick_ring_dequeue_bulk(r, obj_p, 1);
}
/**
 * Dequeue several objects from a ring (multi-consumers safe). When the request
 * objects are more than the available objects, only dequeue the actual number
 * of objects
 *
 * This function uses a "compare and set" instruction to move the
 * consumer index atomically.
 *
 * @param r
 *   A pointer to the ring structure.
 * @param obj_table
 *   A pointer to a table of void * pointers (objects) that will be filled.
 * @param n
 *   The number of objects to dequeue from the ring to the obj_table.
 * @return
 *   - n: Actual number of objects dequeued, 0 if ring is empty
 */
static inline int ipaugenblick_ring_dequeue_burst(struct ipaugenblick_ring *r, void **obj_table, unsigned n)
{
        if (r->cons.sc_dequeue)
	    return __ipaugenblick_ring_sc_dequeue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_VARIABLE);
	else
	    return __ipaugenblick_ring_mc_dequeue(r, obj_table, n, IPAUGENBLICK_RING_QUEUE_VARIABLE);
}
/* return the size of memory occupied by a ring */
ssize_t ipaugenblick_ring_get_memsize(unsigned count);

int ipaugenblick_ring_init(struct ipaugenblick_ring *r, unsigned count,	unsigned flags);

#endif
