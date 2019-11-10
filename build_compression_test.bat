:: Setup
set OUTPUT=app_compression_test
call params.bat %1 %OUTPUT%
set COMMON_LIB_PATHS= %COMMON_LIB_PATHS% -libpath:"../src/openjp3d"
set LDLIBS=%LDLIBS% openjp2.lib openjp3d.lib

:: Compiling
@echo on
md bin
cd bin
clang++.exe "../src/%OUTPUT%.cpp" -o "%OUTPUT%.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%

:: Linking
link.exe "%OUTPUT%.o" -out:"%OUTPUT%.exe"^
  %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LIB_PATHS% %COMMON_LDLIBS% %LDLIBS%
del "%OUTPUT%.o"
cd ..

@echo off
set "PATH=%OLD_PATH%"
