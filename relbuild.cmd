@echo off
setlocal ENABLEEXTENSIONS & pushd .
premake4.exe --release vs2005
call %~dp0setvcvars.cmd 2005
echo %VCVER_FRIENDLY%
vcbuild.exe /time /rebuild /showenv /M1 /nologo "/htmllog:$(SolutionDir)buildlog.html" "%~dp0ntobjx_release.vs8.sln" "$ALL"
call ollisign.cmd /a ntobjx_release\*.exe "https://bitbucket.org/assarbad/ntobjx" "ntobjx: NT Objects"
popd & endlocal & goto :EOF
