
@echo off
setlocal enabledelayedexpansion

set curPath=%~dp0

rem 延迟加载变量，先替换在加载
set curPath=!curPath:\=\\!

echo %curPath%

wmic process where ExecutablePath="%curPath%cefhelper.exe" delete 
rem call terminate
rem wmic process where name="cefhelper.exe" get name,ExecutablePath
