FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_main_tcp_udp.c -o ipaugenblick_main_tcp_udp.o
gcc ipaugenblick_main_tcp_udp.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../build/libnetinet.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_client_tcp_udp
