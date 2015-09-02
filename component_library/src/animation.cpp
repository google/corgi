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

#include "component_library/animation.h"
#include "component_library/rendermesh.h"
#include "motive/anim.h"
#include "motive/init.h"

FPL_ENTITY_DEFINE_COMPONENT(fpl::component_library::AnimationComponent,
                            fpl::component_library::AnimationData)

using fpl::entity::EntityRef;
using motive::RigAnim;
using motive::RigInit;

namespace fpl {
namespace component_library {

void AnimationComponent::Animate(const EntityRef& entity, const RigAnim& anim) {
  AnimationData* data = Data<AnimationData>(entity);
  RenderMeshData* render_data = Data<RenderMeshData>(entity);
  assert(data != nullptr && render_data != nullptr);

  // Initialize the RigMotivator to animate the `mesh` according to `anim`.
  const Mesh* mesh = render_data->mesh;
  const RigInit init(anim, mesh->bone_transforms(), mesh->bone_parents(),
                     mesh->num_bones());
  data->motivator.Initialize(init, &engine_);
}

}  // component_library
}  // fpl