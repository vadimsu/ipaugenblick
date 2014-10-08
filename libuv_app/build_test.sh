rm -f libuv_app
gcc test.c libuv.a libdpdk.a -lpthread -lrt -ldl -o libuv_app
