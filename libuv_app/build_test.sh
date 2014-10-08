rm -f libuv_app
gcc test.c ../build/libnetinet.a ../dpdk_libs/libdpdk.a build/libuv_app libuv.a -lpthread -lrt -ldl -o libuv_app
