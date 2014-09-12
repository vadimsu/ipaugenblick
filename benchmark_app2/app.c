#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <api.h>


void app_main_loop();

void app_init(char *my_ip_addr,unsigned short my_port,char *peer_ip_addr,unsigned short port)
{
	create_client_socket(my_ip_addr,my_port,peer_ip_addr,port);
	app_main_loop();
}
