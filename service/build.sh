rm -f ipaugenblick_srv
gcc ipaugenblick_main.c build/libipaugenblickservice.a ../build/libnetinet.a ../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o ipaugenblick_srv
