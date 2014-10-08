/*
 * NET		An implementation of the SOCKET network access protocol.
 *
 * Version:	@(#)socket.c	1.1.93	18/02/95
 *
 * Authors:	Orest Zborowski, <obz@Kodak.COM>
 *		Ross Biro
 *		Fred N. van Kempen, <waltje@uWalt.NL.Mugnet.ORG>
 *
 * Fixes:
 *		Anonymous	:	NOTSOCK/BADF cleanup. Error fix in
 *					shutdown()
 *		Alan Cox	:	verify_area() fixes
 *		Alan Cox	:	Removed DDI
 *		Jonathan Kamens	:	SOCK_DGRAM reconnect bug
 *		Alan Cox	:	Moved a load of checks to the very
 *					top level.
 *		Alan Cox	:	Move address structures to/from user
 *					mode above the protocol layers.
 *		Rob Janssen	:	Allow 0 length sends.
 *		Alan Cox	:	Asynchronous I/O support (cribbed from the
 *					tty drivers).
 *		Niibe Yutaka	:	Asynchronous I/O for writes (4.4BSD style)
 *		Jeff Uphoff	:	Made max number of sockets command-line
 *					configurable.
 *		Matti Aarnio	:	Made the number of sockets dynamic,
 *					to be allocated when needed, and mr.
 *					Uphoff's max is used as max to be
 *					allowed to allocate.
 *		Linus		:	Argh. removed all the socket allocation
 *					altogether: it's in the inode now.
 *		Alan Cox	:	Made sock_alloc()/sock_release() public
 *					for NetROM and future kernel nfsd type
 *					stuff.
 *		Alan Cox	:	sendmsg/recvmsg basics.
 *		Tom Dyas	:	Export net symbols.
 *		Marcin Dalecki	:	Fixed problems with CONFIG_NET="n".
 *		Alan Cox	:	Added thread locking to sys_* calls
 *					for sockets. May have errors at the
 *					moment.
 *		Kevin Buhr	:	Fixed the dumb errors in the above.
 *		Andi Kleen	:	Some small cleanups, optimizations,
 *					and fixed a copy_from_user() bug.
 *		Tigran Aivazian	:	sys_send(args) calls sys_sendto(args, NULL, 0)
 *		Tigran Aivazian	:	Made listen(2) backlog sanity checks
 *					protocol-independent
 *
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 *
 *	This module is effectively the top level interface to the BSD socket
 *	paradigm.
 *
 *	Based upon Swansea University Computer Society NET3.039
 */
#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/slab.h>
//#include <linux/mm.h>
#include <specific_includes/linux/socket.h>
//#include <linux/file.h>
#include <specific_includes/linux/net.h>
//#include <linux/interrupt.h>
//#include <linux/thread_info.h>
//#include <linux/rcupdate.h>
#include <specific_includes/linux/netdevice.h>
//#include <linux/proc_fs.h>
//#include <linux/seq_file.h>
//#include <linux/mutex.h>
#include <specific_includes/linux/if_bridge.h>
//#include <linux/if_frad.h>
#include <specific_includes/linux/if_vlan.h>
//#include <linux/init.h>
//#include <linux/poll.h>
//#include <linux/cache.h>
//#include <linux/module.h>
//#include <linux/highmem.h>
//#include <linux/mount.h>
//#include <linux/security.h>
//#include <linux/syscalls.h>
//#include <linux/compat.h>
//#include <linux/kmod.h>
//#include <linux/audit.h>
//#include <linux/wireless.h>
//#include <linux/nsproxy.h>
//#include <linux/magic.h>
//#include <linux/xattr.h>

//#include <asm/uaccess.h>
//#include <asm/unistd.h>

//#include <net/compat.h>
//#include <net/wext.h>
//#include <net/cls_cgroup.h>

#include <specific_includes/net/sock.h>
#include <specific_includes/linux/netfilter.h>

//#include <linux/if_tun.h>
//#include <linux/ipv6_route.h>
#include <specific_includes/linux/route.h>
#include <specific_includes/linux/sockios.h>
//#include <linux/atalk.h>
#include <specific_includes/net/busy_poll.h>

#ifdef CONFIG_NET_RX_BUSY_POLL
unsigned int sysctl_net_busy_read __read_mostly;
unsigned int sysctl_net_busy_poll __read_mostly;
#endif
static const struct net_proto_family __rcu *net_families[NPROTO] __read_mostly;

void __sock_recv_timestamp(struct msghdr *msg, struct sock *sk,
	struct sk_buff *skb)
{
	int need_software_tstamp = sock_flag(sk, SOCK_RCVTSTAMP);
	struct timespec ts[3];
	int empty = 1;
	struct skb_shared_hwtstamps *shhwtstamps =
		skb_hwtstamps(skb);

	/* Race occurred between timestamp enabling and packet
	   receiving.  Fill in the current time for now. */
	if (need_software_tstamp && skb->tstamp.tv64 == 0)
		__net_timestamp(skb);

	if (need_software_tstamp) {
		if (!sock_flag(sk, SOCK_RCVTSTAMPNS)) {
			struct timeval tv;
			skb_get_timestamp(skb, &tv);
			put_cmsg(msg, SOL_SOCKET, SCM_TIMESTAMP,
				 sizeof(tv), &tv);
		} else {
			skb_get_timestampns(skb, &ts[0]);
			put_cmsg(msg, SOL_SOCKET, SCM_TIMESTAMPNS,
				 sizeof(ts[0]), &ts[0]);
		}
	}


	memset(ts, 0, sizeof(ts));
	if (sock_flag(sk, SOCK_TIMESTAMPING_SOFTWARE) &&
	    ktime_to_timespec_cond(skb->tstamp, ts + 0))
		empty = 0;
	if (shhwtstamps) {
		if (sock_flag(sk, SOCK_TIMESTAMPING_SYS_HARDWARE) &&
		    ktime_to_timespec_cond(shhwtstamps->syststamp, ts + 1))
			empty = 0;
		if (sock_flag(sk, SOCK_TIMESTAMPING_RAW_HARDWARE) &&
		    ktime_to_timespec_cond(shhwtstamps->hwtstamp, ts + 2))
			empty = 0;
	}
	if (!empty)
		put_cmsg(msg, SOL_SOCKET,
			 SCM_TIMESTAMPING, sizeof(ts), &ts);
}
void sock_tx_timestamp(struct sock *sk, __u8 *tx_flags)
{
#if 0
	*tx_flags = 0;
	if (sock_flag(sk, SOCK_TIMESTAMPING_TX_HARDWARE))
		*tx_flags |= SKBTX_HW_TSTAMP;
	if (sock_flag(sk, SOCK_TIMESTAMPING_TX_SOFTWARE))
		*tx_flags |= SKBTX_SW_TSTAMP;
	if (sock_flag(sk, SOCK_WIFI_STATUS))
		*tx_flags |= SKBTX_WIFI_STATUS;
#endif
}
EXPORT_SYMBOL(sock_tx_timestamp);
/**
 *	sock_release	-	close a socket
 *	@sock: socket to close
 *
 *	The socket is released from the protocol stack if it has a release
 *	callback, and the inode is then released if the socket is bound to
 *	an inode not a file.
 */

void sock_release(struct socket *sock)
{
#if 0
	if (sock->ops) {
		struct module *owner = sock->ops->owner;

		sock->ops->release(sock);
		sock->ops = NULL;
		module_put(owner);
	}

//	if (rcu_dereference_protected(sock->wq, 1)->fasync_list)
//		printk(KERN_ERR "sock_release: fasync list not empty!\n");

	if (test_bit(SOCK_EXTERNALLY_ALLOCATED, &sock->flags))
		return;

	this_cpu_sub(sockets_in_use, 1);
	if (!sock->file) {
		iput(SOCK_INODE(sock));
		return;
	}
	sock->file = NULL;
#endif
}
EXPORT_SYMBOL(sock_release);
int sock_wake_async(struct socket *sock, int how, int band)
{
         if (!sock)
                 return -1;
	 switch(band)
	 {
	 case POLL_OUT:
		//app_glue_on_transmission_opportunity(sock);
		break;
	 case POLL_IN:
		//app_glue_data_available_cbk(sock);
		break;
	 case POLL_ERR:
		//app_glue_on_error(sock);
		break;
	 case POLL_HUP:
		 //app_glue_on_hup(sock);
	 	break;
	 }
         return 0;
}
 EXPORT_SYMBOL(sock_wake_async);
/**
 *	sock_alloc	-	allocate a socket
 *
 *	Allocate a new inode and socket object. The two are bound together
 *	and initialised. The socket is then returned. If we are out of inodes
 *	NULL is returned.
 */

static struct socket *sock_alloc(void)
{
//	struct inode *inode;
	struct socket *sock = rte_zmalloc(NULL,sizeof(struct socket),CACHE_LINE_SIZE);

//	inode = new_inode_pseudo(sock_mnt->mnt_sb);
//	if (!inode)
//		return NULL;

//	sock = SOCKET_I(inode);

//	kmemcheck_annotate_bitfield(sock, type);
//	inode->i_ino = get_next_ino();
//	inode->i_mode = S_IFSOCK | S_IRWXUGO;
//	inode->i_uid = current_fsuid();
//	inode->i_gid = current_fsgid();
//	inode->i_op = &sockfs_inode_ops;

//	this_cpu_add(sockets_in_use, 1);
	return sock;
}
int __sock_create(struct net *net, int family, int type, int protocol,
			 struct socket **res, int kern)
{
	int err;
	struct socket *sock;
	const struct net_proto_family *pf;

	/*
	 *      Check protocol is in range
	 */
	if (family < 0 || family >= NPROTO)
		return -EAFNOSUPPORT;
	if (type < 0 || type >= SOCK_MAX)
		return -EINVAL;

	/* Compatibility.

	   This uglymoron is moved from INET layer to here to avoid
	   deadlock in module load.
	 */
	if (family == PF_INET && type == SOCK_PACKET) {
		static int warned;
		if (!warned) {
			warned = 1;
			printf("uses obsolete (PF_INET,SOCK_PACKET)\n");
		}
		family = PF_PACKET;
	}

	err = security_socket_create(family, type, protocol, kern);
	if (err)
		return err;

	/*
	 *	Allocate the socket and allow the family to set things up. if
	 *	the protocol is 0, the family is instructed to select an appropriate
	 *	default.
	 */
	sock = sock_alloc();
	if (!sock) {
		net_warn_ratelimited("socket: no more sockets\n");
		return -ENFILE;	/* Not exactly a match, but its the
				   closest posix thing */
	}

	sock->type = type;
    memset(&sock->read_queue_entry,0,sizeof(sock->read_queue_entry));
    memset(&sock->write_queue_entry,0,sizeof(sock->write_queue_entry));
    sock->read_queue_present = 0;
    sock->closed_queue_present = 0;
    sock->accept_queue_present = 0;
    sock->write_queue_present = 0;
#ifdef CONFIG_MODULES
	/* Attempt to load a protocol module if the find failed.
	 *
	 * 12/09/1996 Marcin: But! this makes REALLY only sense, if the user
	 * requested real, full-featured networking support upon configuration.
	 * Otherwise module support will break!
	 */
	if (rcu_access_pointer(net_families[family]) == NULL)
		request_module("net-pf-%d", family);
#endif
	rcu_read_lock();
	pf = rcu_dereference(net_families[family]);
	err = -EAFNOSUPPORT;
	if (!pf)
		goto out_release;

	/*
	 * We will call the ->create function, that possibly is in a loadable
	 * module, so we have to bump that loadable module refcnt first.
	 */
//	if (!try_module_get(pf->owner))
//		goto out_release;

	/* Now protected by module ref count */
	rcu_read_unlock();

	err = pf->create(net, sock, protocol, kern);
	if (err < 0)
		goto out_module_put;
	/*
	 * Now to bump the refcnt of the [loadable] module that owns this
	 * socket at sock_release time we decrement its refcnt.
	 */
//	if (!try_module_get(sock->ops->owner))
//		goto out_module_busy;

	/*
	 * Now that we're done with the ->create function, the [loadable]
	 * module can have its refcnt decremented
	 */
//	module_put(pf->owner);
	err = security_socket_post_create(sock, family, type, protocol, kern);
	if (err)
		goto out_sock_release;
	*res = sock;

	return 0;

out_module_busy:
	err = -EAFNOSUPPORT;
out_module_put:
	sock->ops = NULL;
//	module_put(pf->owner);
out_sock_release:
	sock_release(sock);
	return err;

out_release:
	rcu_read_unlock();
	goto out_sock_release;
}
EXPORT_SYMBOL(__sock_create);
int sock_create_kern(int family, int type, int protocol, struct socket **res)
{
	return __sock_create(&init_net, family, type, protocol, res, 1);
}
EXPORT_SYMBOL(sock_create_kern);

/**
 *	sock_register - add a socket protocol handler
 *	@ops: description of protocol
 *
 *	This function is called by a protocol handler that wants to
 *	advertise its address family, and have it linked into the
 *	socket interface. The value ops->family coresponds to the
 *	socket system call protocol family.
 */
int sock_register(const struct net_proto_family *ops)
{
	int err;

	if (ops->family >= NPROTO) {
		printf("protocol %d >= NPROTO(%d)\n", ops->family,
		       NPROTO);
		return -ENOBUFS;
	}

	spin_lock(&net_family_lock);
	if (rcu_dereference_protected(net_families[ops->family],
				      lockdep_is_held(&net_family_lock)))
		err = -EEXIST;
	else {
		rcu_assign_pointer(net_families[ops->family], ops);
		err = 0;
	}
	spin_unlock(&net_family_lock);

	printf("NET: Registered protocol family %d\n", ops->family);
	return err;
}
EXPORT_SYMBOL(sock_register);
static inline void sock_recv_drops(struct msghdr *msg, struct sock *sk,
				   struct sk_buff *skb)
{
	if (sock_flag(sk, SOCK_RXQ_OVFL) && skb && skb->dropcount)
		put_cmsg(msg, SOL_SOCKET, SO_RXQ_OVFL,
			sizeof(__u32), &skb->dropcount);
}
/*
 * called from sock_recv_timestamp() if sock_flag(sk, SOCK_RCVTSTAMP)
 */
void __sock_recv_ts_and_drops(struct msghdr *msg, struct sock *sk,
	struct sk_buff *skb)
{
	sock_recv_timestamp(msg, sk, skb);
	sock_recv_drops(msg, sk, skb);
}
EXPORT_SYMBOL_GPL(__sock_recv_ts_and_drops);
void __sock_recv_wifi_status(struct msghdr *msg, struct sock *sk,
	struct sk_buff *skb)
{
	int ack;

	if (!sock_flag(sk, SOCK_WIFI_STATUS))
		return;
	if (!skb->wifi_acked_valid)
		return;

	ack = skb->wifi_acked;

	put_cmsg(msg, SOL_SOCKET, SCM_WIFI_STATUS, sizeof(ack), &ack);
}
EXPORT_SYMBOL_GPL(__sock_recv_wifi_status);
int sock_create_lite(int family, int type, int protocol, struct socket **res)
{
         int err;
         struct socket *sock = NULL;
 
         err = security_socket_create(family, type, protocol, 1);
         if (err)
                 goto out;
 
         sock = sock_alloc();
         if (!sock) {
                 err = -ENOMEM;
                 goto out;
         }
 
         sock->type = type;
         err = security_socket_post_create(sock, family, type, protocol, 1);
         if (err)
                 goto out_release;
 
out:
         *res = sock;
         return err;
out_release:
         sock_release(sock);
         sock = NULL;
         goto out;
}
EXPORT_SYMBOL(sock_create_lite);
int kernel_accept(struct socket *sock, struct socket **newsock, int flags)
{
         struct sock *sk = sock->sk;
         int err;

         err = sock_create_lite(sk->sk_family, sk->sk_type, sk->sk_protocol,
                                newsock);
         if (err < 0)
                 goto done;
 
         err = sock->ops->accept(sock, *newsock, flags);
         if (err < 0) {
                 sock_release(*newsock);
                 *newsock = NULL;
                 goto done;
         }
 
         (*newsock)->ops = sock->ops;
//         __module_get((*newsock)->ops->owner);
 
done:
         return err;
}
EXPORT_SYMBOL(kernel_accept);
int kernel_listen(struct socket *sock, int backlog)
{
	 int rc;
     rc = sock->ops->listen(sock, backlog);
	 return rc;
}
EXPORT_SYMBOL(kernel_listen);
int kernel_connect(struct socket *sock, struct sockaddr *addr, int addrlen,
                    int flags)
{
	 int rc;
	 rc = sock->ops->connect(sock, addr, addrlen, flags);
	 return rc;
}
EXPORT_SYMBOL(kernel_connect);
int kernel_bind(struct socket *sock, struct sockaddr *addr, int addrlen)
{
	 int rc;
	 rc = sock->ops->bind(sock, addr, addrlen);
 	 return rc;
}
EXPORT_SYMBOL(kernel_bind); 
int kernel_sendpage(struct socket *sock, struct page *page, int offset,
                     size_t size, int flags)
{
         if (likely(sock->ops->sendpage))
                 return sock->ops->sendpage(sock, page, offset, size, flags);
 
         return sock_no_sendpage(sock, page, offset, size, flags);
}
int kernel_sendmsg(struct socket *sock, struct msghdr *msg,size_t size)
{
	if (likely(sock->ops->sendmsg))
		return sock->ops->sendmsg(NULL,sock, msg, size);
	//return sock_no_sendmsg(sock, page, offset, size, flags);
	return -1;
}
static inline int __sock_recvmsg_nosec(struct kiocb *iocb, struct socket *sock,
                                       struct msghdr *msg, size_t size, int flags)
{
        struct sock_iocb *si = kiocb_to_siocb(iocb);

        si->sock = sock;
        si->scm = NULL;
        si->msg = msg;
        si->size = size;
        si->flags = flags;

        return sock->ops->recvmsg(iocb, sock, msg, size, flags);
}

static inline int __sock_recvmsg(struct kiocb *iocb, struct socket *sock,
                                 struct msghdr *msg, size_t size, int flags)
{
        //int err = security_socket_recvmsg(sock, msg, size, flags);

        return /*err ?:*/ __sock_recvmsg_nosec(iocb, sock, msg, size, flags);
}

int sock_recvmsg(struct socket *sock, struct msghdr *msg,
                 size_t size, int flags)
{
        struct kiocb iocb;
        struct sock_iocb siocb;
        int ret;

       // init_sync_kiocb(&iocb, NULL);
        iocb.private = &siocb;
        ret = __sock_recvmsg(&iocb, sock, msg, size, flags);
//        if (-EIOCBQUEUED == ret)
  //              ret = wait_on_sync_kiocb(&iocb);
        return ret;
}
EXPORT_SYMBOL(sock_recvmsg);
int kernel_recvmsg(struct socket *sock, struct msghdr *msg,
                   struct iovec *vec, size_t num, size_t size, int flags)
{
//         mm_segment_t oldfs = get_fs();
         int result;
#if 0
  //      set_fs(KERNEL_DS);
        /*
         * the following is safe, since for compiler definitions of kvec and
         * iovec are identical, yielding the same in-core layout and alignment
         */
        msg->msg_iov = (struct iovec *)vec, msg->msg_iovlen = num;
        result = sock_recvmsg(sock, msg, size, flags);
        //set_fs(oldfs);
        return result;
#else
        msg->msg_iov = (struct iovec *)vec, msg->msg_iovlen = num;
        if (likely(sock->ops->recvmsg))
            return sock->ops->recvmsg(NULL,sock, msg, size, flags);
#endif
}
EXPORT_SYMBOL(kernel_recvmsg);

void kernel_close(struct socket *sock)
{
	struct sock *sk = sock->sk;
	if(sk) {
	    sk->sk_prot->close(sk,0);
	}
}

int kernel_getsockname(struct socket *sock, struct sockaddr *addr,
                          int *addrlen)
{
         return sock->ops->getname(sock, addr, addrlen, 0);
}

 
int kernel_getpeername(struct socket *sock, struct sockaddr *addr,
                         int *addrlen)
{
         return sock->ops->getname(sock, addr, addrlen, 1);
}
