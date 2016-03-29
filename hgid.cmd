@echo off
setlocal ENABLEEXTENSIONS & pushd .
set HGTIPFILE="%~dp0hgid.h"
for /f %%i in ('hg id -i') do @echo #define HG_REV_ID "%%i"> %HGTIPFILE%
for /f %%i in ('hg id -n') do @echo #define HG_REV_NO %%i>>  %HGTIPFILE%
if exist %HGTIPFILE% type %HGTIPFILE%
popd & endlocal & goto :EOF
