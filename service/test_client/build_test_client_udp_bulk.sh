FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_main_udp_bulk.c -o ipaugenblick_main_udp_bulk.o
gcc ipaugenblick_main_udp_bulk.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../build/libnetinet.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_client_udp_bulk
