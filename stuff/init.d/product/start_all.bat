@echo off
@REM cygwin이 깔려 있어야 한다

call start_database_proxy_server.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1 

call start_login_server.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1 

call start_community_server.bat
rem wait 2s
ping -n 2 127.0.0.1 > NUL 2>&1 

call start_zone_server_1.bat
call start_zone_server_2.bat
