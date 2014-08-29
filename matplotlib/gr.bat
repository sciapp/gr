@echo off
setlocal
set GRDIR=S:\gr
set PYTHONPATH=.\
anaconda -i %1 -dmodule://backend_gr
endlocal
