FLAGS=-g
gcc $FLAGS -O -c ipaugenblick_main.c -o ipaugenblick_main.o
gcc ipaugenblick_main.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../build/libnetinet.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_client
