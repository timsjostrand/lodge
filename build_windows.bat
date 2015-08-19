:: "C:/Program Files (x86)/Microsoft Visual Studio 12.0/VC/bin/vcvars32.bat"

clang-cl -o glpong.exe main.c math4.c ^
    /D _WIN32 ^
    /D _USE_MATH_DEFINES ^
    /I"C:/programmering/glfw-3.1.1.bin.WIN32/include" ^
    /I"C:/programmering/glew-1.13.0/include" ^
    /link ^
        /nodefaultlib:libcmt.lib^
        /libpath:"C:/programmering/glfw-3.1.1.bin.WIN32/lib-vc2013/"^
        /libpath:"C:/programmering/glew-1.13.0/lib/Release/Win32/"^
        opengl32.lib glfw3.lib glew32.lib ^
        msvcrt.lib user32.lib gdi32.lib kernel32.lib winspool.lib comdlg32.lib ^
        advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
