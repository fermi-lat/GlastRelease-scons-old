@echo off
setlocal
set PYTHONROOT=%CALIBGENCALROOT%\python;%PYTHONROOT%;
python %CALIBGENCALROOT%\python\tholdCIGen.py %1 %2 %3 %4 %5 %6 %7 %8 %9
endlocal