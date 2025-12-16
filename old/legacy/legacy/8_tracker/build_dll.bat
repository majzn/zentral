@ECHO OFF

If [%1]==[] (
ECHO No argument.
GOTO END  
)

SET "OPTIMIZATION= -O1 -fno-strict-aliasing"
SET "STANDARD= -pedantic -ansi"
SET "DEBUG= -Wall"

SET "LINK= -lopengl32 -lkernel32 -lshlwapi -lwinmm -lgdi32 -ldsound -lole32"

ECHO COMPILING / %1 / %2
gcc -c %STANDARD% %DEBUG% %OPTIMIZATION% ../src/%1.c -o ../bin/%1.o
ECHO LINKING
gcc  -shared ../bin/%1.o -o ../bin/%1.dll %LINK%
ECHO FINISHED COMPILING
:END
