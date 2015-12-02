# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
cmake_minimum_required(VERSION 2.8.12)

project(corgi)

option(corgi_build_component_library
  "Build a library of standard components along with the entity system"
  ON)

set(corgi_standalone_mode OFF)
if("${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
  set(corgi_standalone_mode ON)
  message(STATUS "corgi standalone: building library and samples")
else()
  message(STATUS "corgi library: not building samples")
endif()
option(corgi_build_samples "Build the corgi sample executables."
       ${corgi_standalone_mode})

# Option to enable / disable the test build.
option(corgi_build_tests "Build tests for this project." ON)

# Compile the game with the debug flag
set(corgi_DEBUG ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Directory which contains the source for 3rd party libraries.
if(NOT DEFINED third_party_root)
  get_filename_component(
    third_party_root "${CMAKE_CURRENT_LIST_DIR}/../../../../external"
    REALPATH)
endif()

# Directory which contains source for FPL libraries.
if(NOT DEFINED fpl_root)
  get_filename_component(
      fpl_root "${CMAKE_CURRENT_LIST_DIR}/../../libs" REALPATH)
endif()

# If the dependencies directory exists, assume this is the root directory for
# all libraries required by this project.
if(NOT DEFINED dependencies_root)
  set(dependencies_root "${CMAKE_CURRENT_SOURCE_DIR}/dependencies")
  if(EXISTS "${dependencies_root}")
    set(third_party_root "${dependencies_root}")
    set(fpl_root "${dependencies_root}")
  endif()
endif()

set(dependencies_gtest_dir "${fpl_root}/googletest"
    CACHE PATH "Directory containing the GoogleTest library.")
set(dependencies_flatbuffers_dir "${fpl_root}/flatbuffers"
    CACHE PATH "Directory containing the Flatbuffers library.")
set(dependencies_fplutil_dir "${fpl_root}/fplutil"
    CACHE PATH "Directory containing the fplutil library.")
set(dependencies_breadboard_dir "${fpl_root}/breadboard"
    CACHE PATH "Directory containing the breadboard library.")
set(dependencies_fplbase_dir "${fpl_root}/fplbase"
    CACHE PATH "Directory containing the FPLBase library.")
set(dependencies_mathfu_dir "${fpl_root}/mathfu"
    CACHE PATH "Directory containing the MathFu library.")
set(dependencies_motive_dir "${fpl_root}/motive"
    CACHE PATH "Directory containing the motive animation library.")
set(dependencies_bulletphysics_distr_dir "${third_party_root}/bulletphysics"
    CACHE PATH "Directory containing the Bullet Physics distribution.")
# NOTE: This is only required as the breadboard module library and CORGI's
# component library have a circular dependency.
set(dependencies_corgi_dir "${CMAKE_CURRENT_LIST_DIR}"
    CACHE PATH "Directory containing the CORGI library.")

# Temporary files (like object files) created while compiling projects.
set(tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/obj)

# CORGI source files.
set(corgi_SRCS
    include/corgi/component.h
    include/corgi/component_id_lookup.h
    include/corgi/component_interface.h
    include/corgi/entity_common.h
    include/corgi/entity_manager.h
    include/corgi/entity.h
    include/corgi/vector_pool.h
    src/entity_manager.cpp
    src/version.cpp)

# Includes for this project.
include_directories(src include)

# Detect clang
if(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

# Compiler flags.
set(C_FLAGS_WARNINGS "")
if(MSVC)
  set(C_FLAGS_WARNINGS
      "/W4 /WX /wd4065 /wd4355 /wd4127 /wd4244 /wd4512 /wd4800")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
       CMAKE_COMPILER_IS_CLANGXX)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -Werror -Wextra")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS_WARNINGS}")
set(CMAKE_CXX_FLAGS_DEFAULT "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEFAULT "${CMAKE_C_FLAGS}")

# Library targets.
add_library(corgi ${corgi_SRCS})

if (corgi_build_component_library)
  set(CORGI_COMPONENT_LIBRARY_DIR ${CMAKE_CURRENT_LIST_DIR}/component_library)
  # The component library uses a bunch of Flatbuffers, so we should build them.
  # Generate source files for all FlatBuffers schema files under the src
  # directory.
  if(NOT TARGET flatc)
    set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")
    add_subdirectory("${dependencies_flatbuffers_dir}" ${tmp_dir}/flatbuffers)
  endif()

  set(CORGI_COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR
      ${CORGI_COMPONENT_LIBRARY_DIR}/${CMAKE_FILES_DIRECTORY}/include)
  file(GLOB_RECURSE
       CORGI_COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS
       ${CORGI_COMPONENT_LIBRARY_DIR}/schemas/*.fbs)

  # Generate rules to build the set of output files from the set of input
  # schema files.
  set(corgi_component_library_flatbuffer_dependencies
      "${dependencies_fplbase_dir}/schemas")
  build_flatbuffers(
     "${CORGI_COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS}"
     "${corgi_component_library_flatbuffer_dependencies}"
     corgi_component_library_generated_includes
     ""
     "${CORGI_COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR}"
     ""
     "")

  # Include dependencies for the component library.
  if(NOT TARGET breadboard)
    add_subdirectory("${dependencies_breadboard_dir}" ${tmp_dir}/breadboard)
  endif()

  if(NOT TARGET fplbase)
    add_subdirectory("${dependencies_fplbase_dir}" ${tmp_dir}/fplbase)
  endif()

  if(NOT TARGET LinearMath)
    set_compiler_flags_for_external_libraries()
    set(BUILD_CPU_DEMOS OFF CACHE BOOL "")
    set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "")
    set(BUILD_BULLET3 OFF CACHE BOOL "")
    set(BUILD_EXTRAS OFF CACHE BOOL "")
    set(BUILD_UNIT_TESTS OFF CACHE BOOL "")
    set(USE_MSVC_RUNTIME_LIBRARY_DLL ON)
    add_subdirectory("${dependencies_bulletphysics_distr_dir}"
                     ${tmp_dir}/bulletphysics)
    restore_compiler_flags()
  endif()

  set(mathfu_build_benchmarks OFF CACHE BOOL "")
  set(mathfu_build_tests OFF CACHE BOOL "")
  add_subdirectory(${dependencies_mathfu_dir} ${tmp_dir}/mathfu)

  if (NOT TARGET motive)
    set(motive_build_samples OFF CACHE BOOL "")
    set(motive_build_tests OFF CACHE BOOL "")
    add_subdirectory("${dependencies_motive_dir}" ${tmp_dir}/motive)
  endif()

  # component library source files.
  set(CORGI_COMPONENT_LIBRARY_INCLUDE_DIR
      ${CORGI_COMPONENT_LIBRARY_DIR}/include/corgi_component_library)
  set(corgi_component_library_SRCS
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/animation.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/camera_interface.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/common_services.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/component_utils.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/entity_factory.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/graph.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/meta.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/physics.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/rendermesh.h
      ${CORGI_COMPONENT_LIBRARY_INCLUDE_DIR}/transform.h
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/animation.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/common_services.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/component_utils.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/entity_factory.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/graph.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/meta.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/physics.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/rendermesh.cpp
      ${CORGI_COMPONENT_LIBRARY_DIR}/src/transform.cpp)

  include_directories(${CORGI_COMPONENT_LIBRARY_DIR}/include)
  get_property(FPLBASE_FLATBUFFERS_GENERATED_INCLUDES_DIR
               TARGET fplbase_generated_includes
               PROPERTY GENERATED_INCLUDES_DIR)
  include_directories(${FPLBASE_FLATBUFFERS_GENERATED_INCLUDES_DIR})
  include_directories(${dependencies_bulletphysics_distr_dir}/src)
  include_directories(${dependencies_corgi_dir}/include)
  include_directories(${dependencies_breadboard_dir}/include)
  include_directories(${dependencies_flatbuffers_dir}/include)
  include_directories(${dependencies_fplbase_dir}/include)
  include_directories(${dependencies_fplutil_dir}/libfplutil/include)
  include_directories(${dependencies_motive_dir}/include)
  include_directories(${dependencies_mathfu_dir}/include)

  set_compiler_flags_for_external_libraries()
  add_library(corgi_component_library ${corgi_component_library_SRCS})
  add_dependencies(corgi_component_library
                   corgi
                   corgi_component_library_generated_includes
                   fplbase_generated_includes)
  target_link_libraries(corgi_component_library LINK_INTERFACE_LIBRARIES
                        BulletDynamics BulletCollision LinearMath)
  mathfu_configure_flags(corgi_component_library)
  restore_compiler_flags()
endif()  # corgi_build_component_library

if(corgi_build_samples)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/sample)
endif()

# gtest seems to prefer the non-DLL runtime on Windows, which conflicts with
# everything else.
option(
  gtest_force_shared_crt
  "Use shared (DLL) run-time lib even when Google Test is built as static lib."
  ON)

# Tests.
if(corgi_build_tests)
  set(GUNIT_INCDIR "${dependencies_gtest_dir}/include")
  set(GTEST_LIBDIR "${dependencies_gtest_dir}")
  add_subdirectory("${dependencies_gtest_dir}" ${tmp_dir}/googletest)

  include(${GTEST_LIBDIR}/cmake/internal_utils.cmake)
  config_compiler_and_linker()
  string(REPLACE "-W4" "-W3" cxx_default "${cxx_default}")
  string(REPLACE "-Wshadow" "" cxx_default "${cxx_default}")
  string(REPLACE "-Wextra" "" cxx_default "${cxx_default}")

  include_directories(${GUNIT_INCDIR}
                      ${CMAKE_CURRENT_SOURCE_DIR})

  function(test_executable name libs)
    cxx_executable_with_flags(${name}_test "${cxx_default}"
        "${libs}" ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests/${name}_test.cpp
        ${ARGN})
    mathfu_configure_flags(${name}_test)
  endfunction()

  test_executable(vector_pool "gtest")
endif()


