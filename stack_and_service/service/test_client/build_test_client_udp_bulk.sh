#FLAGS=-Ofast
FLAGS=-g
gcc $FLAGS -c ipaugenblick_main_udp_bulk.c -o ipaugenblick_main_udp_bulk.o
gcc ipaugenblick_main_udp_bulk.o -L/usr/lib/ipaugenblick -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_client_udp_bulk.bin
