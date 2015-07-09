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
get_directory_property(entity_has_parent PARENT_DIRECTORY)

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
set(dependencies_event_dir "${fpl_root}/event"
    CACHE PATH "Directory containing the event library.")
set(dependencies_pindrop_dir "${fpl_root}/pindrop"
    CACHE PATH "Directory containing the pindrop library.")
set(dependencies_fplbase_dir "${fpl_root}/fplbase"
    CACHE PATH "Directory containing the FPLBase library.")
set(dependencies_entity_dir "${fpl_root}/entity"
    CACHE PATH "Directory containing the Entity library.")
set(dependencies_flatui_dir "${fpl_root}/imgui"
    CACHE PATH "Directory containing the FlatUI library.")
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
include_directories(src include src/entity include/entity)

if (entity_build_component_library)
  set(COMPONENT_COLLECTION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/component_library)
  # component library source files.
  set(component_library_SRCS
      ${COMPONENT_COLLECTION_DIR}/include/component_library/common_services.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/editor.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/entity_factory.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/physics.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/rendermesh.h
      ${COMPONENT_COLLECTION_DIR}/include/component_library/transform.h
      ${COMPONENT_COLLECTION_DIR}/src/common_services.cpp
      ${COMPONENT_COLLECTION_DIR}/src/editor.cpp
      ${COMPONENT_COLLECTION_DIR}/src/entity_factory.cpp
      ${COMPONENT_COLLECTION_DIR}/src/rendermesh.cpp
      ${COMPONENT_COLLECTION_DIR}/src/physics.cpp
      ${COMPONENT_COLLECTION_DIR}/src/transform.cpp
  )
  # The component library uses a bunch of Flatbuffers, so we should build them.
  # Generate source files for all FlatBuffers schema files under the src
  # directory.
  set(COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR
     ${COMPONENT_COLLECTION_DIR}/${CMAKE_FILES_DIRECTORY}/include)
  if(entity_has_parent)
    set(COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR
        ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR} PARENT_SCOPE)
  endif()
  file(GLOB_RECURSE
    COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS
    ${COMPONENT_COLLECTION_DIR}/schemas/*.fbs)
  
  # Generate rules to build the set of output files from the set of input
  # schema files.
  foreach(flatbuffers_schema ${COMPONENT_LIBRARY_FLATBUFFERS_SCHEMAS})
    get_filename_component(filename ${flatbuffers_schema} NAME_WE)
    set(flatbuffers_include
      ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR}/${filename}_generated.h)
    add_custom_command(
      OUTPUT ${flatbuffers_include}
      COMMAND flatc --gen-includes --gen-mutable
          -I ${dependencies_fplbase_dir}/schemas
          -o ${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR}
          -c ${flatbuffers_schema}
      DEPENDS flatc ${flatbuffers_schema})
    list(APPEND FLATBUFFERS_GENERATED_INCLUDES ${flatbuffers_include})
  endforeach()
  add_custom_target(component_library_generated_includes
    DEPENDS ${FLATBUFFERS_GENERATED_INCLUDES})

  include_directories(${COMPONENT_COLLECTION_DIR}/include)
  include_directories(${FPLBASE_FLATBUFFERS_GENERATED_INCLUDES_DIR})
  include_directories(${COMPONENT_LIBRARY_FLATBUFFERS_GENERATED_INCLUDES_DIR})
  include_directories(${dependencies_bulletphysics_distr_dir}/src)
  include_directories(${dependencies_entity_dir}/include)
  include_directories(${dependencies_event_dir}/include)
  include_directories(${dependencies_flatbuffers_dir}/include)
  include_directories(${dependencies_fplbase_dir}/include)
  include_directories(${dependencies_fplutil_dir}/libfplutil/include)
  include_directories(${dependencies_flatui_dir}/include)
  include_directories(${dependencies_motive_dir}/include)
  include_directories(${dependencies_mathfu_dir}/include)
  include_directories(${dependencies_pindrop_dir}/include)
endif()  # entity_build_component_library

# Detect clang
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif()

# Compiler flags.
set(C_FLAGS_WARNINGS "")
if(MSVC)
  set(C_FLAGS_WARNINGS "/W4 /WX /wd4065 /wd4355")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
       CMAKE_COMPILER_IS_CLANGXX)
  add_definitions(-g)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic -Werror -Wextra")
endif()
if(APPLE)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers -Wno-overloaded-virtual")
  # These additional flags are needed because of Bullet's btSolverBody.h and
  # btConvexHullShape.h causing warnings with the Apple build
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS_WARNINGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS_WARNINGS}")

# Library targets.
add_library(entity ${entity_SRCS})

if (entity_build_component_library)
  add_library(component_library ${component_library_SRCS})
  add_dependencies(component_library component_library_generated_includes)
  add_dependencies(component_library mathfu_generated_includes)
  add_dependencies(component_library fplbase_generated_includes)
  mathfu_configure_flags(component_library)
endif()  # entity_build_component_library

