@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "database_proxy_server" sne.launcher.debug.exe -v -p "-c database_proxy_server.ini" %GIDEON_SERVER_DIR%\bin\x64\DatabaseProxyServer.debug.dll
