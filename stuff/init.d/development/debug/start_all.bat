@echo off

call start_database_proxy_server.bat
rem wait 4s
ping -n 4 127.0.0.1 > NUL 2>&1 

call start_login_server.bat
rem wait 4s
ping -n 4 127.0.0.1 > NUL 2>&1 

call start_community_server.bat
rem wait 4s
ping -n 4 127.0.0.1 > NUL 2>&1 

call start_zone_server_1.bat
call start_zone_server_2.bat
call start_zone_server_3.bat


