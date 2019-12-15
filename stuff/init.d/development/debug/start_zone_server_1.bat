@echo off

start "zone_server_1" sne.launcher.debug.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\zone_server_1.conf.debug"

