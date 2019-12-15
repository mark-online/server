@echo off

call _build.bat Clean Debug Win32
if errorlevel 1 goto build_error

call _build.bat Clean Release Win32
if errorlevel 1 goto build_error

call _build.bat Clean Debug x64
if errorlevel 1 goto build_error

call _build.bat Clean Release x64
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
