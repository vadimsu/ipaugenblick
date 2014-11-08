#include <specific_includes/dummies.h>
#include <specific_includes/linux/types.h>
#include <specific_includes/linux/bitops.h>
#include <specific_includes/linux/slab.h>
#include <specific_includes/linux/hash.h>
#include <specific_includes/linux/socket.h>
#include <api.h>

void app_main_loop();
struct socket *packet_sock;
void app_init(char *dev_name,unsigned short protocol)
{
	packet_sock = create_packet_socket(dev_name,protocol);
	app_main_loop();
}
