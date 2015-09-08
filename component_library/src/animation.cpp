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
#include "component_library/common_services.h"
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

void AnimationComponent::AddFromRawData(entity::EntityRef& entity,
                                        const void* raw_data) {
  auto animation_def = static_cast<const AnimationDef*>(raw_data);
  AnimationData* animation_data = AddEntity(entity);
  animation_data->anim_table_object = animation_def->anim_table_object();
  AnimateFromTable(entity, animation_def->anim_table_start_idx());
}

entity::ComponentInterface::RawDataUniquePtr AnimationComponent::ExportRawData(
    const entity::EntityRef& entity) const {
  const AnimationData* animation_data = GetComponentData(entity);
  if (animation_data == nullptr) return nullptr;

  flatbuffers::FlatBufferBuilder fbb;
  bool defaults = entity_manager_->GetComponent<CommonServicesComponent>()
                      ->export_force_defaults();
  fbb.ForceDefaults(defaults);

  auto anim_def = CreateAnimationDef(fbb, animation_data->anim_table_object, 0);
  fbb.Finish(anim_def);
  return fbb.ReleaseBufferPointer();
}

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

void AnimationComponent::AnimateFromTable(const entity::EntityRef& entity,
                                          int anim_idx) {
  const AnimationData* data = Data<AnimationData>(entity);
  const motive::RigAnim* anim =
      anim_table_.Query(data->anim_table_object, anim_idx);
  assert(anim != nullptr);
  Animate(entity, *anim);
}

}  // component_library
}  // fpl
