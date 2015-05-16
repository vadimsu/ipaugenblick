#FLAGS=-Ofast
FLAGS=-g
gcc $FLAGS -c ipaugenblick_tcp_echo_server.c -o ipaugenblick_tcp_echo_server.o
gcc ipaugenblick_tcp_echo_server.o -L../ipaugenblick_app_api/build/lib -L../../dpdk-1.8.0/x86_64-default-linuxapp-gcc/lib -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc -lpthread -lrt -ldl -o test_client_tcp_echo_server
