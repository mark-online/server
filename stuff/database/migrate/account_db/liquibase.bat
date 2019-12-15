@echo off

SET LB=..\liquibase\liquibase.bat

call %LB% --changeLogFile=changelogs\master.xml %1 %2 %3 %4 %5 %6 %7 %8 %9

