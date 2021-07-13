@echo off
setlocal ENABLEEXTENSIONS & pushd .
set TGTNAME=ntobjx
set PRJNAME=%TGTNAME%_release
set SIGURL=https://sourceforge.net/projects/%TGTNAME%
set SIGDESC=%TGTNAME%: NT Objects
premake4.exe --release vs2005
call "%~dp0setvcvars.cmd" 2005
:: premake4.exe --release --xp vs2017
:: call "%~dp0setvcvars.cmd" 2017
echo %VCVER_FRIENDLY%
vcbuild.exe /time /rebuild /showenv /M1 /nologo "/htmllog:$(SolutionDir)buildlog.html" "%~dp0%PRJNAME%.vs8.sln" "$ALL"
del /f %PRJNAME%\*.idb
call ollisign.cmd /a %PRJNAME%\*.exe "%SIGURL%" "%SIGDESC%"
:: call :DDKBUILD
rd /s /q "%~dp0%PRJNAME%_intermediate"
set SEVENZIP=%ProgramFiles%\7-Zip\7z.exe
if not exist "%SEVENZIP%" set SEVENZIP=%ProgramFiles(x86)%\7-Zip\7z.exe
for /f %%i in ('hg id -i') do @set RELEASE=%%i
for /f %%i in ('hg id -n') do @set RELEASE=%%i-%RELEASE%
set RELARCHIVE=%~dp0%TGTNAME%-%RELEASE%.7z
if exist "%SEVENZIP%" @(
    pushd "%~dp0%PRJNAME%"
    "%SEVENZIP%" a -y -t7z "%RELARCHIVE%" "*.exe" "*.pdb"
    gpg -ba %RELARCHIVE%
    popd
)
popd & endlocal & goto :EOF

:DDKBUILD
setlocal ENABLEEXTENSIONS & pushd "%~dp0ddkbuild"
call ddkbuild.cmd -W7XP free . -cZ
call ollisign.cmd /a objfre_wxp_x86\i386\*.exe "%%SIGURL%%" "%SIGDESC%"
call ddkbuild.cmd -W7NETX64 free . -cZ
call ollisign.cmd /a objfre_wnet_amd64\amd64\*.exe "%%SIGURL%%" "%SIGDESC%"
set WDKBLD=..\%PRJNAME%\wdk
md "%WDKBLD%"
xcopy /y objfre_wxp_x86\i386\*.exe "%WDKBLD%\"
xcopy /y objfre_wnet_amd64\amd64\*.exe "%WDKBLD%\"
xcopy /y objfre_wxp_x86\i386\*.pdb "%WDKBLD%\"
xcopy /y objfre_wnet_amd64\amd64\*.pdb "%WDKBLD%\"
popd & endlocal
