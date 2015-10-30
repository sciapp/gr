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
set PYTHONPATH=%PYTHONPATH%;%GRDIR%\python
set PATH=%GRDIR%;%PATH%
S:\Anaconda\python.exe %args%
endlocal
