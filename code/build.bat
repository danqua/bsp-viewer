@echo off
set IncludeDirectories= ^
    /I..\code ^
    /I..\extern\SDL2-2.24.0\include ^
    /I..\extern\glm ^
    /I..\extern\glad\include

set LibraryDirectories= ^
    /LIBPATH:..\extern\SDL2-2.24.0\lib\x64

set SourceFiles= ^
    ..\extern\glad\src\glad.c ^
    ..\code\quake_main.cpp

pushd ..\build
cl  /Zi /FC /EHsc %SourceFiles% Shell32.lib SDL2.lib SDL2main.lib %IncludeDirectories% /Fequake_bsp_viewer.exe /link %LibraryDirectories% /SUBSYSTEM:WINDOWS
popd