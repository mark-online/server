@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "arena_server" sne.launcher.debug.exe -v -p "-c arena_server.ini" %GIDEON_SERVER_DIR%\bin\x64\ZoneServer.debug.dll
