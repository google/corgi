# Copyright (c) 2015 Google, Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty.  In no event will the authors be held liable for any damages
# arising from the use of this software.
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 1. The origin of this software must not be misrepresented; you must not
# claim that you wrote the original software. If you use this software
# in a product, an acknowledgment in the product documentation would be
# appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
# misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
cmake_minimum_required(VERSION 2.8.12)

set(CORGI_SAMPLE_BINARY_SRCS
    entity_component_system.cpp
    entity_component_system.h)

# This is the directory into which the executables are built.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/../bin)

add_executable(entity_component_system_sample ${CORGI_SAMPLE_BINARY_SRCS})

target_link_libraries(entity_component_system_sample corgi)
