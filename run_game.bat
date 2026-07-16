@echo off
chcp 65001 >nul

REM ── Qt DLL 路径（核心 Qt 库） ──
set PATH=C:\Qt\6.11.1\mingw_64\bin;%PATH%

REM ── MinGW 运行时 DLL 路径（libstdc++-6.dll 等） ──
set PATH=C:\Qt\Tools\mingw1310_64\bin;%PATH%

REM ── Qt 平台插件路径（qwindows.dll，否则报 "no platform plugin"） ──
set QT_PLUGIN_PATH=C:\Qt\6.11.1\mingw_64\plugins

REM ── QML 模块导入路径 ──
set QML2_IMPORT_PATH=C:\Qt\6.11.1\mingw_64\qml

cd /d "%~dp0build"
echo Starting SkyboundTactics.exe ...
start /B SkyboundTactics.exe
echo Game launched! Close this window or press any key to exit.
pause >nul
