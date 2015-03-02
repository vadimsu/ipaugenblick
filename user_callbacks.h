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
extern uint64_t user_kick_select_rx;
extern uint64_t user_kick_select_tx;
extern uint64_t user_on_tx_opportunity_socket_full;
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
/* once this function is called,
   user application-toward-socket ring is checked.
   If empty, the selector is kicked.
   Otherwise, the data is read from the ring and written to socket. If socket's space is not exhausted,
   selector is kicked. 
*/
static inline __attribute__ ((always_inline)) void user_on_transmission_opportunity(struct socket *sock)
{
        struct page page;
        int i = 0;
        uint32_t to_send_this_time;
        void *socket_satelite_data;
        uint64_t ring_entries;

        user_on_tx_opportunity_called++;

        if(unlikely(!sock)) {
            return;
        }
        socket_satelite_data = get_user_data(sock);

        if(unlikely(!socket_satelite_data)) {
            return;
        }
        if(sock->sk->sk_state == TCP_LISTEN) {
           printf("%s %d\n",__FILE__,__LINE__);exit(0);
        }
        
        if(sock->type == SOCK_STREAM) {
            ring_entries = ipaugenblick_tx_buf_count(socket_satelite_data);
            if(ring_entries == 0) {
                ipaugenblick_mark_writable(socket_satelite_data);
                user_on_tx_opportunity_api_nothing_to_tx++;
                return;
            }
            do {
                i = kernel_sendpage(sock, &page, 0/*offset*/,ring_entries<<10, 0 /*flags*/);
                ring_entries = ipaugenblick_tx_buf_count(socket_satelite_data);
            }while((i > 0)&&(ring_entries > 0));
            if(ring_entries == 0) {
                ipaugenblick_mark_writable(socket_satelite_data);
            }
            else {
                user_on_tx_opportunity_socket_full += (i<=0);
            }
        }
        else if((sock->type == SOCK_DGRAM)||(sock->type == SOCK_RAW)) {
            struct msghdr msghdr;
            struct iovec iov; 
            struct sock *sk = sock->sk;
            int dequeued,rc = 0,exhausted = 0;

            msghdr.msg_namelen = sizeof(struct sockaddr_in);
            msghdr.msg_iov = &iov;
            msghdr.msg_iovlen = 1;
            msghdr.msg_controllen = 0;
            msghdr.msg_control = 0;
            msghdr.msg_flags = 0;

          do {
                ring_entries = ipaugenblick_tx_buf_count(socket_satelite_data);
                dequeued = 0;
               
                if(ring_entries > 0) {
                    struct rte_mbuf *mbuf[ring_entries];
                    int established = (sock->sk) ? (sock->sk->sk_state != TCP_ESTABLISHED) : 0;
                    dequeued = ipaugenblick_dequeue_tx_buf_burst(socket_satelite_data,mbuf,ring_entries);
                    for(i = 0;i < dequeued;i++) {
                        char *p_addr;
                        if(established) {
                            p_addr = (char *)mbuf[i]->pkt.data;
                            p_addr -= sizeof(struct sockaddr_in);
                            msghdr.msg_name = p_addr;
                        }
                        else
                           msghdr.msg_name = NULL;

                        iov.head = mbuf[i]; 
                
                        rc = udp_sendmsg(NULL, sk, &msghdr, mbuf[i]->pkt.data_len);
                        exhausted |= !(rc > 0);
                        if(exhausted) {
                            user_on_tx_opportunity_api_failed += dequeued - i;
                            for(;i < dequeued;i++) {
                                rte_pktmbuf_free(mbuf[i]);
                            }
                            break;
                        }
                    }
//                    printf("ring entries %d dequeued %d\n",ring_entries,dequeued);
                }
                else {
                    user_on_tx_opportunity_api_nothing_to_tx++;
                }
            }while((dequeued > 0) && (!exhausted));
            if(!exhausted)//may write more
                ipaugenblick_mark_writable(socket_satelite_data);
        }
}
/* once data is received, this function is called.
   - If there is no space  in the ring toward user application,
     don't read and kick the selector. Once user is awake, it reads
     the buffers from the ring and kicks the socket. On kick,
     this function is called again
   - If there is insufficient/enugh space, data is read from the socket as much as possible 
     and is  placed in ring toward user application, selector is kicked
*/
static inline __attribute__ ((always_inline)) void user_data_available_cbk(struct socket *sock)
{
    struct msghdr msg;
    struct iovec vec;
    struct rte_mbuf *mbuf;
    int ring_free,exhausted = 0;
    void *socket_satelite_data;
    unsigned int ringset_idx;
    struct sockaddr_in sockaddrin;

    user_on_rx_opportunity_called++;
    memset(&vec,0,sizeof(vec));
    if(unlikely(sock == NULL)) {
        return;
    }
    socket_satelite_data = get_user_data(sock);
    if(!socket_satelite_data) {
        printf("%s %d\n",__FILE__,__LINE__);
        return;
    }

    if(sock->sk->sk_state == TCP_LISTEN) {
        printf("%s %d\n",__FILE__,__LINE__);exit(0);
    }
    
    if((sock->type == SOCK_DGRAM)||(sock->type == SOCK_RAW)) {
        msg.msg_namelen = sizeof(sockaddrin);
        msg.msg_name = &sockaddrin;
    }
    ring_free = ipaugenblick_rx_buf_free_count(socket_satelite_data);
    
    user_rx_ring_full += !ring_free;
    while(ring_free > 0) {
        if(unlikely(kernel_recvmsg(sock, &msg,&vec, 1 /*num*/, ring_free*1448 /*size*/, 0 /*flags*/)) <= 0) {
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
            printf("%s %d\n",__FILE__,__LINE__);//shoud not happen!!!
            rte_pktmbuf_free(msg.msg_iov->head);
            break;
        }
        else {
            user_rx_mbufs++;
        }
        memset(&vec,0,sizeof(vec));
    }

    user_on_rx_opportunity_called_exhausted += exhausted; 
    if((!exhausted)&&(!ring_free)) { 
        ipaugenblick_mark_readable(socket_satelite_data);
    }
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
                    printf("%s %d %p\n",__FILE__,__LINE__,newsock);
                    ipaugenblick_post_accepted(cmd,parent_descriptor);
                }
        }
}

extern void print_user_stats();

#endif /* API_H_ */
