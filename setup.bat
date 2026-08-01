@echo off
setlocal

powershell.exe ^
    -NoProfile ^
    -ExecutionPolicy Bypass ^
    -File "scripts\bootstrap\Bootstrapper.ps1" ^
    -ProjectRoot "%~dp0."