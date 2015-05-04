
#ifndef __IPAUGENBLICK_API_H__
#define __IPAUGENBLICK_API_H__

#include <sys/time.h>

#define IPAUGENBLICK_MAX_SOCKETS 1000

/*
* STRUCTURE:
*    data_and_descriptor
* DESCRIPTION:
*    This structure contains a pointer to buffers data (where the user can write data to send) and buffers descriptors (required when sending the buffers)
* FIELDS:
*    - pdata - a pointer to data
*    - pdesc - buffer's descriptor
*/
struct data_and_descriptor
{
    void *pdata;
    void *pdesc;
};

typedef void (*ipaugenblick_update_cbk_t)(unsigned char command,unsigned char *buffer,int len);

/*
* FUNCTION:
*    int ipaugenblick_app_init(int argc, char **argv,char *app_unique_id)
* DESCRIPTION:
*    This function must be called by an application before any other IPAugenblick
*    API function is called. This will map the data structures used for communicating
*    IPAugenblick service and initialize the data structures used by API library.
* PARAMETERS:
*    - argc - a number of command line parameters passed in argv
*    - argv - an array of pointers to string options, the form is "-c <core mask> -n <number of memory channels> --proc-type secondary". Example: "-c 0xc -n 1 --proc-type secondary". This means
*    the application will run on cores 2 & 3 with one memory channel. Note that --proc-type secondary
*    is mandatory
*    - app_unique_id - string identifying the application (to distinct several applications communicating IPAugenblick service)
* RETURNS:
*    0 if succeeded. Otherwise the process will exit
*/
extern int ipaugenblick_app_init(int argc, char **argv,char *app_unique_id);

extern int ipaugenblick_create_client(ipaugenblick_update_cbk_t update_cbk);

extern int ipaugenblick_read_updates(void);

/*
* FUNCTION:
*    int ipaugenblick_open_socket(int family,int type,int parent)
* DESCRIPTION:
*    This function will open a socket (similar to POSIX socket call)
* PARAMETERS:
*    - family: may be AF_INET, AF_PACKET
*    - type: may be SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
*    - parent: selector's descriptor the socket will be associated with (consult ipaugenblick_open_select)
* RETURNS:
*    If succeeded, >= 0 - opened socket's descriptor. Otherwise negative value is returned
*/
extern int ipaugenblick_open_socket(int family,int type,int parent);

/*
* FUNCTION:
*    int ipaugenblick_v4_connect_bind_socket(int sock,unsigned int ipaddr,unsigned short port,int is_connect)
* DESCRIPTION:
*    This function connects/binds the socket (returned by ipaugenblick_open_socket) and connects/binds (depending to is_connect flag) to the provided address
* PARAMETERS:
*    - sock - socket's descriptor, returned by ipaugenblick_open_socket
*    - ipaddr - IPv4 address
*    - port - port
*    - is_connect - if 1 this function acts like POSIX connect call (connects to the provided address),
*    otherwise it will bind to the provided address
* RETURNS:
*    0 if succeeded, negative if failed
*/
extern int ipaugenblick_v4_connect_bind_socket(int sock,unsigned int ipaddr,unsigned short port,int is_connect);

/*
* FUNCTION:
*    ipaugenblick_listen_socket(int sock)
* DESCRIPTION:
*    This function will put the socket into the listening state (must be bound before, see above), acts like POSIX listen call
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
* RETURNS:
*    - 0 if succeeded, otherwise negative
*/
extern int ipaugenblick_listen_socket(int sock);

/*
* FUNCTION:
*    ipaugenblick_close(int sock)
* DESCRIPTION:
*    This function will close the socket (like POSIX close call)
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
* RETURNS:
*    0 if succeeded, oterwise negative
*/
extern void ipaugenblick_close(int sock);

/*
* FUNCTION:
*    ipaugenblick_get_socket_tx_space(int sock)
* DESCRIPTION:
*    This function returns a number of buffers that can be allocated and sent.
*    This is a min of currently available buffers and socket's buffer space
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
* RETURNS:
*    A number of buffers can be allocated and sent on the socket (0 if none)
*/
extern int ipaugenblick_get_socket_tx_space(int sock);

/*
* FUNCTION:
*    void *ipaugenblick_get_buffer(int length,int owner_sock,void **pdesc)
* DESCRIPTION:
*    This function allocates a buffer (associated with the socket)
* PARAMETERS:
*    - length - buffer's length (currently ignored, a single buffer can contain upto 1448 bytes)
*    - owner_sock - a socket's descriptor, if no buffers to allocate, it is used to signal the service to   
*    wake up the application when some buffers become available
*    - pdesc - allocated buffer's descriptor
* RETURNS:
*    A pointer to data in the allocated buffer, if succeeded, otherwise NULL
*/
extern void *ipaugenblick_get_buffer(int length,int owner_sock,void **pdesc);

/*
* FUNCTION:
*    int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,struct data_and_descriptor *bufs_and_desc)
* DESCRIPTION:
*    This function allocates an array of buffers (which can be later sent at once)
* PARAMETERS:
*    - length - length of the buffer (ignored, upto 1448 for now)
*    - owner_sock - a socket's descriptor, if no buffers to allocate, it is used to signal the service to
*    wake up the application when some buffers become available
*    - count - an amount of buffers to allocate
*    - bufs_and_desc - an array of pointers to buffers' data and buffers' descriptors (consult above regarding its structure)
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_get_buffers_bulk(int length,int owner_sock,int count,struct data_and_descriptor *bufs_and_desc);

/*
* FUNCTION:
*    ipaugenblick_send(int sock,void *pdesc,int offset,int length)
* DESCRIPTION:
*    This function is used to send data on a connected (TCP or connected UDP) socket
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
*    - pdesc - buffer's descriptor (returned when a buffer is allocated)
*    - offset - offset in the buffer
*    - length - buffer's length
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_send(int sock,void *pdesc,int offset,int length);

/*
* FUNCTION:
*    ipaugenblick_send_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths,int buffer_count)
* DESCRIPTION:
*    This function is used to send data on a connected (TCP or connected UDP) socket in bulk
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
*    - bufs_and_desc - an array of struct data_and_descriptor (size of array is buffer_count)
*    - offsets - an array of offsets (size of array is buffer_count)
*    - lengths - an array of lengths (size of array is buffer_count)
*    - buffer_count - size of bufs_and_desc, offsets and lengths arrays
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_send_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths,int buffer_count);

/*
* FUNCTION:
*    int ipaugenblick_sendto(int sock,void *pdesc,int offset,int length,unsigned int ipaddr,unsigned short port)
* DESCRIPTION:
*    This function is used to send data on UDP/RAW socket to a specified destination
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
*    - pdesc - buffer's descriptor (returned when a buffer is allocated)
*    - offset - offset in the buffer
*    - length - length of the buffer
*    - ipaddr - IPv4 address
*    - port - port
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_sendto(int sock,void *pdesc,int offset,int length,unsigned int ipaddr,unsigned short port);

/*
* FUNCTION:
*    ipaugenblick_sendto_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths,unsigned int *ipaddrs,unsigned short *ports, int buffer_count)
* DESCRIPTION:
*    This function is used to send data on a UDP/RAW socket in bulk to specified destinations
* PARAMETERS:
*    - sock - socket's descriptor (returned by ipaugenblick_open_socket)
*    - bufs_and_desc - an array of struct data_and_descriptor (size of array is buffer_count)
*    - offsets - an array of offsets (size of array is buffer_count)
*    - lengths - an array of lengths (size of array is buffer_count)
*    - ipaddrs - an array of IPv4 addresses (size of array is buffer_count)
*    - ports - an array of ports (size of array is buffer_count)
*    - buffer_count - size of bufs_and_desc, offsets and lengths arrays
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_sendto_bulk(int sock,struct data_and_descriptor *bufs_and_desc,int *offsets,int *lengths,unsigned int *ipaddrs,unsigned short *ports,int buffer_count);

/*
* FUNCTION:
*    ipaugenblick_socket_kick(int sock)
* DESCRIPTION:
*    Once one of ipaugenblick_send* functions is called, to ensure ipaugenblick service is notified, this function is called.
*    The mechanism is similar to vhost/virtio kicks
* PARAMETERS:
*    - sock - a socket's descriptor whose rings the ipaugenblick service is supposed to read.
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_socket_kick(int sock);

/*
* FUNCTION:
*    void *ipaugenblick_get_next_buffer_segment(void **pdesc,int *len)
* DESCRIPTION:
*    This function returns a pointer to the data of the next (chained) buffer's segment, its descriptor and its length.
*    This function is supposed to be called for the buffers returned by ipaugenblick_receive and ipaugenblick_receivefrom
* PARAMETERS:
*    - pdesc (IN and OUT) - an address of buffer's descriptor (returned by ipaugenblick_receive*). In case the return value is not NULL,
*    an address of the next buffer's descriptor is written here
*    - len (OUT) - an address where the returned buffer length is written
* RETURNS:
*    If there is a next segment in the chained buffer, an address of the data in that buffer is returned. In this case,
*    the buffer descriptor is returned in pdesc and the buffer length is returned in len. If there is no more segments,
*    returns NULL, in this case pdesc and len are untouched
*/
extern void *ipaugenblick_get_next_buffer_segment(void **pdesc,int *len);

/*
* FUNCTION:
*    void ipaugenblick_release_rx_buffer(void *pdesc,int sock)
* DESCRIPTION:
*    This function is called to free the buffer's chain returned in ipaugenblick_receive*
* PARAMETERS:
*    - pdesc - a buffer descriptor, returned by ipaugenblick_receive*
*    - sock - a socket on which ipaugenblick_receive* was called
* RETURNS:
*    None
*/
extern void ipaugenblick_release_rx_buffer(void *pdesc,int sock);

/*
* FUNCTION:
*    ipaugenblick_receive(int sock,void **buffer,int *len,int *first_segment_len,void **pdesc)
* DESCRIPTION:
*    This function is called to receive a data (buffer's chain) on a TCP socket
* PARAMETERS:
*    - sock - socket's descriptor
*    - buffer - an address of the pointer which (in case of success) will point to the data in the first
*    segment of the buffer chain
*    - len - IN/OUT, in case *len is 0, all the pending data is read from the socket. Otherwise, only specified amount
*    is read. Note that there can be less data then requested to read. In this case, *len will contain an exact number of bytes retrieved (if any)
*    - first_segment_len - an address where the length of the first segment is written. Following segments are retrieved by calling
*    ipaugenblick_get_next_buffer_segment, which also returns buffer descriptor and buffer length for the segment.
*    - pdesc - an address of the buffer descriptor for the first buffer in the chain (and whole chain, save it before calling ipaugenblick_get_next_buffer_segment)
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_receive(int sock,void **buffer,int *len,int *first_segment_len,void **pdesc);

/*
* FUNCTION:
*    ipaugenblick_receivefrom(int sock,void **buffer,int *len,unsigned int *ipaddr, unsigned short *port, int *first_segment_len,void **pdesc)
* DESCRIPTION:
*    This function is called to receive a data (buffer's chain) on a TCP socket
* PARAMETERS:
*    - sock - socket's descriptor
*    - buffer - an address of the pointer which (in case of success) will point to the data in the first
*    segment of the buffer chain
*    - len - IN/OUT, in case *len is 0, all the pending data is read from the socket. Otherwise, only specified amount
*    is read. Note that there can be less data then requested to read. In this case, *len will contain an exact number of bytes retrieved (if any)
*    - first_segment_len - an address where the length of the first segment is written. Following segments are retrieved by calling
*    ipaugenblick_get_next_buffer_segment, which also returns buffer descriptor and buffer length for the segment.
*     - pdesc - an address of the buffer descriptor for the first buffer in the chain (and whole chain, save it before calling ipaugenblick_get_next_buffer_segment)
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_receivefrom(int sock,void **buffer,int *len,unsigned int *ipaddr,unsigned short *port,void **pdesc);

/*
* FUNCTION:
*    ipaugenblick_release_tx_buffer(void *pdesc)
* DESCRIPTION:
*    This function is called for a buffer that was allocated using ipaugenblick_get_buffer/ipaugenblick_get_buffers_bulk.
* PARAMETERS:
*    - pdesc - buffer's descriptor
* RETURNS:
*    None
*/
extern void ipaugenblick_release_tx_buffer(void *pdesc);

/*
* FUNCTION:
*    ipaugenblick_accept(int sock,unsigned int *ipaddr,unsigned short *port)
* DESCRIPTION:
*    This function accepts a new socket connected to listening
* PARAMETERS:
*    - sock - socket's descriptor of the listening socket
*    - ipaddr - a pointer to accepted connection's IPv4 address
*    - port - a pointer to accepted connection's port
* RETURNS:
*    An accepted socket's descriptor, if succeeded. Otherwise -1
*/
extern int ipaugenblick_accept(int sock,unsigned int *ipaddr,unsigned short *port);

/*
* FUNCTION:
*    int ipaugenblick_open_select(void)
* DESCRIPTION:
*    This function opens a selector. The selector can be used similary to POSIX select by
*    associating sockets upon either creation (ipaugenblick_open_socket)
*    or later by calling ipaugenblick_set_socket_select
* PARAMETERS:
*    None
* RETURNS:
*    Positive value, if succeeded. Otherwise negative
*/
extern int ipaugenblick_open_select(void);

/*
* FUNCTION:
*    ipaugenblick_set_socket_select(int sock,int select)
* DESCRIPTION:
*    Sets socket's selector (similary to POSIX select, a number of sockets can be watched for readable/writable events).
* PARAMETERS:
*    - sock - socket for which to set the selector
*    - select - selector's descriptor (returned by ipaugenblick_open_select)
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_set_socket_select(int sock,int select);

/*
* FUNCTION:
*    int ipaugenblick_is_connected(int sock)
* DESCRIPTION:
*    Tests if a socket was connected
* PARAMETERS:
*     - sock - socket's descriptor
* RETURNS:
*    - 1 if connected, 0 if not
*/
extern int ipaugenblick_is_connected(int sock);

/*
* FUNCTION:
*    ipaugenblick_select(int selector,unsigned short *mask,struct timeval* timeout)
* DESCRIPTION:
*    This function watches for socket previously associated with the selector for readable/writable events
* PARAMETERS:
*    - selector - selector's descriptor (returned by ipaugenblick_open_select)
*    - mask - a pointer to bitmask (0x1 means a socket is readable, 0x2 means the socket is writable)
*    - timeout - a pointer to struct timeval (original Linux structure). Set the fields to 0 if you want to poll (non-blocking),
*    set the pointer to NULL if you want to block util an event arrives or set tv_sec and/or tv_usec to values to wait.
* RETURNS:
*    A socket's descriptor became readable/writable, otherwise negative
*/
extern int ipaugenblick_select(int selector,unsigned short *mask,struct timeval* timeout);

/*
* FUNCTION:
*    int ipaugenblick_add_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop,short metric)
* DESCRIPTION:
*    Populates a route entry to the ipaugenblick service
* PARAMETERS:
*    - ipaddr - a route entry's destination IPv4 address
*    - mask - address's mask
*    - nexthop - next hop's IPv4 address
*    - metric - route entry metric
* RETURNS:
*    0 if succeeded, otherewise negative
*/
extern int ipaugenblick_add_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop,short metric);

/*
* FUNCTION:
*    int ipaugenblick_del_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop)
* DESCRIPTION:
*    Deletes a route entry from ipaugenblick service
* PARAMETERS:
*    - ipaddr - a route entry's destination IPv4 address
*    - mask - a route entry's mask
*    - nexthop - a route entry's next hop's IPv4 address
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_del_v4_route(unsigned int ipaddr,unsigned int mask,unsigned int nexthop);

/*
* FUNCTION:
*    void ipaugenblick_getsockname(int sock, int is_local,unsigned int *ipaddr,unsigned short *port)
* DESCRIPTION:
*    Gets either local socket IP address/port or remote peer's
* PARAMETERS:
*    - sock - socket's descriptor
*    - is_local - if 1, gets local address, otherwise gets remote peer's
*    - ipaddr - a pointer to IPv4 address
*    - port - a pointer to port
* RETURNS:
*    None
*/
extern void ipaugenblick_getsockname(int sock, int is_local,unsigned int *ipaddr,unsigned short *port);

/*
* FUNCTION:
    int ipaugenblick_setsockopt(int sock, int level, int optname,char *optval, unsigned int optlen)
* DESCRIPTION:
*    This function sets socket's options (similar to POSIX's one)
* PARAMETERS:
*    - sock - socket's descriptor
*    - level
*    - optname
*    - optval
*    - optlen - Consult Linux's setsockopt
* RETURNS:
*    0 if succeeded, otherwise negative
*/
extern int ipaugenblick_setsockopt(int sock, int level, int optname,char *optval, unsigned int optlen);

#endif
