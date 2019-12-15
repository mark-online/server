@echo off

start "login_server" sne.launcher.debug.exe -d -h "%GIDEON_SERVER_HOME%" -p "-f conf\development\login_server.conf.debug"

