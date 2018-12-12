@echo "copy_x64"
rem Set SRC and DEST
set SRCPATH= %1
set DESTPATH= %2
mkdir %DESTPATH%
del /S /Q %DESTPATH% >nul
copy %SRCPATH%\*\* %DESTPATH% >nul
goto end

:end
@echo "THE END"