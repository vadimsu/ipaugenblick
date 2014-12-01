gcc -O -c ipaugenblick_init.c -o ipaugenblick_init.o
gcc -O -c ipaugenblick_service.c -o ipaugenblick_service.o
ar -rcs ipaugenblick_memory_service.a ipaugenblick_init.o ipaugenblick_service.o
