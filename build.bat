@echo off

:: Parameters
set "LLVMPath=C:\Program Files\LLVM"
set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise"
set "VSVersion=14.16.27023"
set "WinSDKVersion=10.0.17763.0"
set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
set "OUTPUT=main.exe"

:: Setup
set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
set "OLD_PATH=%PATH%"
set "PATH=%LLVMPath%\bin;%VSBasePath%\bin\Hostx64\x64;%PATH%"

:: Compiler flags
:: TODO: add different build configurations (release, debug, etc)
set CFLAGS= ^
  -Xclang -flto-visibility-public-std -std=c++17^
  -fdiagnostics-absolute-paths -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd^
  -Wall -Wextra -pedantic^
  -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-missing-braces^
  -g -gcodeview -gno-column-info

::set CFLAGS= ^
  ::/Od /nologo /fp:fast /fp:except- /EHsc /GR- /Zo /Oi /W4 /wd4201 /wd4100 /wd4189 /wd4505 /wd4127 /FC /Zi /arch:AVX2

set CDEFS= ^
  -D_CRT_SECURE_NO_WARNINGS -Dmg_Slow=1

:: Include directories
::set INCLUDE_DIRS= ^
::   -I "%VSBasePath%\include" ^
::   -I "%WinSDKPath%\Include\%WinSDKVersion%\shared" ^
::   -I "%WinSDKPath%\Include\%WinSDKVersion%\ucrt" ^
::   -I "%WinSDKPath%\Include\%WinSDKVersion%\um"

:: Linker flags
set LDFLAGS= ^
  -machine:x64 -nodefaultlib -subsystem:console -incremental:no -debug:full -opt:ref,icf

:: Linked libs
set LDLIBS= ^
  -libpath:"%VSBasePath%\lib\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
  -libpath:"%LLVMPath%\lib" ^
  libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib kernel32.lib User32.lib ^
  legacy_stdio_definitions.lib oldnames.lib legacy_stdio_wide_specifiers.lib ^
  libomp.lib

:: Compiling
@echo on
::@for %%f in (*.cpp) do clang++.exe "%%~f" -o "%%~nf.o" -c %CFLAGS% %CDEFS%
clang++.exe "build.cpp" -o "build.o" -c %CFLAGS% %CDEFS%

:: Linking
::@set "LINK_FILES="
::@for %%f in (*.o) do @call set LINK_FILES=%%LINK_FILES%% "%%~f"

::lld-link.exe %LINK_FILES% -out:"%OUTPUT%" %LDFLAGS% %LDLIBS%
::link %LINK_FILES% %LDFLAGS% %LDLIBS% -out:"%OUTPUT%"
lld-link.exe "build.o" -out:"%OUTPUT%" %LDFLAGS% %LDLIBS%

@echo off
del "build.o"
set "PATH=%OLD_PATH%"
