cmake_minimum_required (VERSION 3.12)
project(compress LANGUAGES C CXX)
find_package(zfp REQUIRED)
find_package(stlab REQUIRED)
add_executable(compress compress.cpp ../mg_memory_map.cpp ../mg_string.cpp)
set_target_properties(compress PROPERTIES
  CXX_STANDARD 20
  RUNTIME_OUTPUT_DIRECTORY bin)
target_compile_options(compress PUBLIC -Xclang -fcoroutines-ts)
target_link_libraries(compress PUBLIC zfp::zfp stlab::stlab)
