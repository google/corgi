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

#ifndef COMPONENT_LIBRARY_ANIMATION_H_
#define COMPONENT_LIBRARY_ANIMATION_H_

#include "entity/component.h"
#include "motive/anim_table.h"
#include "motive/motivator.h"
#include "motive/engine.h"

namespace motive {
class RigAnim;
}

namespace fpl {
namespace component_library {

struct AnimationData;

struct AnimationData {
  // Holds and processes the animation. Call motivator.GlobalTransforms()
  // to get an array of matrices: one for each bone in the animation.
  motive::RigMotivator motivator;

  // If animation is set using the AnimationComponent's anim_table_,
  // specifies the query parameters for the anim_table_.
  int anim_table_object;

  AnimationData() : anim_table_object(-1) {}
};

class AnimationComponent : public entity::Component<AnimationData> {
 public:
  // Update all motivators in AnimationData, and any other motivators that
  // were initialized with `engine_`.
  virtual void UpdateAllEntities(entity::WorldTime delta_time) {
    engine_.AdvanceFrame(delta_time);
  }

  // Serialize and deserialize.
  virtual void AddFromRawData(entity::EntityRef& entity, const void* data);
  virtual RawDataUniquePtr ExportRawData(const entity::EntityRef& entity) const;

  // Begin playback of `anim` on `entity`.
  // Note that `entity` must also have a RenderMeshComponent in order for
  // this animation to be applied.
  void Animate(const entity::EntityRef& entity, const motive::RigAnim& anim);

  // Query the anim_table for an animation, then play it.
  void AnimateFromTable(const entity::EntityRef& entity, int anim_idx);

  // Return true if an animation exists in the AnimTable for `anim_idx`.
  bool HasAnim(const entity::EntityRef& entity, int anim_idx) const {
    const AnimationData* data = Data<AnimationData>(entity);
    return anim_table_.Query(data->anim_table_object, anim_idx) != nullptr;
  }

  // The engine can be used for external Motivators as well.
  // For the greatest efficiency, there should be only one MotiveEngine.
  motive::MotiveEngine& engine() { return engine_; }
  const motive::MotiveEngine& engine() const { return engine_; }

  // The animation table is queried when AnimateFromTable() is called.
  motive::AnimTable& anim_table() { return anim_table_; }
  const motive::AnimTable& anim_table() const { return anim_table_; }

 private:
  // Holds MotiveProcessors that, in turn, hold the animation state.
  // Calling AdvanceFrame() on this updates all the animations at once.
  motive::MotiveEngine engine_;

  // Holds a table of animations.
  // AnimTable is a simple array of arrays. Top array is indexed by
  // `anim_table_object` in AnimationData. Bottom array is indexed
  // by `anim_idx` in the AnimateFromTable() call.
  motive::AnimTable anim_table_;
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::AnimationComponent,
                              fpl::component_library::AnimationData)

#endif  // COMPONENT_LIBRARY_ANIMATION_H_
