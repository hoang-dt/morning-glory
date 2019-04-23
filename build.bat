@echo off

:: Parameters
set "LLVMPath=C:\Program Files\LLVM"
set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
set "VSVersion=14.20.27508"
set "WinSDKVersion=10.0.17763.0"
set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
set "OUTPUT=main.exe"

:: Setup
set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
set "OLD_PATH=%PATH%"
set "PATH=%LLVMPath%\bin;%VSBasePath%\bin\Hostx64\x64;%PATH%"

:: Compiler flags
:: TODO: add different build configurations (release, debug, etc)
set CFLAGS="Please provide a build config: Debug, FastDebug, Release"
set COMMON_CFLAGS= -Xclang -flto-visibility-public-std -std=gnu++2a ^
  -fdiagnostics-absolute-paths -fno-exceptions -fno-rtti -fopenmp -fopenmp-simd ^
  -Wall -Wextra -pedantic -Wno-gnu-zero-variadic-macro-arguments -Wfatal-errors -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-missing-braces ^
  -g -gcodeview -gno-column-info
if %1==Release (set CFLAGS= -O2 -DNDEBUG -ftree-vectorize -march=native)
if %1==FastDebug (set CFLAGS= -Og -DNDEBUG -ftree-vectorize -march=native)
if %1==Debug (set CFLAGS= -O0 -D_DEBUG)

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
set COMMON_LDLIBS= -libpath:"%VSBasePath%\lib\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64" ^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64" ^
  -libpath:"%LLVMPath%\lib"
if %1==Release (set LDLIBS= ^
  libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib kernel32.lib User32.lib ^
  legacy_stdio_definitions.lib oldnames.lib legacy_stdio_wide_specifiers.lib ^
  libomp.lib dbghelp.lib)
if %1==FastDebug (set LDLIBS= ^
  libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib kernel32.lib User32.lib ^
  legacy_stdio_definitions.lib oldnames.lib legacy_stdio_wide_specifiers.lib ^
  libomp.lib dbghelp.lib)
if %1==Debug (set LDLIBS= ^
  libucrtd.lib libvcruntimed.lib libcmtd.lib libcpmtd.lib kernel32.lib User32.lib ^
  legacy_stdio_definitions.lib oldnames.lib legacy_stdio_wide_specifiers.lib ^
  libomp.lib dbghelp.lib)

:: Compiling
@echo on
::@for %%f in (*.cpp) do clang++.exe "%%~f" -o "%%~nf.o" -c %CFLAGS% %CDEFS%
md bin
cd bin
clang++.exe "../src/build.cpp" -o "build.o" -c %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%

:: Linking
::@set "LINK_FILES="
::@for %%f in (*.o) do @call set LINK_FILES=%%LINK_FILES%% "%%~f"

::lld-link.exe %LINK_FILES% -out:"%OUTPUT%" %LDFLAGS% %LDLIBS%
::link %LINK_FILES% %LDFLAGS% %LDLIBS% -out:"%OUTPUT%"
::lld-link.exe "build.o" -out:"%OUTPUT%" %LDFLAGS% %LDLIBS%
link.exe "build.o" /DEBUG -out:"%OUTPUT%" %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LDLIBS% %LDLIBS%
del "build.o"
cd ..

@echo off
set "PATH=%OLD_PATH%"
