@echo off

for %%i in (..\..\..\log\*.pid) do for /f %%k in (%%i) do taskkill /f /pid %%k
