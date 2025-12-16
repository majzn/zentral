@ECHO OFF


gcc -Wall -Wextra -pedantic -ansi -Ofast -c %1.c -o ../Obj/%1.o
gcc -Wall -Wextra -pedantic -ansi -Ofast ../Obj/%1.o -o ../Release/%1.exe -lopengl32 -lkernel32 -lshlwapi -lwinmm -lgdi32 -ldsound
