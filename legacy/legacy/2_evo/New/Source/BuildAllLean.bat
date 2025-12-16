@ECHO OFF
call "Build/CopyAssets"
ECHO Compiling library
call "Build/BuildLean" soyapp
ECHO Compiling DLLs
call "Build/BuildLeanDll" soytracker
Play.lnk