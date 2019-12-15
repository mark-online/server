@echo off

call rebuild_all_win32.bat
if errorlevel 1 goto build_error

call rebuild_all_x64.bat
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
