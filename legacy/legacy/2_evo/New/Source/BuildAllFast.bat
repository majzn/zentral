@ECHO OFF
call "Build/CopyAssets"
ECHO Compiling library
call "Build/BuildFastLib" soyapp
ECHO Compiling DLLs
call "Build/BuildFastDll" soytracker
Play.lnk