/*
 * user_callbacks.h
 *
 *  Created on: Jul 6, 2014
 *      Author: Vadim Suraev vadim.suraev@gmail.com
 *  Contains function prototypes for building applications
 *  on the top of Linux TCP/IP ported to userland and integrated with DPDK 1.6
 *  These functions have to be implemented by the application(s)
 */

#ifndef __USER_CALLBACKS_H_
#define __USER_CALLBACKS_H_

extern uint64_t user_on_tx_opportunity_cycles;
extern uint64_t user_on_tx_opportunity_called;
extern uint64_t user_on_tx_opportunity_getbuff_called;
extern uint64_t user_on_tx_opportunity_api_nothing_to_tx;
extern uint64_t user_on_tx_opportunity_api_failed;
extern uint64_t user_on_tx_opportunity_api_mbufs_sent;
extern uint64_t user_on_tx_opportunity_cannot_get_buff;
extern uint64_t user_on_rx_opportunity_called;
extern uint64_t user_on_rx_opportunity_called_exhausted;
extern uint64_t user_rx_mbufs;
extern uint64_t user_on_tx_opportunity_cannot_send;
extern uint64_t user_rx_ring_full; 
extern uint64_t user_kick_socket_rx;
extern uint64_t g_last_time_transmitted;

static inline __attribute__ ((always_inline)) void *get_user_data(void *socket)
{
        struct socket *sock = (struct socket *)socket;
        if(!sock) {
                printf("PANIC: socket NULL %s %d\n",__FILE__,__LINE__);while(1);
        }
        if(!sock->sk) {
                printf("PANIC: socket->sk NULL\n");while(1);
        }
        return sock->sk->sk_user_data;
}

static inline __attribute__ ((always_inline)) int user_on_transmission_opportunity(struct socket *sock)
{
        struct page page;
        int i = 0,sent = 0;
        uint32_t to_send_this_time;
        void *socket_satelite_data;
        uint64_t ring_entries;

        user_on_tx_opportunity_called++;

        if(!sock) {
            return 0;
        }
        socket_satelite_data = get_user_data(sock);

        if(!socket_satelite_data) {
            return 0;
        }
        
        if(sock->type == SOCK_STREAM) {
            ring_entries = ipaugenblick_tx_buf_count(socket_satelite_data);
            if(ring_entries == 0) {
                ipaugenblick_mark_writable(socket_satelite_data);
                user_on_tx_opportunity_api_nothing_to_tx++;
                return 0;
            }
            while(likely((to_send_this_time = app_glue_calc_size_of_data_to_send(sock)) > 0))/*while(1)*/ {
                    sock->sk->sk_route_caps |= NETIF_F_SG | NETIF_F_ALL_CSUM;
                   to_send_this_time = (to_send_this_time > (ring_entries<<10)) ? (ring_entries<<10) : to_send_this_time;
                    i = kernel_sendpage(sock, &page, 0/*offset*/,to_send_this_time, 0 /*flags*/);
                    if(i <= 0) {
                        user_on_tx_opportunity_api_failed++;
                        break;
                    }
                    else
                        sent += i;
            }
            if(i > 0)
                ipaugenblick_mark_writable(socket_satelite_data);
        }
        else if((sock->type == SOCK_DGRAM)||(sock->type == SOCK_RAW)) {
            struct msghdr msghdr;
            struct iovec iov;
            struct rte_mbuf *mbuf[MAX_PKT_BURST];
            struct sock *sk = sock->sk;
            int dequeued,rc = 0,loop = 0;

            msghdr.msg_namelen = sizeof(struct sockaddr_in);
            msghdr.msg_iov = &iov;
            msghdr.msg_iovlen = 1;
            msghdr.msg_controllen = 0;
            msghdr.msg_control = 0;
            msghdr.msg_flags = 0;

          do {
                ring_entries = ipaugenblick_tx_buf_count(socket_satelite_data);
                if((ring_entries < MAX_PKT_BURST)&&((rte_rdtsc() - g_last_time_transmitted) < 30000)) {
                    if(!ring_entries)
                        ipaugenblick_mark_writable(socket_satelite_data);
                    break;
                }
                dequeued = ipaugenblick_dequeue_tx_buf_burst(socket_satelite_data,mbuf,MAX_PKT_BURST);
                for(i = 0;(i < dequeued)&&(rc >= 0);i++) {
                    char *p_addr;
                    if((sock->sk)&&(sock->sk->sk_state != TCP_ESTABLISHED)) {
                        p_addr = (char *)mbuf[i]->pkt.data;
                        p_addr -= sizeof(struct sockaddr_in);
                        msghdr.msg_name = p_addr;
                    }
                    else
                       msghdr.msg_name = NULL;

                    iov.head = mbuf[i]; 
                //    sent = 1;
                    rc = udp_sendmsg(NULL, sk, &msghdr, mbuf[i]->pkt.data_len);
                    sent = (rc > 0);
                    if(sent > 0)
                        g_last_time_transmitted = rte_rdtsc();
                }
                user_on_tx_opportunity_api_failed += dequeued - i;
                for(;i < dequeued;i++) {
                    rte_pktmbuf_free(mbuf[i]);
                }
            }while((dequeued > 0) && (sent > 0));
            if(rc > 0)
                ipaugenblick_mark_writable(socket_satelite_data);
        } 
        return /*sent*/1;
}

static inline __attribute__ ((always_inline)) int user_data_available_cbk(struct socket *sock)
{
    struct msghdr msg;
    struct iovec vec;
    struct rte_mbuf *mbuf;
    int i,ring_free,exhausted = 0;
    void *socket_satelite_data;
    unsigned int ringset_idx;
    struct sockaddr_in sockaddrin;

    user_on_rx_opportunity_called++;
    memset(&vec,0,sizeof(vec));
    if(unlikely(sock == NULL)) {
        return 0;
    }
    socket_satelite_data = get_user_data(sock);
    if(!socket_satelite_data) {
        printf("%s %d\n",__FILE__,__LINE__);
        return 0;
    }
    
    if((sock->type == SOCK_DGRAM)||(sock->type == SOCK_RAW)) {
        msg.msg_namelen = sizeof(sockaddrin);
        msg.msg_name = &sockaddrin;
    }
    ring_free = ipaugenblick_rx_buf_free_count(socket_satelite_data);
    if(!ring_free) {
        ipaugenblick_kick_socket(socket_satelite_data);
        user_rx_ring_full++;
        return 0;
    }
    while(ring_free > 0) {
        if(unlikely((i = kernel_recvmsg(sock, &msg,&vec, 1 /*num*/, ring_free*1448 /*size*/, 0 /*flags*/)) <= 0)) {
            exhausted = 1;
            break;
        }
        ring_free--;
        if((sock->type == SOCK_DGRAM)||(sock->type == SOCK_RAW)) {
            char *p_addr = (char *)msg.msg_iov->head->pkt.data;
            p_addr -= msg.msg_namelen;
            rte_memcpy(p_addr,msg.msg_name,msg.msg_namelen);
        } 
        
        if(ipaugenblick_submit_rx_buf(msg.msg_iov->head,socket_satelite_data)) {
            printf("%s %d\n",__FILE__,__LINE__);
            rte_pktmbuf_free(msg.msg_iov->head);
        }
        else {
            user_rx_mbufs++;
        }
        memset(&vec,0,sizeof(vec));
    }

    if((!ring_free)||(exhausted)) {
        user_on_rx_opportunity_called_exhausted += exhausted;
        user_kick_socket_rx++;
        ipaugenblick_kick_socket(socket_satelite_data);
        return exhausted;
    }
    return 0;
}
static inline __attribute__ ((always_inline)) void user_on_socket_fatal(struct socket *sock)
{
        user_data_available_cbk(sock);/* flush data */
}
static inline __attribute__ ((always_inline)) int user_on_accept(struct socket *sock)
{
        struct socket *newsock = NULL;
        ipaugenblick_cmd_t *cmd;
        void *parent_descriptor;

        while(likely(kernel_accept(sock, &newsock, 0) == 0)) {
                newsock->sk->sk_route_caps |= NETIF_F_SG |NETIF_F_ALL_CSUM|NETIF_F_GSO;
                cmd = ipaugenblick_get_free_command_buf();
                if(cmd) {
                    cmd->cmd = IPAUGENBLICK_SOCKET_ACCEPTED_COMMAND;
                    parent_descriptor = get_user_data(sock); 
                    cmd->u.accepted_socket.socket_descr = newsock;
                    app_glue_set_user_data(newsock,NULL);
                    ipaugenblick_post_accepted(cmd,parent_descriptor);
                }
        }
}

extern void print_user_stats();

#endif /* API_H_ */
