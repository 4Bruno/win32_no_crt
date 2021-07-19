@echo off

cls

set WindowMode=0
if !%1!==!window! set WindowMode=1
if !%2!==!window! set WindowMode=1
set FlagMainStyle=/DSHOW_CONSOLE=1
set Subsystem=/SUBSYSTEM:console
if !%WindowMode%!==!1! (
    echo -------- Using window app  ------------
    set FlagMainStyle=/DSHOW_CONSOLE=0
    set Subsystem=/SUBSYSTEM:windows
)

set DebugMode=0
if !%1!==!debug! set DebugMode=1
if !%2!==!debug! set DebugMode=1
set Build=release
if !%DebugMode%!==!1! (
    echo -------- Debug Build       ------------
    set Build=debug
)

rem Build dlls will set build path
call ..\build.bat %Build%

pushd %Build%

rem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
rem Pay attention to flags with - at the end which disables them
rem !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

rem No CRuntime requires setting stack to 1 MB (static) to be declared in the linker flags
set AvoidCRuntime=/fp:fast /fp:except- /GS- /Gs9999999
set MaxWarningLevel=/WX /W4 /wd4100 /wd4189
rem Keys: no try/catch, no runtime RTTI (c++ virtual), intrinsics
set CompilerFlags=/I..\src  %FlagMainStyle% /nologo /WL /GR- /EHa- /Zo /Oi %MaxWarningLevel% /FC /Z7 %AvoidCRuntime%
set LinkerFlags=/STACK:0x100000,0x100000 /incremental:no /opt:ref

cl.exe  %CompilerFlags% /DDEBUG_LOG=1 ..\test\test_threads.cpp ..\src\msvc.c ^
    /link %LinkerFlags% %Subsystem% /NODEFAULTLIB user32.lib kernel32.lib win32_window.lib win32_threading.lib

popd %Build% 

