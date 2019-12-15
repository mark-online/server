@echo off

set pid=%1

if "%pid%"=="" goto usage

mkdir .\dumps
adplus_x64\adplus.exe -crash -p %pid% -o .\dumps

goto done

:usage
echo ------------------------------------------------------
echo "USAGE: %0 <process-id>"
echo Attaches to a process and stays attached waiting for exceptions or other events to act.
echo Used mostly when troubleshooting crashes but can be used in many more situations.
echo ------------------------------------------------------

:done
