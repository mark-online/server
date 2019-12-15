REM @echo off
REM
REM Build & release x64
REM

set VERSION=%1

pushd .
if "%VERSION%" == "" goto usage

:setvar
setlocal
set TARGET_NAME=gideon_server_%VERSION%

set REL_BASE_DIR=release
set REL_DIR=%REL_BASE_DIR%\%TARGET_NAME%

:makedirs
mkdir %REL_BASE_DIR%
mkdir %REL_DIR%

echo copy_files
xcopy /S /Y ..\..\bin\x64\CommunityServer.dll %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\CommunityServer.pdb %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\DatabaseProxyServer.dll %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\DatabaseProxyServer.pdb %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\LoginServer.dll %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\LoginServer.pdb %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\serverbase.dll %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\serverbase.pdb %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\ZoneServer.dll %REL_DIR%\
xcopy /S /Y ..\..\bin\x64\ZoneServer.pdb %REL_DIR%\

echo zip
cd %REL_BASE_DIR%
c:\cygwin\bin\zip -9 -r %TARGET_NAME%.zip %TARGET_NAME%
if errorlevel 1 goto done

echo remove_dirs
rmdir /s /q %TARGET_NAME%

goto done

:usage
echo "USAGE: %0 <version>"
goto done

:done
popd

if "%2"=="nowait" goto end
pause

:end
