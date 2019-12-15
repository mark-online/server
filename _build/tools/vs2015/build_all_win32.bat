@echo off

call _build.bat Build Debug Win32
if errorlevel 1 goto build_error

call _build.bat Build Release Win32
if errorlevel 1 goto build_error

call _build.bat Build StaticMtDebug Win32
if errorlevel 1 goto build_error

call _build.bat Build StaticMtRelease Win32
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
