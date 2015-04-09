FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_route.c -o ipaugenblick_route.o
gcc ipaugenblick_route.o ../ipaugenblick_app_api/build/libipaugenblickservice.a ../../dpdk_libs/libdpdk.a  -lpthread -lrt -ldl -o test_route
