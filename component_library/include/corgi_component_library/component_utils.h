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

#ifndef CORGI_COMPONENT_LIBRARY_COMPONENT_UTILS_H_
#define CORGI_COMPONENT_LIBRARY_COMPONENT_UTILS_H_

#include <cfloat>
#include "corgi/component.h"
#include "mathfu/glsl_mappings.h"

namespace corgi {
namespace component_library {
/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @fn bool GetMaxMinPositionsForEntity(corgi::EntityRef& entity,
/// corgi::EntityManager& entity_manager, mathfu::vec3* max,
/// mathfu::vec3* min)
///
/// @brief Get the minimum and maximum positions of an Entity, based on its
/// and its children's rendermeshes.
///
/// @param[in] entity An EntityRef reference to the Entity whose minimum and
/// maximum positions should be returned through the `min` and `max` parameters.
/// @param[in] entity_manager An EntityManager reference to the EntityManager
/// reponsible for managing this Entity.
/// @param[out] max A mathfu::vec3 pointer that is used to capture the output
/// value of the maximum Entity position.
/// @param[out] min A mathfu::vec3 pointer that is used to capture the output
/// value of the minimum Entity position.
///
/// @return Returns `true` if `min` and `max` were set successfully. Otherwise
/// it returns false.
bool GetMaxMinPositionsForEntity(corgi::EntityRef& entity,
                                 corgi::EntityManager& entity_manager,
                                 mathfu::vec3* max, mathfu::vec3* min);
/// @}

}  // component_library
}  // corgi

#endif  // CORGI_COMPONENT_LIBRARY_COMPONENT_UTILS_H_
