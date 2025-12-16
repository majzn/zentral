@ECHO OFF

ECHO Compiling library
call "Build/BuildLib" soyapp
ECHO Compiling DLLs
call "Build/BuildDll" evolution
Play.lnk