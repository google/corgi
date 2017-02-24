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

#ifndef CORGI_COMPONENT_LIBRARY_BULLET_PHYSICS_H_
#define CORGI_COMPONENT_LIBRARY_BULLET_PHYSICS_H_

// Suppress warnings in the Bullet header files.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif  // defined(__clang__)

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif  // defined(__GNUC__)

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127)  // conditional expression is constant
#pragma warning(disable : 4065)  // switch statement contains 'default' but no
                                 // 'case' labels
#pragma warning(disable : 4244)  // implicit type conversion may lose data
#pragma warning(disable : 4511)  // couldn't generate a default copy-constructor
#pragma warning(disable : 4512)  // couldn't generate assignment operator
#pragma warning(disable : 4706)  // assignment statement inside conditional
                                 // expression
#pragma warning(disable : 4127)  // conditional expression is constant
#pragma warning(disable : 4100)  // parameter not referenced
#pragma warning(disable : 4189)  // variable initialized but not referenced
#pragma warning(disable : 4505)  // function is not referenced
#pragma warning(disable : 4702)  // unreachable code
#pragma warning(disable : 4305)  // truncation from 'double' to 'const btScalar'
#endif                           // defined(_MSC_VER)
#include "btBulletDynamicsCommon.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif  // defined(_MSC_VER)

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif  // defined(__GNUC__)

#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // defined(__clang__)

#endif  // CORGI_COMPONENT_LIBRARY_BULLET_PHYSICS_H_
