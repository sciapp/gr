@echo off
setlocal
set GRDIR=S:\gr
IF "%1"=="" (
  set args="-"
) ELSE (
  set args=%1 %2 %3 %4 %5 %6 %7 %8
)
IF "%MPLBACKEND%"=="gr" (
  set MPLBACKEND="module://gr.matplotlib.backend_gr"
)
set GSBIN=S:\gs\gs9.06\bin
set GSLIB=S:\gs\gs9.06\lib
set WXLIB=S:\wxWidgets\lib\vc100_dll
set PYTHONPATH=%GRDIR%\python
set PATH=%GRDIR%;%GSBIN%;%WXLIB%;%PATH%
S:\Python27\python %args%
endlocal
