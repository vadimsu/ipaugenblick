#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <api.h>

//void *create_client_socket(char *peer_ip_addr,unsigned short port);

//void *create_server_socket(char *my_ip_addr,unsigned short port);

void app_main_loop();
struct socket *udp_socket[10];
void app_init(char *my_ip_addr,unsigned short port)
{
        int i;

        for(i = 0;i < 10;i++) {
	    udp_socket[i] = create_udp_socket(my_ip_addr,port+i);
        }
	app_main_loop();
}
