FLAGS=-g
gcc $FLAGS -O -c ipaugenblick_main.c -o ipaugenblick_main.o
gcc ipaugenblick_main.o -L../ipaugenblick_app_api -L../ipaugenblick_memory_common -lipaugenblick_api -lipaugenblick_memory_common -o test_client
