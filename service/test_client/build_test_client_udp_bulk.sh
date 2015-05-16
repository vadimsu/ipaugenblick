#FLAGS=-Ofast
FLAGS=-g
gcc $FLAGS -c ipaugenblick_main_udp_bulk.c -o ipaugenblick_main_udp_bulk.o
gcc ipaugenblick_main_udp_bulk.o -L../ipaugenblick_app_api/build/lib -L../../dpdk-1.8.0/x86_64-default-linuxapp-gcc/lib -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_client_udp_bulk
