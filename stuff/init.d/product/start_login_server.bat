@echo off

start "login_server" sne.launcher.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\product\login_server.conf"

