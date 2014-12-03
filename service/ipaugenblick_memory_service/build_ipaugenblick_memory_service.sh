gcc -g -O -c ipaugenblick_init.c -o ipaugenblick_init.o
gcc -g  -O -c ipaugenblick_service.c -o ipaugenblick_service.o
ar rcs libipaugenblick_memory_service.a ipaugenblick_init.o ipaugenblick_service.o
