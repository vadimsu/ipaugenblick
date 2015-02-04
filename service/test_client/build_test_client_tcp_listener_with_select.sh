FLAGS=-g
gcc $FLAGS -O -c ipaugenblick_main_tcp_listener_with_select.c -o ipaugenblick_main_tcp_listener_with_select.o
gcc ipaugenblick_main_tcp_listener_with_select.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../build/libnetinet.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_client_tcp_with_select
