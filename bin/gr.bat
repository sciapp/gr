@echo off
setlocal
set GRDIR=S:\gr
set PYTHONPATH=%GRDIR%
set PATH=%GRDIR%;%PATH%
S:\Python27\python %1 %2 %3 %4 %5 %6 %7 %8
endlocal
