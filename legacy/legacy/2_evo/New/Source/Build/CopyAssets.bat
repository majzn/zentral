soydoc soylib.h

xcopy /y /s/e "..\Resources\assets" "..\Release\assets\"
xcopy /y /s/e "..\Resources\data" "..\Release\data\"
xcopy /y pal.h "Distributable\src\"
xcopy /y pal_win32.h "Distributable\src\"
xcopy /y pal.c "Distributable\src\"