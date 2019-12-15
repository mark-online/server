@echo off

start "community_server" sne.launcher.debug.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\community_server.conf.debug"

