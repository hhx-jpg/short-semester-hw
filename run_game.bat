@echo off
chcp 65001 >nul
set PATH=D:\Qt\6.11.1\mingw_64\bin;%PATH%
set QML2_IMPORT_PATH=D:\Qt\6.11.1\mingw_64\qml
cd /d "%~dp0build"
echo Starting SkyboundTactics.exe ...
start /B SkyboundTactics.exe
echo Game launched! Close this window or press any key to exit.
pause >nul
