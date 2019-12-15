@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "login_server" sne.launcher.exe -v -p "-c login_server.ini" %GIDEON_SERVER_DIR%\bin\x64\LoginServer.dll
