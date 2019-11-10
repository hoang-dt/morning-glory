@echo off

:: Parameters
set "LLVMPath=%userprofile%\scoop\apps\llvm\9.0.0"
set "VSPath=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community"
set "VSVersion=14.23.28105"
set "WinSDKVersion=10.0.17763.0"
set "WinSDKPath=C:\Program Files (x86)\Windows Kits\10"
set "OUTPUT=%2"

:: Setup
set "VSBasePath=%VSPath%\VC\Tools\MSVC\%VSVersion%"
set "OLD_PATH=%PATH%"
set "PATH=%LLVMPath%\bin;%VSBasePath%\bin\Hostx64\x64;%PATH%"

:: Compiler flags
set INCLUDE_PATHS=-I../src
set CFLAGS="Please provide a build config: Debug, FastDebug, Release"
set COMMON_CFLAGS=-Xclang -flto-visibility-public-std -std=gnu++2a -pedantic^
  -g -gcodeview -gno-column-info -mavx2 -mlzcnt -march=native^
  -fdiagnostics-absolute-paths -fopenmp -fopenmp-simd -fms-extensions^
  -Wall -Wextra -Wfatal-errors -Wno-nested-anon-types -Wno-vla-extension^
  -Wno-gnu-anonymous-struct -Wno-missing-braces -Wno-gnu-zero-variadic-macro-arguments
if %1==Release (set CFLAGS=-O2 -funroll-loops -DNDEBUG -ftree-vectorize)
if %1==FastDebug (set CFLAGS=-Og -DNDEBUG -ftree-vectorize)
if %1==Debug (set CFLAGS=-O0 -D_DEBUG)

:: Compiler defs
set COMMON_CDEFS=-D_CRT_SECURE_NO_WARNINGS
if %1==Release (set CDEFS=)
if %1==FastDebug (set CDEFS=-Dmg_Slow=1 -Dmg_Verbose=1)
if %1==Debug (set CDEFS=-Dmg_Slow=1 -Dmg_Verbose=1)

:: Linker flags
set COMMON_LDFLAGS=-machine:x64 -nodefaultlib -subsystem:console^
  -incremental:no -debug:full -opt:ref,icf
if %1==Release (set LDFLAGS=)
if %1==FastDebug (set LDFLAGS=)
if %1==Debug (set LDFLAGS=)

:: Linker lib paths
set COMMON_LIB_PATHS=-libpath:"%VSBasePath%\lib\x64"^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\ucrt\x64"^
  -libpath:"%WinSDKPath%\Lib\%WinSDKVersion%\um\x64"^
  -libpath:"%LLVMPath%\lib"
:: Linker libs
set COMMON_LDLIBS=kernel32.lib User32.lib legacy_stdio_definitions.lib^
 oldnames.lib legacy_stdio_wide_specifiers.lib libomp.lib dbghelp.lib
if %1==Release (set LDLIBS=libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib libconcrt.lib)
if %1==FastDebug (set LDLIBS=libucrt.lib libvcruntime.lib libcmt.lib libcpmt.lib libconcrt.lib)
if %1==Debug (set LDLIBS=libucrtd.lib libvcruntimed.lib libcmtd.lib libcpmtd.lib libconcrtd.lib)
