FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_main_tcp_udp.c -o ipaugenblick_main_tcp_udp.o
gcc ipaugenblick_main_tcp_udp.o -L/usr/lib/ipaugenblick -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_client_tcp_udp.bin
