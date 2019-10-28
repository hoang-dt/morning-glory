@echo off

:: Parameters
set "LLVMPath=%userprofile%\scoop\apps\llvm\9.0.0"
set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
set "VSVersion=14.23.28105"
set "WinSDKVersion=10.0.17763.0"
set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
set "OUTPUT=app_compression_test.exe"

:: Setup
set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
set "OLD_PATH=%PATH%"
set "PATH=%LLVMPath%\bin;%VSBasePath%\bin\Hostx64\x64;%PATH%"

:: Compiler flags
set INCLUDE_PATHS=-I../src
set CFLAGS="Please provide a build config: Debug, FastDebug, Release"
set COMMON_CFLAGS= -Xclang -flto-visibility-public-std -std=gnu++2a ^
  -fdiagnostics-absolute-paths -fopenmp -fopenmp-simd -fms-extensions^
  -Wall -Wextra -pedantic -Wno-gnu-zero-variadic-macro-arguments -Wfatal-errors ^
  -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-vla-extension^
  -g -gcodeview -gno-column-info -mavx2 -mlzcnt
if %1==Release (set CFLAGS= -O3 -funroll-loops -DNDEBUG -ftree-vectorize -march=native)
if %1==FastDebug (set CFLAGS= -Og -DNDEBUG -ftree-vectorize -march=native)
if %1==Debug (set CFLAGS= -O0 -D_DEBUG -march=native)

set COMMON_CDEFS= -D_CRT_SECURE_NO_WARNINGS
if %1==Release (set CDEFS= )
if %1==FastDebug (set CDEFS= -Dmg_Slow=1 -Dmg_Verbose=1 -Dmg_CollectStats=1)
if %1==Debug (set CDEFS= -Dmg_Slow=1 -Dmg_Verbose=1 -Dmg_CollectStats=1)

:: Linker flags
set COMMON_LDFLAGS= -machine:x64 -nodefaultlib -subsystem:console -incremental:no -debug:full -opt:ref,icf
if %1==Release (set LDFLAGS= )
if %1==FastDebug (set LDFLAGS= )
if %1==Debug (set LDFLAGS= )

:: Linked libs
set COMMON_LIB_PATHS= -libpath:"%VSBasePath%\lib\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
  -libpath:"%LLVMPath%\lib"
set COMMON_LIBS= kernel32.lib User32.lib ^
  legacy_stdio_definitions.lib oldnames.lib legacy_stdio_wide_specifiers.lib ^
  libomp.lib dbghelp.lib
if %1==Release (set LDLIBS= libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib libconcrt.lib)
if %1==FastDebug (set LDLIBS= libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib libconcrt.lib)
if %1==Debug (set LDLIBS= libucrtd.lib libvcruntimed.lib libcmtd.lib libcpmtd.lib libconcrtd.lib)

:: Compiling
@echo on
::@for %%f in (*.cpp) do clang++.exe "%%~f" -o "%%~nf.o" -c %CFLAGS% %CDEFS%
md bin
cd bin
clang++.exe "../src/app_compression_test.cpp" -o "app_compression_test.o" -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%

:: Linking
::@set "LINK_FILES="
::@for %%f in (*.o) do @call set LINK_FILES=%%LINK_FILES%% "%%~f"

::lld-link.exe %LINK_FILES% -out:"%OUTPUT%" %LDFLAGS% %LDLIBS%
::link %LINK_FILES% %LDFLAGS% %LDLIBS% -out:"%OUTPUT%"
link.exe "app_compression_test.o" -libpath:"e:\Workspace\file-format-new\src\openjp3d" /DEBUG -out:"%OUTPUT%" %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LIB_PATHS% %COMMON_LIBS% %LDLIBS%
::link.exe "app_compression_test.o" -libpath:"e:\Workspace\file-format-new\src\openjp3d" /DEBUG -out:"%OUTPUT%" openjp2.lib %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LIB_PATHS% %COMMON_LIBS% %LDLIBS%
del "app_compression_test.o"
cd ..

@echo off
set "PATH=%OLD_PATH%"
