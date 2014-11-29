/*
 * mm_porting.c
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains functions wrappers for Linux kernel memory management functions
 *  to port the Linux TCP/IP to userland with integration with DPDK 1.6
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/log2.h>
#include <specific_includes/linux/skbuff.h>
#include <rte_config.h>
#include <rte_common.h>
#include <rte_memcpy.h>
#include <rte_lcore.h>
#include <rte_mempool.h>
#include <rte_cycles.h>
#include <rte_hash.h>
#include <rte_byteorder.h>

void vfree(void *mem)
{
    rte_free(mem);
}

void *vzalloc(int size)
{
    return rte_zmalloc(NULL,size,CACHE_LINE_SIZE);
}

void *kmemdup(void *addr, int size, int dummy)
{
    void *mem = rte_malloc(NULL,size,CACHE_LINE_SIZE);

    if(mem == NULL) {
        return NULL;
    }
    memcpy(mem,addr,size);
    return mem;
}

void *__kmalloc(size_t size, gfp_t flags)
{
    if(flags & ___GFP_ZERO) {
        return rte_zmalloc(NULL,size,CACHE_LINE_SIZE);
    }
    return rte_malloc(NULL,size,CACHE_LINE_SIZE);
}
void * __must_check __krealloc(void *mem, size_t size, gfp_t flags)
{
    return rte_realloc(mem,size,CACHE_LINE_SIZE);
}
void * __must_check krealloc(void *mem, size_t size, gfp_t flags)
{
    return __krealloc(mem,size,flags);
}
void kfree(void *mem)
{
    rte_free(mem);
}
void kzfree(void *mem)
{
    rte_free(mem);
}
size_t ksize(const void *mem)
{
    return 0;
}
static void kmem_cache_custom_init(struct rte_mempool *mp, void *opaque_arg,
		      void *m, unsigned i)
{
	void (*init_cbk)(void *) = opaque_arg;

	if(init_cbk) {
		init_cbk(m);
	}
}

struct kmem_cache *kmem_cache_create(const char *name, size_t element_size, size_t count,
			unsigned long flag,
			void (*init_cbk)(void *))
{
    return rte_mempool_create(name,count,element_size,0,0,NULL,NULL,kmem_cache_custom_init,(void *)init_cbk,rte_socket_id(),
    		/*MEMPOOL_F_SP_PUT | MEMPOOL_F_SC_GET*/0);
}

void kmem_cache_destroy(struct kmem_cache *cache)
{
	printf("NOT IMPLEMENTED %s %d\n",__FILE__,__LINE__);
}

unsigned long __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    return 0;
}

void kmemleak_alloc(void *ret, int size, int dummy, gfp_t flags)
{
}

unsigned long nr_free_buffer_pages(void)
{
    return 1024*8*MAX_PKT_BURST;
}

/*
 * allocate a large system hash table from bootmem
 * - it is assumed that the hash table must contain an exact power-of-2
 *   quantity of entries
 * - limit is the number of hash buckets, not the total allocation size
 */
void *__init alloc_large_system_hash(const char *tablename,
				     unsigned long bucketsize,
				     unsigned long numentries,
				     int scale,
				     int flags,
				     unsigned int *_hash_shift,
				     unsigned int *_hash_mask,
				     unsigned long low_limit,
				     unsigned long high_limit)
{
    unsigned long log2qty,size;
    void *table = NULL;

    if(numentries == 0) {
    	numentries = 1 << 16;
    }

    size = numentries*bucketsize;
 
    table = rte_zmalloc(NULL,size,CACHE_LINE_SIZE);
 
    if (!table)
           panic("Failed to allocate hash table\n");
 
    printf("%s hash table entries: %ld (order: %d, %lu bytes)\n",
           tablename,
           (1UL << log2qty),
           ilog2(size) - PAGE_SHIFT,
           size);
    log2qty = ilog2(numentries);
    if (_hash_shift)
            *_hash_shift = log2qty;
    if (_hash_mask)
            *_hash_mask = (1 << log2qty) - 1;
 
    return table;
}
/*
 *      And now for the all-in-one: copy and checksum from a user iovec
 *      directly to a datagram
 *      Calls to csum_partial but the last must be in 32 bit chunks
 *
 *      ip_build_xmit must ensure that when fragmenting only the last
 *      call to this function will be unaligned also.
 */
int csum_partial_copy_fromiovecend(struct sk_buff *skb, struct iovec *iov,
                                 int offset, unsigned int len, __wsum *csump)
{
	__wsum csum = *csump;
	struct rte_mbuf *current = iov->head;
	while(current) {
		struct rte_mbuf *mbuf = current;
		if(offset < mbuf->pkt.data_len) {
			struct page page;
			current = current->pkt.next;
			page.mbuf = mbuf;
			*csump = csum_partial(((char *)mbuf->pkt.data)+offset,mbuf->pkt.data_len - offset, csum);
			skb_fill_page_desc(skb,skb_shinfo(skb)->nr_frags,&page,offset,mbuf->pkt.data_len - offset);
			skb->len += mbuf->pkt.data_len - offset;
		    skb->data_len += mbuf->pkt.data_len - offset;
			offset = 0;
		}
		else {
			offset -= mbuf->pkt.data_len;
			current = current->pkt.next;
		}
	}
    *csump = csum;
    return 0;
}
int memcpy_fromiovecend(struct sk_buff *skb, const struct iovec *iov,
			       int offset, int len)
{
	struct rte_mbuf *current = iov->head;
	while(current) {
		struct rte_mbuf *mbuf = current;
		if(offset < mbuf->pkt.data_len) {
			struct page page;
			current = current->pkt.next;
			page.mbuf = mbuf;
			skb_fill_page_desc(skb,skb_shinfo(skb)->nr_frags,&page,offset,mbuf->pkt.data_len - offset);
			skb->len += mbuf->pkt.data_len - offset;
			skb->data_len += mbuf->pkt.data_len - offset;
			offset = 0;
		}
		else {
			offset -= mbuf->pkt.data_len;
			current = current->pkt.next;
		}
	}
	return 0;
}
int memcpy_fromiovecend2(unsigned char *kdata, const struct iovec *iov,
			       int offset, int len)
{
	int copied = 0;
	struct rte_mbuf *current = iov->head;
	while((current)&&(copied < len)) {
		struct rte_mbuf *mbuf = current;
		if(offset < mbuf->pkt.data_len) {
			char *data = (char *)mbuf->pkt.data;
			current = current->pkt.next;
			rte_memcpy(kdata+copied,data+offset,mbuf->pkt.data_len - offset);
			copied += mbuf->pkt.data_len - offset;
			offset = 0;
		}
		else {
			offset -= mbuf->pkt.data_len;
			current = current->pkt.next;
		}
	}
    return (copied == len);
}
int memcpy_fromiovec(struct sk_buff *skb, struct iovec *iov, int len)
{
    return memcpy_fromiovecend(skb,iov,0,len);
}
int memcpy_fromiovec2(unsigned char *kdata, struct iovec *iov, int len)
{
    return memcpy_fromiovecend2(kdata,iov,0,len);
}
void free_pages(unsigned long addr, unsigned int order)
{
}

unsigned long get_zeroed_page(gfp_t gfp_mask)
{
    return 0;
}

void release_pages(struct page **pages, int nr, int cold)
{
}
int get_user_pages_fast(unsigned long start, int nr_pages, int write,
			struct page **pages)
{
    return 0;
}

bool gfp_pfmemalloc_allowed(gfp_t gfp_mask)
{
    return false;
}
int copy_from_user(void *dst,void *src,int size)
{
    rte_memcpy(dst,src,size); 
    return 0;
}
int copy_to_user(void *dst,void *src,int size)
{
    rte_memcpy(dst,src,size);
    return 0;
}

int memcpy_toiovec(struct iovec *iov,struct rte_mbuf *mbuf,int offset,int len)
{
	if(rte_pktmbuf_adj(mbuf,offset) == NULL) {
		printf("CANNOT ADJUST MBUF %s %d %d %d %d\n",__FILE__,__LINE__,offset,len,mbuf->pkt.data_len);
		return -1;
	}
	rte_mbuf_refcnt_update(mbuf,1);
	if(iov->head == NULL) {
		iov->head = mbuf;
		iov->tail = mbuf;
	}
	else if(iov->tail != mbuf) { /* check the case partial data is submitted */
		iov->tail->pkt.next = mbuf;
		iov->tail = iov->tail->pkt.next;
	}
	iov->len += len;
	return 0;
}

/*
 * pktmbuf constructor, given as a callback function to
 * rte_mempool_create().
 * Set the fields of a packet mbuf to their default values.
 */
void rte_custom_pktmbuf_init(struct rte_mempool *mp,
		 void *opaque_arg,
		 void *_m,
		 __attribute__((unused)) unsigned i)
{
	struct rte_mbuf *m = _m;
	uint32_t buf_len = mp->elt_size - sizeof(struct rte_mbuf);
        struct rte_mempoll *bufmempool = (struct rte_mempool *)opaque_arg;
        void *raw_data_buf = NULL;
        int rc;

        RTE_MBUF_ASSERT(bufmempool);

	RTE_MBUF_ASSERT(mp->elt_size >= sizeof(struct rte_mbuf));
        rc = rte_mempool_get(bufmempool, &raw_data_buf);
        RTE_MBUF_ASSERT(rc == 0);
        RTE_MBUF_ASSERT(raw_data_buf);

	memset(m, 0, mp->elt_size);
        memset(raw_data_buf,0,MBUF_SIZE);

	/* start of buffer is just after mbuf structure */
	m->buf_addr = raw_data_buf;
	m->buf_physaddr = rte_mempool_virt2phy(bufmempool, raw_data_buf);
	m->buf_len = (uint16_t)MBUF_SIZE;

	/* keep some headroom between start of buffer and data */
	m->pkt.data = (char*) m->buf_addr + RTE_MIN(RTE_PKTMBUF_HEADROOM, m->buf_len);

	/* init some constant fields */
	m->type = RTE_MBUF_PKT;
	m->pool = mp;
	m->pkt.nb_segs = 1;
	m->pkt.in_port = 0xff;
}
