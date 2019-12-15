@echo off

call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" x86

SET PROJECT=..\..\vs2010\GideonServer.sln /Project clientnet

devenv %PROJECT% /Build "Debug|Win32"
if errorlevel 1 goto build_error

devenv %PROJECT% /Build "Release|Win32"
if errorlevel 1 goto build_error

devenv %PROJECT% /Build "StaticMtDebug|Win32"
if errorlevel 1 goto build_error

devenv %PROJECT% /Build "StaticMtRelease|Win32"
if errorlevel 1 goto build_error

REM exit /b 0
goto done

:build_error
REM exit /b 1

:done
pause
