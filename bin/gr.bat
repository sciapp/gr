@echo off
setlocal
set GRDIR=S:\gr
set GSBIN=S:\gs\gs9.06\bin
set GSLIB=S:\gs\gs9.06\lib
set WXLIB=S:\wxWidgets\lib\vc100_dll
set PYTHONPATH=%GRDIR%
set PATH=%GRDIR%;%GSBIN%;%WXLIB%;%PATH%
S:\Python27\python %1 %2 %3 %4 %5 %6 %7 %8
endlocal
