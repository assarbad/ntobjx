@echo off
setlocal ENABLEEXTENSIONS & pushd .
set PRJNAME=ntobjx_release
set SIGURL=https://bitbucket.org/assarbad/ntobjx
set SIGDESC=ntobjx: NT Objects
premake4.exe --release vs2005
call "%~dp0setvcvars.cmd" 2005
echo %VCVER_FRIENDLY%
vcbuild.exe /time /rebuild /showenv /M1 /nologo "/htmllog:$(SolutionDir)buildlog.html" "%~dp0%PRJNAME%.vs8.sln" "$ALL"
del /f %PRJNAME%\*.idb
call ollisign.cmd /a %PRJNAME%\*.exe "%SIGURL%" "%SIGDESC%"
:: call :DDKBUILD
rd /s /q "%~dp0%PRJNAME%_intermediate"
popd & endlocal & goto :EOF

:DDKBUILD
setlocal ENABLEEXTENSIONS & pushd "%~dp0ddkbuild"
call ddkbuild.cmd -WLH2K free . -cZ
call ollisign.cmd /a objfre_w2k_x86\i386\*.exe "%%SIGURL%%" "%SIGDESC%"
call ddkbuild.cmd -WLHNETX64 free . -cZ
call ollisign.cmd /a objfre_wnet_amd64\amd64\*.exe "%%SIGURL%%" "%SIGDESC%"
set WDKBLD=..\%PRJNAME%\wdk
md "%WDKBLD%"
xcopy /y objfre_w2k_x86\i386\*.exe "%WDKBLD%\"
xcopy /y objfre_wnet_amd64\amd64\*.exe "%WDKBLD%\"
xcopy /y objfre_w2k_x86\i386\*.pdb "%WDKBLD%\"
xcopy /y objfre_wnet_amd64\amd64\*.pdb "%WDKBLD%\"
popd & endlocal
