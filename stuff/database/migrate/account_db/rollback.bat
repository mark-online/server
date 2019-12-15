@echo off

set COUNT=%1

if "%COUNT%" == "" set COUNT=1


call liquibase.bat rollbackCount %COUNT%
