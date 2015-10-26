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
#include "component_library/graph.h"
#include "component_library/meta.h"
#include "component_library/rendermesh.h"
#include "motive/anim.h"
#include "motive/init.h"

FPL_ENTITY_DEFINE_COMPONENT(fpl::component_library::AnimationComponent,
                            fpl::component_library::AnimationData)

BREADBOARD_DEFINE_EVENT(fpl::component_library::kAnimationCompleteEventId)

using fpl::entity::EntityRef;
using motive::MotiveTime;
using motive::RigAnim;
using motive::RigInit;

namespace fpl {
namespace component_library {

void AnimationComponent::UpdateAllEntities(entity::WorldTime delta_time) {
  // Pre-update loop.
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    AnimationData* animation_data = GetComponentData(iter->entity);
    if (animation_data->motivator.Valid()) {
      // Log debug info. Only log the header the first time.
      if (animation_data->debug_state == kAnimationDebug_OutputHeaderAndState) {
        LogInfo(animation_data->motivator.CsvHeaderForDebugging().c_str());
        animation_data->debug_state = kAnimationDebug_OutputState;
      }
      if (animation_data->debug_state != kAnimationDebug_Inactive) {
        LogInfo(animation_data->motivator.CsvValuesForDebugging().c_str());
      }
    }
  }

  engine_.AdvanceFrame(delta_time);

  // Post-update loop.
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    AnimationData* animation_data = GetComponentData(iter->entity);
    if (animation_data->motivator.Valid()) {
      // Broadcast when animations complete.
      MotiveTime time_remaining = animation_data->motivator.TimeRemaining();
      if (time_remaining <= 0 && animation_data->previous_time_remaining > 0) {
        GraphData* graph_data = Data<GraphData>(iter->entity);
        if (graph_data) {
          graph_data->broadcaster.BroadcastEvent(kAnimationCompleteEventId);
        }
      }
      animation_data->previous_time_remaining = time_remaining;
    }
  }
}

void AnimationComponent::AddFromRawData(entity::EntityRef& entity,
                                        const void* raw_data) {
  auto animation_def = static_cast<const AnimationDef*>(raw_data);
  AnimationData* animation_data = AddEntity(entity);
  animation_data->anim_table_object = animation_def->anim_table_object();
  animation_data->debug_state =
      animation_def->debug() ? kAnimationDebug_OutputHeaderAndState
                             : kAnimationDebug_Inactive;
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

  auto anim_def = CreateAnimationDef(
      fbb, animation_data->anim_table_object, 0,
      animation_data->debug_state != kAnimationDebug_Inactive);
  fbb.Finish(anim_def);
  return fbb.ReleaseBufferPointer();
}

void AnimationComponent::InitializeMotivator(const EntityRef& entity) {
  AnimationData* data = Data<AnimationData>(entity);
  RenderMeshData* render_data = Data<RenderMeshData>(entity);
  assert(data != nullptr && render_data != nullptr);

  // Remember the entry in the anim table for this entity.
  const motive::RigAnim& defining_anim =
      anim_table_.DefiningAnim(data->anim_table_object);

  // Initialize the RigMotivator to animate the `mesh` according to
  // `defining_anim`.
  const Mesh* mesh = render_data->mesh;
  const RigInit init(defining_anim, mesh->bone_transforms(),
                     mesh->bone_parents(), mesh->num_bones());
  data->motivator.Initialize(init, &engine_);
}

void AnimationComponent::Animate(const EntityRef& entity, const RigAnim& anim) {
  AnimationData* data = Data<AnimationData>(entity);

  // We initialize the rig motivator only once, using the defining_anim so that
  // it can play back any animation for this object in the `anim_table_`.
  if (!data->motivator.Valid()) {
    InitializeMotivator(entity);
  }

  // Instead of reinitializing the rig motivator, we perform a smooth transition
  // from the current state to the new animation `anim`.
  data->motivator.BlendToAnim(anim);
}

bool AnimationComponent::AnimateFromTable(const entity::EntityRef& entity,
                                          int anim_idx) {
  const AnimationData* data = Data<AnimationData>(entity);
  const motive::RigAnim* anim =
      anim_table_.Query(data->anim_table_object, anim_idx);
  if (anim == nullptr) return false;

  Animate(entity, *anim);
  return true;
}

}  // component_library
}  // fpl
