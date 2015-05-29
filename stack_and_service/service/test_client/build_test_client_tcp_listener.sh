FLAGS=-g
#FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_main_tcp_listener.c -o ipaugenblick_main_tcp_listener.o
gcc ipaugenblick_main_tcp_listener.o -L/usr/lib/ipaugenblick -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_client_tcp.bin
