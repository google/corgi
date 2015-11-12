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

#include <cfloat>

#include "corgi_component_library/component_utils.h"
#include "corgi_component_library/rendermesh.h"
#include "corgi_component_library/transform.h"

using mathfu::vec3;

namespace corgi {
namespace component_library {

bool GetMaxMinPositionsForEntity(corgi::EntityRef& entity,
                                 corgi::EntityManager& entity_manager,
                                 vec3* max, vec3* min) {
  bool found_mesh = false;
  auto transform_data = entity_manager.GetComponentData<TransformData>(entity);
  auto rendermesh_data =
      entity_manager.GetComponentData<RenderMeshData>(entity);
  if (rendermesh_data) {
    found_mesh = true;
    *max = vec3::Max(
        *max, vec3::HadamardProduct(rendermesh_data->mesh->max_position(),
                                    transform_data->scale));
    *min = vec3::Min(
        *min, vec3::HadamardProduct(rendermesh_data->mesh->min_position(),
                                    transform_data->scale));
  }
  // Check against child entities as well
  bool child_had_values = false;
  vec3 child_max(-FLT_MAX);
  vec3 child_min(FLT_MAX);
  for (auto iter = transform_data->children.begin();
       iter != transform_data->children.end(); ++iter) {
    child_had_values |= GetMaxMinPositionsForEntity(iter->owner, entity_manager,
                                                    &child_max, &child_min);
  }
  if (child_had_values) {
    *max = vec3::Max(*max,
                     vec3::HadamardProduct(child_max, transform_data->scale));
    *min = vec3::Min(*min,
                     vec3::HadamardProduct(child_min, transform_data->scale));
  }
  return found_mesh || child_had_values;
}

}  // component_library
}  // corgi
