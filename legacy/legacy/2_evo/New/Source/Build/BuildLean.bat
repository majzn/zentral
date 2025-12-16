@ECHO OFF

gcc -Wall -Wextra -pedantic -ansi -Os -s -ffunction-sections -fdata-sections -fno-exceptions  -c %1.c -o ../Obj/%1.o
gcc -Wall -Wextra -pedantic -ansi -Os -s -Wl,--gc-sections ../Obj/%1.o -o ../Release/%1.exe -lopengl32 -lkernel32 -lshlwapi -lwinmm -lgdi32 -ldsound