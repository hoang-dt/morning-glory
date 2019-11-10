call params.bat %1 %2

:: Compiling
@echo on
md bin
cd bin
clang++.exe "../src/%OUTPUT%.cpp" -o "%OUTPUT%.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%

:: Linking
link.exe "%OUTPUT%.o" /DEBUG -out:"%OUTPUT%.exe"^
  %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LIB_PATHS% %COMMON_LDLIBS% %LDLIBS%
del "%OUTPUT%.o"
cd ..

@echo off
set "PATH=%OLD_PATH%"
