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
#include "motive/motivator.h"
#include "motive/engine.h"

namespace motive {
class RigAnim;
}

namespace fpl {
namespace component_library {

struct AnimationData;

struct AnimationData {
  motive::MotivatorMatrix4f motivator;
};

class AnimationComponent : public entity::Component<AnimationData> {
 public:
  // Update all motivators in AnimationData, and any other motivators that
  // were initialized with `engine_`.
  virtual void UpdateAllEntities(entity::WorldTime delta_time) {
    engine_.AdvanceFrame(delta_time);
  }

  // Nothing to serialize or deserialize.
  virtual void AddFromRawData(entity::EntityRef& /*entity*/,
                              const void* /*data*/) {}

  // Begin playback of `anim` on `entity`.
  // Note that `entity` must also have a RenderMeshComponent in order for
  // this animation to be applied.
  void Animate(const entity::EntityRef& entity, const motive::RigAnim& anim);

  // The engine can be used for external Motivators as well.
  // For the greatest efficiency, there should be only one MotiveEngine.
  motive::MotiveEngine& engine() { return engine_; }
  const motive::MotiveEngine& engine() const { return engine_; }

 private:
  // Holds MotiveProcessors that, in turn, hold the animation state.
  // Calling AdvanceFrame() on this updates all the animations at once.
  motive::MotiveEngine engine_;
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::AnimationComponent,
                              fpl::component_library::AnimationData)

#endif  // COMPONENT_LIBRARY_ANIMATION_H_
