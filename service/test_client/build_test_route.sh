FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_route.c -o ipaugenblick_route.o
gcc ipaugenblick_route.o -L../ipaugenblick_app_api/build/lib -L../../dpdk-1.8.0/x86_64-default-linuxapp-gcc/lib -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_route
