@echo off
pushd %~dp0\..\
call Tools\premake\premake5.exe vs2022
popd
pause
