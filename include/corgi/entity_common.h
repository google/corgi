// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stddef.h>
#include <stdint.h>

#ifndef CORGI_ENTITY_COMMON_H_
#define CORGI_ENTITY_COMMON_H_

namespace corgi {

/// @file
/// @def CORGI_MAX_COMPONENT_COUNT
///
/// @brief The maximum number of components in the system.
///
/// Redefine this to fit however many Components your system needs.
///
/// @warning This should be set with care, because it affects the size of each
/// Entity. Ideally this should be set to something that is as close as
/// possible to the actual number of Components used by the program.
#define CORGI_MAX_COMPONENT_COUNT 30

/// @var ComponentId
///
/// @brief This represents the ID of a Component.
///
/// @note Depending on kMaxComponentCount, this value is either an uint8_t,
/// uint16_t, or uint32_t. (However, if you need an uint32_t, you are
/// probably doing something odd.)
/// @{
#if kMaxComponentCount <= 0xff
typedef uint8_t ComponentId;
#elif kMaxComponentCount <= 0xffff
typedef uint16_t ComponentId;
#else
typedef uint32_t ComponentId;
#endif
/// @}

/// @var kInvalidComponent
///
/// @brief A sentinel value to represent an invalid Component.
///
/// @note Component IDs start at 1.
const ComponentId kInvalidComponent = 0;

/// @cond CORGI_INTERNAL
static const ComponentId kMaxComponentCount = CORGI_MAX_COMPONENT_COUNT;
/// @endcond

/// @typedef WorldTime
///
/// @brief A typedef that represents time in the game.
typedef int WorldTime;

/// @cond CORGI_INTERNAL
const int kMillisecondsPerSecond = 1000;
/// @endcond

/// @typedef ComponentIndex
///
/// @brief A ComponentIndex is a value used to represent the location of a piece
/// of ComponentData, normally inside of a VectorPool.
typedef uint16_t ComponentIndex;

/// @var kUnusedComponentIndex
///
/// @brief A sentinel value to represent an invalid Component.
///
/// Since all Entities contain an array corresponding to every
/// Component in the system, this value is used as a default value
/// to indicate that a specific Component is not registered with
/// a given Entity.
static const ComponentIndex kUnusedComponentIndex =
    static_cast<ComponentIndex>(-1);

}  // corgi

#endif  // CORGI_ENTITY_COMMON_H_
