#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/errno.h>
#include <string.h>

extern void ipaugenblick_main_loop(void);

int main(int argc,char **argv)
{
    ipaugenblick_main_loop();
    return 0;
}
