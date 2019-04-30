cmake_minimum_required (VERSION 3.12)
project(compress LANGUAGES C CXX)
find_package(zfp REQUIRED)
find_package(stlab REQUIRED)
add_executable(compress compress.cpp
  ../mg_args.cpp ../mg_dataset.cpp ../mg_filesystem.cpp ../mg_io.cpp
  ../mg_memory.cpp ../mg_memory_map.cpp ../mg_mutex.cpp ../mg_stacktrace.cpp
  ../mg_string.cpp ../mg_timer.cpp ../mg_volume.cpp)
set_target_properties(compress PROPERTIES CXX_STANDARD 20)
target_compile_options(compress PUBLIC "SHELL:-Xclang -fopenmp" "SHELL:-Xclang -fcoroutines-ts")
target_link_libraries(compress PUBLIC zfp::zfp stlab::stlab)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(compress PUBLIC
    -Wall -Wno-c++98-compat -Wno-missing-prototypes -Wno-unused-macros
    -Wno-c++98-compat-local-type-template-args -Wno-extra-semi-stmt
    -Wno-zero-as-null-pointer-constant -Wno-c++98-compat-pedantic
    -Wno-old-style-cast -Wno-gnu-zero-variadic-macro-arguments
    -Wno-global-constructors -Wno-cast-align -Wno-exit-time-destructors)
  if (WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	#TODO: only add -g for debug build?
    target_link_options(compress PUBLIC -libpath:${LlvmPath} libomp.lib)
  endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  #target_compile_definitions(compress PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32)
  target_compile_definitions(compress PUBLIC _CRT_SECURE_NO_WARNINGS)
  target_link_options(compress PUBLIC
    -libpath:${VsBasePath} -libpath:${WinSdkCrtPath} -libpath:${WinSdkMPath}
    -libpath:${LlvmPath} dbghelp.lib)
elseif (UNIX OR APPLE OR MINGW)
  #target_compile_options(compress PUBLIC -Wall)
endif()
