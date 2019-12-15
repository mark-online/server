@echo off

call _build.bat Build Debug x64
if errorlevel 1 goto build_error

call _build.bat Build Release x64
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
