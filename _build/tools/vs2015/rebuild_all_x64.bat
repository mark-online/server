@echo off

call _build.bat Rebuild Debug x64
if errorlevel 1 goto build_error

call _build.bat Rebuild Release x64
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
