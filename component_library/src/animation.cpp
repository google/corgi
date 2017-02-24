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

#include "corgi_component_library/animation.h"
#include "corgi_component_library/common_services.h"
#include "corgi_component_library/graph.h"
#include "corgi_component_library/meta.h"
#include "corgi_component_library/rendermesh.h"
#include "motive/anim.h"
#include "motive/init.h"

CORGI_DEFINE_COMPONENT(corgi::component_library::AnimationComponent,
                       corgi::component_library::AnimationData)

BREADBOARD_DEFINE_EVENT(corgi::component_library::kAnimationCompleteEventId)

using corgi::EntityRef;
using motive::MotiveTime;
using motive::RigAnim;
using motive::RigInit;

namespace corgi {
namespace component_library {

// TODO: Make these configurable per call, instead of const in the anim.
static const float kAnimStartTime = 0.0f;
static const float kAnimPlaybackRate = 1.0f;
static const float kAnimBlendTime = 200.0f;

void AnimationComponent::UpdateAllEntities(corgi::WorldTime delta_time) {
  // Pre-update loop.
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    AnimationData* animation_data = GetComponentData(iter->entity);
    if (animation_data->motivator.Valid()) {
      switch (animation_data->debug_state) {
        case AnimationDebugState_AllChannelsWithHeader:
          // Log debug info. Only log the header the first time.
          fplbase::LogInfo(
              animation_data->motivator.CsvHeaderForDebugging().c_str());
          animation_data->debug_state = AnimationDebugState_AllChannels;
          FPL_FALLTHROUGH_INTENDED

        case AnimationDebugState_AllChannels:
          fplbase::LogInfo(
              animation_data->motivator.CsvValuesForDebugging().c_str());
          break;

        case AnimationDebugState_OneBone:
          fplbase::LogInfo("\n%s",
              animation_data->motivator.LocalTransformsForDebugging(
                  animation_data->debug_bone).c_str());
          break;

        default: break;
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

void AnimationComponent::AddFromRawData(corgi::EntityRef& entity,
                                        const void* raw_data) {
  auto animation_def = static_cast<const corgi::AnimationDef*>(raw_data);
  AnimationData* animation_data = AddEntity(entity);
  animation_data->anim_table_object = animation_def->anim_table_object();
  animation_data->debug_state = animation_def->debug_state();
  animation_data->debug_bone = animation_def->debug_bone();
  if (animation_def->anim_table_start_idx() >= 0) {
    AnimateFromTable(entity, animation_def->anim_table_start_idx());
  }
}

corgi::ComponentInterface::RawDataUniquePtr AnimationComponent::ExportRawData(
    const corgi::EntityRef& entity) const {
  const AnimationData* animation_data = GetComponentData(entity);
  if (animation_data == nullptr) return nullptr;

  flatbuffers::FlatBufferBuilder fbb;
  bool defaults = entity_manager_->GetComponent<CommonServicesComponent>()
                      ->export_force_defaults();
  fbb.ForceDefaults(defaults);

  auto anim_def = CreateAnimationDef(
      fbb, animation_data->anim_table_object, 0, animation_data->debug_state,
      animation_data->debug_bone);
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
  auto mesh = render_data->mesh;
  const RigInit init(defining_anim, mesh->bone_parents(),
                     static_cast<motive::BoneIndex>(mesh->num_bones()));
  data->motivator.Initialize(init, &engine_);
}

void AnimationComponent::Animate(const EntityRef& entity, const RigAnim& anim) {
  AnimationData* data = Data<AnimationData>(entity);
  motive::SplinePlayback playback(kAnimStartTime, anim.repeat(),
                                  kAnimPlaybackRate, kAnimBlendTime);

  // We initialize the rig motivator only once, using the defining_anim so that
  // it can play back any animation for this object in the `anim_table_`.
  if (!data->motivator.Valid()) {
    InitializeMotivator(entity);
    playback.blend_x = 0.0f;
  }

  // Instead of reinitializing the rig motivator, we perform a smooth transition
  // from the current state to the new animation `anim`.
  data->motivator.BlendToAnim(anim, playback);
}

bool AnimationComponent::AnimateFromTable(const corgi::EntityRef& entity,
                                          int anim_idx) {
  AnimationData* data = Data<AnimationData>(entity);
  const motive::RigAnim* anim =
      anim_table_.Query(data->anim_table_object, anim_idx);
  if (anim == nullptr) return false;

  Animate(entity, *anim);
  data->last_anim_idx = anim_idx;
  return true;
}

}  // component_library
}  // corgi
