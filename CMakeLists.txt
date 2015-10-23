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

project(entity)

option(entity_build_component_library
  "Build a library of standard components along with the entity system"
  ON)

# Compile the game with the debug flag
set(entity_DEBUG ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Directory which contains the source for 3rd party libraries.
if(NOT DEFINED third_party_root)
  get_filename_component(
    third_party_root "${CMAKE_CURRENT_SOURCE_DIR}/../../../../external"
    REALPATH)
endif()

# Directory which contains source for FPL libraries.
if(NOT DEFINED fpl_root)
  get_filename_component(
      fpl_root "${CMAKE_CURRENT_SOURCE_DIR}/../../libs" REALPATH)
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
set(dependencies_pindrop_dir "${fpl_root}/pindrop"
    CACHE PATH "Directory containing the pindrop library.")
set(dependencies_fplbase_dir "${fpl_root}/fplbase"
    CACHE PATH "Directory containing the FPLBase library.")
set(dependencies_entity_dir "${fpl_root}/entity"
    CACHE PATH "Directory containing the Entity library.")
set(dependencies_mathfu_dir "${fpl_root}/mathfu"
    CACHE PATH "Directory containing the MathFu library.")
set(dependencies_motive_dir "${fpl_root}/motive"
    CACHE PATH "Directory containing the motive animation library.")
set(dependencies_bulletphysics_distr_dir "${third_party_root}/bulletphysics"
    CACHE PATH "Directory containing the Bullet Physics distribution.")

# Temporary files (like object files) created while compiling projects.
set(tmp_dir ${CMAKE_CURRENT_BINARY_DIR}/obj)

# entity source files.
set(entity_SRCS
    include/entity/component.h
    include/entity/component_id_lookup.h
    include/entity/component_interface.h
    include/entity/entity_common.h
    include/entity/entity_manager.h
    include/entity/entity.h
    include/entity/vector_pool.h
    src/entity_manager.cpp)

# Includes for this project.
include_directories(src include)

# Detect clang
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
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
add_library(entity ${entity_SRCS})

if (entity_build_component_library)
  set(COMPONENT_COLLECTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/component_library)
  # The component library uses a bunch of Flatbuffers, so we should build them.
  # Generate source files for all FlatBuffers schema files under the src
  # directory.
  set(COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR
     ${COMPONENT_COLLECTION_DIR}/${CMAKE_FILES_DIRECTORY}/include)
  file(GLOB_RECURSE
    COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS
    ${COMPONENT_COLLECTION_DIR}/schemas/*.fbs)

  # Generate rules to build the set of output files from the set of input
  # schema files.
  unset(COMPONENT_LIBRARY_GENERATED_INCLUDES)
  foreach(flatbuffers_schema ${COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS})
    get_filename_component(filename ${flatbuffers_schema} NAME_WE)
    set(flatbuffers_include
      ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR}/${filename}_generated.h)
    file(COPY ${flatbuffers_schema}
         DESTINATION ${CMAKE_BINARY_DIR}/assets/flatbufferschemas/)
    add_custom_command(
      OUTPUT ${flatbuffers_include}
      COMMAND flatc --gen-mutable
          -o ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR}
          -I ${dependencies_fplbase_dir}/schemas
          -c ${flatbuffers_schema}
      DEPENDS flatc ${flatbuffers_schema})
    list(APPEND COMPONENT_LIBRARY_GENERATED_INCLUDES ${flatbuffers_include})
  endforeach()
  add_custom_target(component_library_generated_includes
    DEPENDS ${COMPONENT_LIBRARY_GENERATED_INCLUDES})
  set_property(TARGET component_library_generated_includes
               PROPERTY COMPONENT_LIBRARY_DIR
               ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR})

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

  if(NOT TARGET pindrop)
    set(pindrop_build_demo OFF CACHE BOOL "")
    set(pindrop_build_tests OFF CACHE BOOL "")
    add_subdirectory("${dependencies_pindrop_dir}" ${tmp_dir}/pindrop)
  endif()

  # component library source files.
  set(component_library_SRCS
      ${COMPONENT_COLLECTION_DIR}/include/component_library/animation.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/camera_interface.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/common_services.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/component_utils.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/entity_factory.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/graph.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/meta.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/physics.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/rendermesh.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/transform.h
      ${COMPONENT_COLLECTION_DIR}/src/animation.cpp
      ${COMPONENT_COLLECTION_DIR}/src/common_services.cpp
      ${COMPONENT_COLLECTION_DIR}/src/component_utils.cpp
      ${COMPONENT_COLLECTION_DIR}/src/entity_factory.cpp
      ${COMPONENT_COLLECTION_DIR}/src/graph.cpp
      ${COMPONENT_COLLECTION_DIR}/src/meta.cpp
      ${COMPONENT_COLLECTION_DIR}/src/physics.cpp
      ${COMPONENT_COLLECTION_DIR}/src/rendermesh.cpp
      ${COMPONENT_COLLECTION_DIR}/src/transform.cpp)

  include_directories(${COMPONENT_COLLECTION_DIR}/include)
  get_property(FPLBASE_FLATBUFFERS_GENERATED_INCLUDES_DIR
               TARGET fplbase_generated_includes PROPERTY FPLBASE_DIR)
  include_directories(${FPLBASE_FLATBUFFERS_GENERATED_INCLUDES_DIR})
  include_directories(${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR})
  include_directories(${dependencies_bulletphysics_distr_dir}/src)
  include_directories(${dependencies_entity_dir}/include)
  include_directories(${dependencies_breadboard_dir}/include)
  include_directories(${dependencies_flatbuffers_dir}/include)
  include_directories(${dependencies_fplbase_dir}/include)
  include_directories(${dependencies_fplutil_dir}/libfplutil/include)
  include_directories(${dependencies_motive_dir}/include)
  include_directories(${dependencies_mathfu_dir}/include)
  include_directories(${dependencies_pindrop_dir}/include)

  set_compiler_flags_for_external_libraries()
  add_library(component_library ${component_library_SRCS})
  add_dependencies(component_library
                   entity
                   component_library_generated_includes
                   fplbase_generated_includes)
  target_link_libraries(component_library LINK_INTERFACE_LIBRARIES
                        BulletDynamics BulletCollision LinearMath)
  mathfu_configure_flags(component_library)
  restore_compiler_flags()
endif()  # entity_build_component_library
