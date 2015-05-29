FLAGS=-Ofast
gcc $FLAGS -c ipaugenblick_route.c -o ipaugenblick_route.o
gcc ipaugenblick_route.o -L/usr/lib/ipaugenblick -lipaugenblickservice -lrte_ring -lrte_timer -lrte_eal -lrte_mempool -lrte_malloc  -lpthread -lrt -ldl -o test_route
