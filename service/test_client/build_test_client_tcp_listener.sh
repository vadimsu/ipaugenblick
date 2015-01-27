FLAGS=-g
gcc $FLAGS -O -c ipaugenblick_main_tcp_listener.c -o ipaugenblick_main_tcp_listener.o
gcc ipaugenblick_main_tcp_listener.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../build/libnetinet.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_client_tcp
