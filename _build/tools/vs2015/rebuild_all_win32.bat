@echo off

call _build.bat Rebuild Debug Win32
if errorlevel 1 goto build_error

call _build.bat Rebuild Release Win32
if errorlevel 1 goto build_error

call _build.bat Rebuild StaticMtDebug Win32
if errorlevel 1 goto build_error

call _build.bat Rebuild StaticMtRelease Win32
if errorlevel 1 goto build_error

exit /b 0
goto done

:build_error
exit /b 1

:done
