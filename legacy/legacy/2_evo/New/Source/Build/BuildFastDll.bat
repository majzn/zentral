@ECHO OFF

gcc -Wall -Wextra -pedantic -ansi -Ofast -c %1.c -o ../Obj/%1.o
gcc -Wall -Wextra -pedantic -ansi -Ofast -shared ../Obj/%1.o -o ../Release/%1.dll -lopengl32 -lkernel32 -lshlwapi -lwinmm -lgdi32 -ldsound