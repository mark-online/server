REM @echo off
REM
REM Build & release ClientNet
REM

set VERSION=%1

pushd .
if "%VERSION%" == "" goto usage

if "%SNE_DIR%" == "" goto error_env
if "%SNE_DEPENDENT_DIR%" == "" goto error_env

:setvar
setlocal
set TARGET_NAME=gideon_clientnet_%VERSION%
call detail\set_properties.bat

set GIDEON_DIR=..\..

set REL_BASE_DIR=release
set REL_DIR=%REL_BASE_DIR%\%TARGET_NAME%

:makedirs
mkdir %REL_BASE_DIR%
mkdir %REL_DIR%
mkdir %REL_DIR%\bin
mkdir %REL_DIR%\lib

echo copy_clientnet_dll
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.dll %REL_DIR%\bin
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.debug.dll %REL_DIR%\bin

echo copy_clientnet_pdb
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.pdb %REL_DIR%\bin
copy /Y %GIDEON_DIR%\bin\Win32\clientnet.debug.pdb %REL_DIR%\bin

echo copy_clientnet_lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.lib %REL_DIR%\lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.debug.lib %REL_DIR%\lib

echo copy_clientnet_static_mt_lib
copy /Y %GIDEON_DIR%\lib\Win32\clientnet.static.mt.lib %REL_DIR%\lib
REM copy /Y %GIDEON_DIR%\lib\Win32\clientnet.static.mt.debug.lib %REL_DIR%\lib

echo zip
cd %REL_BASE_DIR%
c:\cygwin\bin\zip -9 -r %TARGET_NAME%.zip %TARGET_NAME%
if errorlevel 1 goto done
goto done

echo remove_dirs
c:\cygwin\bin\rmdir /s /q %TARGET_NAME%

goto done

:usage
echo "USAGE: %0 <version>"
goto done

:error_env
echo "SNE_DIR or SNE_DEPENDENT_DIR environment variable is not defined!"

:done
popd

if "%2"=="nowait" goto end
pause

:end
