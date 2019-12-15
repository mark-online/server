@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "zone_server_1" sne.launcher.exe -v -p "-c zone_server_1.ini" %GIDEON_SERVER_DIR%\bin\x64\ZoneServer.dll
