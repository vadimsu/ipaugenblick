gcc -g -fPIC -O -c ipaugenblick_memory_common.c -o ipaugenblick_memory_common.o
gcc -g -fPIC -O -c ipaugenblick_ring.c -o ipaugenblick_ring.o
ar rcs libipaugenblick_memory_common.a ipaugenblick_memory_common.o ipaugenblick_ring.o
