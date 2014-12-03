FLAGS=-g
gcc $FLAGS -O -c ipaugenblick_main.c -o ipaugenblick_main.o
gcc ipaugenblick_main.o -L../ipaugenblick_memory_service -lipaugenblick_memory_service -L../ipaugenblick_memory_common -lipaugenblick_memory_common -o test_server
