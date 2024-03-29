# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   message(FATAL_ERROR "CMake >= 2.6.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 2.6)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_targetsDefined)
set(_targetsNotDefined)
set(_expectedTargets)
foreach(_expectedTarget bgfx::astc-codec bgfx::astc bgfx::edtaa3 bgfx::etc1 bgfx::etc2 bgfx::iqa bgfx::squish bgfx::nvtt bgfx::pvrtc bgfx::bx bgfx::bimg bgfx::bgfx bgfx::shaderc bgfx::geometryc bgfx::texturec bgfx::texturev)
  list(APPEND _expectedTargets ${_expectedTarget})
  if(NOT TARGET ${_expectedTarget})
    list(APPEND _targetsNotDefined ${_expectedTarget})
  endif()
  if(TARGET ${_expectedTarget})
    list(APPEND _targetsDefined ${_expectedTarget})
  endif()
endforeach()
if("${_targetsDefined}" STREQUAL "${_expectedTargets}")
  unset(_targetsDefined)
  unset(_targetsNotDefined)
  unset(_expectedTargets)
  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT "${_targetsDefined}" STREQUAL "")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_targetsDefined}\nTargets not yet defined: ${_targetsNotDefined}\n")
endif()
unset(_targetsDefined)
unset(_targetsNotDefined)
unset(_expectedTargets)


# Compute the installation prefix relative to this file.
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)
if(_IMPORT_PREFIX STREQUAL "/")
  set(_IMPORT_PREFIX "")
endif()

# Create imported target bgfx::astc-codec
add_library(bgfx::astc-codec STATIC IMPORTED)

# Create imported target bgfx::astc
add_library(bgfx::astc STATIC IMPORTED)

# Create imported target bgfx::edtaa3
add_library(bgfx::edtaa3 STATIC IMPORTED)

# Create imported target bgfx::etc1
add_library(bgfx::etc1 STATIC IMPORTED)

# Create imported target bgfx::etc2
add_library(bgfx::etc2 STATIC IMPORTED)

set_target_properties(bgfx::etc2 PROPERTIES
  INTERFACE_LINK_LIBRARIES "bgfx::bx"
)

# Create imported target bgfx::iqa
add_library(bgfx::iqa STATIC IMPORTED)

# Create imported target bgfx::squish
add_library(bgfx::squish STATIC IMPORTED)

# Create imported target bgfx::nvtt
add_library(bgfx::nvtt STATIC IMPORTED)

set_target_properties(bgfx::nvtt PROPERTIES
  INTERFACE_LINK_LIBRARIES "bgfx::bx"
)

# Create imported target bgfx::pvrtc
add_library(bgfx::pvrtc STATIC IMPORTED)

# Create imported target bgfx::bx
add_library(bgfx::bx STATIC IMPORTED)

set_target_properties(bgfx::bx PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "__STDC_LIMIT_MACROS;__STDC_FORMAT_MACROS;__STDC_CONSTANT_MACROS"
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "psapi"
)

# Create imported target bgfx::bimg
add_library(bgfx::bimg STATIC IMPORTED)

set_target_properties(bgfx::bimg PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "bgfx::bx;bgfx::astc-codec;bgfx::astc;bgfx::edtaa3;bgfx::etc1;bgfx::etc2;bgfx::iqa;bgfx::squish;bgfx::nvtt;bgfx::pvrtc"
)

# Create imported target bgfx::bgfx
add_library(bgfx::bgfx STATIC IMPORTED)

set_target_properties(bgfx::bgfx PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "bgfx::bx;bgfx::bimg"
)

# Create imported target bgfx::shaderc
add_executable(bgfx::shaderc IMPORTED)

# Create imported target bgfx::geometryc
add_executable(bgfx::geometryc IMPORTED)

# Create imported target bgfx::texturec
add_executable(bgfx::texturec IMPORTED)

# Create imported target bgfx::texturev
add_executable(bgfx::texturev IMPORTED)

if(CMAKE_VERSION VERSION_LESS 2.8.12)
  message(FATAL_ERROR "This file relies on consumers using CMake 2.8.12 or greater.")
endif()

# Load information for each installed configuration.
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/bgfx-config-*.cmake")
foreach(f ${CONFIG_FILES})
  include(${f})
endforeach()

# Cleanup temporary variables.
set(_IMPORT_PREFIX)

# Loop over all imported files and verify that they actually exist
foreach(target ${_IMPORT_CHECK_TARGETS} )
  foreach(file ${_IMPORT_CHECK_FILES_FOR_${target}} )
    if(NOT EXISTS "${file}" )
      message(FATAL_ERROR "The imported target \"${target}\" references the file
   \"${file}\"
but this file does not exist.  Possible reasons include:
* The file was deleted, renamed, or moved to another location.
* An install or uninstall procedure did not complete successfully.
* The installation package was faulty and contained
   \"${CMAKE_CURRENT_LIST_FILE}\"
but not all the files it references.
")
    endif()
  endforeach()
  unset(_IMPORT_CHECK_FILES_FOR_${target})
endforeach()
unset(_IMPORT_CHECK_TARGETS)

# This file does not depend on other imported targets which have
# been exported from the same project but in a separate export set.

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
