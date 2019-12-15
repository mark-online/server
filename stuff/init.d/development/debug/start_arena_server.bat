@echo off

start "arena_server" sne.launcher.debug.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\arena_server.conf.debug"

