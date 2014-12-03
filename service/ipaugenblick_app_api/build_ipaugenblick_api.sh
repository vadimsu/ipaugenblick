FLAGS=-fPIC -g
gcc $FLAGS -c ipaugenblick_api.c -o ipaugenblick_api.o
ar rcs libipaugenblick_api.a ipaugenblick_api.o
