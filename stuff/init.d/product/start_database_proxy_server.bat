@echo off

start "database_proxy_server" sne.launcher.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\product\database_proxy_server.conf"

