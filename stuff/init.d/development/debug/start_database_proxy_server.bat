@echo off

start "database_proxy_server" sne.launcher.debug.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\database_proxy_server.conf.debug"
