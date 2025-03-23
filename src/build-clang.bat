@echo off
echo Compiling fsb.cc with Clang
clang++ -m32 -O2 -o fsb.exe fsb.cc -luser32
if %ERRORLEVEL% neq 0 (
    echo Compilation failed.
    exit /b %ERRORLEVEL%
)
echo Compilation successful. fsb.exe created.
