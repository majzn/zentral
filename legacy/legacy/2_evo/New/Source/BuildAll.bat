@ECHO OFF

call "Build/CopyAssets"
ECHO Compiling library
call "Build/BuildLib" pal
ECHO Compiling DLLs
call "Build/BuildDll" evolution