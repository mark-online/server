@echo off

start "arena_server" sne.launcher.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\arena_server.conf"

