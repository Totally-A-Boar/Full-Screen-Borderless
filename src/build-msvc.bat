@echo off
setlocal

echo Warning! Compiling fsb via this script may result in a larger binary size.

:: Ensure the Visual Studio environment is set up
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x86

:: Compile the source 
cl /EHsc /O2 /MT /DNDEBUG /Fe:fsb.exe /MACHINE:X86 fsb.cc /link user32.lib

:: Check for successful compilation
if %errorlevel% neq 0 (
	echo Compilation failed!
	pause
	exit /b %errorlevel%
)

echo Successfully compiled fsb.exe
pause

endlocal