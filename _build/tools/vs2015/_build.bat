@echo off
REM
REM Build script
REM

set command=%1
set configuration=%2
set platform=%3

if "%command%"=="" goto usage
if "%configuration%"=="" goto usage
if "%platform%"=="" goto usage

:build
call _msbuild_%platform%.bat _msbuild.xml /t:%command% /p:Configuration=%configuration% /p:Platform=%platform%
exit /b %errorlevel%

goto done

:usage
echo -----------------------------------------------------------------
echo "USAGE: %0 <command> <configuration> <platform>"
echo "  command       := Build | Rebuild | Clean | UnitTest"
echo "  configuration := Debug | Release | StaticDebug | StaticRelease"
echo "  platform := Win32 | x64"
echo -----------------------------------------------------------------
goto done

:build_error
echo *********************
echo *** Build Errors! ***
echo *********************
exit /B 1

:done
exit /B 0

