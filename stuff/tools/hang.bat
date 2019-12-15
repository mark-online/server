@echo off

set pid=%1

if "%pid%"=="" goto usage

mkdir .\dumps
adplus_x64\adplus.exe -hang -p %pid% -o .\dumps

goto done

:usage
echo ------------------------------------------------------
echo "USAGE: %0 <process-id>"
echo Attaches to a process, takes a memory dump and detaches;
echo   used in hang situations.
echo ------------------------------------------------------

:done
