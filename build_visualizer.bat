set OUTPUT=app_visualizer
call params.bat %1 %OUTPUT%
set INCLUDE_PATHS=%INCLUDE_PATHS% -I../src/bgfx/include -I../src/bgfx/common^
   -I../src/bgfx/3rdparty -I../src/bgfx/include/compat/msvc
set COMMON_CDEFS=%COMMON_CDEFS% -D__STDC_FORMAT_MACROS -D_DLL
set COMMON_LIB_PATHS=%COMMON_LIB_PATHS% -libpath:"../src/bgfx/lib"
set COMMON_LDLIBS=%COMMON_LDLIBS% gdi32.lib shell32.lib comdlg32.lib astc-codec.lib astc.lib
if %1==Release (set LDLIBS= ucrt.lib vcruntime.lib msvcrt.lib msvcprt.lib concrt.lib bgfx.lib bimg.lib bx.lib)
if %1==FastDebug (set LDLIBS= ucrt.lib vcruntime.lib msvcrt.lib msvcprt.lib concrt.lib bgfx.lib bimg.lib bx.lib)
if %1==Debug (set LDLIBS= ucrtd.lib vcruntimed.lib msvcrtd.lib msvcprtd.lib concrtd.lib bgfxd.lib bimgd.lib bxd.lib)

:: Compiling
@echo on
md bin
cd bin
clang++.exe "../src/%OUTPUT%.cpp" -o "%OUTPUT%.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%^
  -I../src/bgfx/include/compat/msvc
clang++.exe "../src/bgfx/common/bgfx_utils.cpp" -o "bgfx_utils.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%
clang++.exe "../src/bgfx/3rdparty/dear-imgui/imgui.cpp" -o "imgui.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%
clang++.exe "../src/bgfx/3rdparty/dear-imgui/imgui_draw.cpp" -o "imgui_draw.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%
clang++.exe "../src/bgfx/3rdparty/dear-imgui/imgui_widgets.cpp" -o "imgui_widgets.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%
clang++.exe "../src/bgfx/common/imgui/imgui.cpp" -o "imgui_bgfx.o"^
  -c %INCLUDE_PATHS% %COMMON_CFLAGS% %CFLAGS% %COMMON_CDEFS% %CDEFS%

:: Linking
link.exe "%OUTPUT%.o" /DEBUG -out:"%OUTPUT%.exe"^
  %COMMON_LDFLAGS% %LDFLAGS% %COMMON_LIB_PATHS% %COMMON_LDLIBS% %LDLIBS%^
  imgui.o imgui_draw.o imgui_widgets.o imgui_bgfx.o bgfx_utils.o
del "%OUTPUT%.o" bgfx_utils.o imgui.o imgui_draw.o imgui_widgets.o imgui_bgfx.o
cd ..

@echo off
set "PATH=%OLD_PATH%"
