@echo off

call start.database_proxy_server.debug.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1

call start.login_server.debug.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1

call start.community_server.debug.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1

call start.zone_server_1.debug.bat
call start.zone_server_2.debug.bat

