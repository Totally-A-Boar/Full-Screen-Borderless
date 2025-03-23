@echo off
echo Compiling fsb.cc with MinGW g++
g++ -m32 -O2 -s -o fsb.exe fsb.cc -luser32
if %ERRORLEVEL% neq 0 (
    echo Compilation failed.
	pause
    exit /b %ERRORLEVEL%
)
pause
echo Compilation successful. fsb.exe created.
