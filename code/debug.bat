@echo off
call "%~dp0\build.bat"

devenv %~dp0\..\build\win32_paths.exe