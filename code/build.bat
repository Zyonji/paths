@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set WarningFlags=/W4 /wd4100 /wd4201 /wd4505 /wd4706 /WX
set OptionFlags=-DCOMPILE_WIN32=1
set LinkerFlags=/link /INCREMENTAL:NO /OPT:REF user32.lib Gdi32.lib

set BuildFlags=/FC /fp:fast /GL /GR- /Gw /nologo /Oi
set DebugFlags=/Od /Zi
set OptimizedFlags=/O2

set CompilerFlags=%BuildFlags% %DebugFlags% %WarningFlags% %OptionFlags%
REM set CompilerFlags=%BuildFlags% %OptimizedFlags% %WarningFlags% %OptionFlags%

IF NOT EXIST %~dp0\..\build mkdir %~dp0\..\build
pushd %~dp0\..\build
cl %CompilerFlags% %~dp0\win32_paths.cpp %LinkerFlags%
popd