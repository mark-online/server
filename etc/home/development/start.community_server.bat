@echo off

set PATH=%PATH%;%SNE_DIR%\bin\x64;%GIDEON_CS_DIR%\bin\x64;%GIDEON_SERVER_DIR%\bin\x64

start "community_server" sne.launcher.exe -v -p "-c community_server.ini" %GIDEON_SERVER_DIR%\bin\x64\CommunityServer.dll
