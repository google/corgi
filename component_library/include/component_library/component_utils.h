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

#ifndef COMPONENT_LIBRARY_COMPONENT_UTILS_H_
#define COMPONENT_LIBRARY_COMPONENT_UTILS_H_

#include <cfloat>
#include "entity/component.h"
#include "mathfu/glsl_mappings.h"

namespace fpl {
namespace component_library {

// Get the max and min positions of an entity, based on its rendermesh.
// Returns true if positions are gotten, and results stored in the provided
// max and min vectors.
bool GetMaxMinPositionsForEntity(entity::EntityRef& entity,
                                 entity::EntityManager& entity_manager,
                                 mathfu::vec3* max, mathfu::vec3* min);

}  // component_library
}  // fpl

#endif  // COMPONENT_LIBRARY_COMPONENT_UTILS_H_
