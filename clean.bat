@echo off

cd .
del *.opt *.sdf *.ncb

cd bin
del *.exp *.pdb *.ilk
del *_debug*.*

cd ..
rd /S /Q output
md output

goto :eof

