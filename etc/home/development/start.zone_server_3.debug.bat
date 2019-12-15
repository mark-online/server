@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "zone_server_3" sne.launcher.debug.exe -v -p "-c zone_server_3.ini" %GIDEON_SERVER_DIR%\bin\x64\ZoneServer.debug.dll
