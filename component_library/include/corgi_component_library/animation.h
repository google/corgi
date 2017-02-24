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

#ifndef CORGI_COMPONENT_LIBRARY_ANIMATION_H_
#define CORGI_COMPONENT_LIBRARY_ANIMATION_H_

#include "breadboard/event.h"
#include "corgi/component.h"
#include "library_components_generated.h"
#include "motive/anim_table.h"
#include "motive/engine.h"
#include "motive/motivator.h"

/// @brief Namespace for Motive library.
namespace motive {
class RigAnim;
}

/// @brief Namespace for CORGI library.
namespace corgi {
namespace component_library {

BREADBOARD_DECLARE_EVENT(kAnimationCompleteEventId)

/// @file
/// @addtogroup corgi_component_library
/// @{

/// @struct AnimationData
///
/// @brief The Component data structure that corresponds to each Entity that
/// is registered with the AnimationComponent. It contains all the information
/// for each Entity's animation.
struct AnimationData {
  AnimationData()
      : anim_table_object(-1),
        last_anim_idx(-1),
        previous_time_remaining(motive::kMotiveTimeEndless),
        debug_state(AnimationDebugState_None),
        debug_bone(motive::kInvalidBoneIdx) {}

  /// @var motivator
  ///
  /// @brief Holds and processes the animaton. Calls can be made to
  /// `motivator.GlobalTransforms()` to get an array of matrices: one for
  /// each bone in the animation.
  motive::RigMotivator motivator;

  /// @var anim_table_object
  ///
  /// @brief Specifies the object-type query parameter for the AnimTable,
  /// which is used to index the first array of the AnimTable.
  int anim_table_object;

  /// @var last_anim_idx
  ///
  /// @brief Tracks the index of the last animation that was played by
  /// `AnimateFromTable()`.
  int last_anim_idx;

  /// @var previous_time_remaining
  ///
  /// @brief The amount of time remaning from the previous animation. This
  /// can be used for firing animation events.
  motive::MotiveTime previous_time_remaining;

  /// @var debug_state
  ///
  /// @brief The debug information output to the log every frame, for this
  /// animating object. You probably want this to be None for all but one
  /// animating object, since it outputs a lot of data.
  AnimationDebugState debug_state;

  /// @var debug_bone
  ///
  /// @brief When `debug_state` outputs bone-specific information, this
  /// member specifies the bone. Ignored otherwise.
  motive::BoneIndex debug_bone;
};

/// @class AnimationComponent
///
/// @brief A Component that provides an elegant way to handle Entity animation
/// by interacting with the Motive animation library.
class AnimationComponent : public Component<AnimationData> {
 public:
  /// @brief Deconstructor of the Animation component.
  virtual ~AnimationComponent() {}

  /// @brief Updates all Motivators in the AnimationData, as well as, any other
  /// Motivators that were initialized with AnimationComponent's MotiveEngine.
  ///
  /// @param[in] delta_time An corgi::WorldTime representing the delta time
  /// since the last call to UpdateAllEntities.
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  /// @brief Deserialize a flat binary buffer to create and populate an Entity
  /// from raw data.
  ///
  /// @param[in,out] entity An EntityRef reference that points to an Entity that
  /// is being added from the raw data.
  /// @param[in] data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* data);

  /// @brief Serializes an AnimationComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to an Entity whose corresponding
  /// AnimationData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a
  /// flat binary buffer.
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  /// @brief Begin playback of a given RigAnim on a given Entity.
  ///
  /// @param[in] entity A const EntityRef reference to an Entity that the
  /// RigAnim `anim` should be applied on.
  ///
  /// @warning This method asserts that `entity` must also have a
  /// RenderMeshComponent in order for the animation to be applied.
  ///
  /// @param[in] anim A const motive::RigAnim reference to the animation that
  /// should be applied on `entity`.
  void Animate(const corgi::EntityRef& entity, const motive::RigAnim& anim);

  /// @brief Query the AnimTable for an animation and then play it.
  ///
  /// @param[in] entity A const EntityRef reference to the Entity whose
  /// corresponding animation should be started.
  /// @param[in] anim_idx An int index of the animation that should be
  /// started.
  ///
  /// @return Returns `true` if a new animation was successfully started.
  /// Otherwise, it returns `false` if the animation could not be found in
  /// the AnimTable.
  bool AnimateFromTable(const corgi::EntityRef& entity, int anim_idx);

  /// @brief Check if a certain animation exists with a given index in the
  /// AnimTable for a given Entity.
  ///
  /// @param[in] entity A const EntityRef reference to the Entity whose
  /// corresponding animation should be checked.
  /// @param[in] anim_idx An int index of the animation whose existence should
  /// be checked.
  ///
  /// @return Returns `true` if the animation exists in the AnimTable with
  /// `anim_idx`. Returns `false` if the animation is not found in the
  /// AnimTable.
  bool HasAnim(const corgi::EntityRef& entity, int anim_idx) const {
    const AnimationData* data = Data<AnimationData>(entity);
    return anim_table_.Query(data->anim_table_object, anim_idx) != nullptr;
  }

  /// @brief Get the length of an Entity's animation, given an animation index.
  ///
  /// @param[in] entity A const EntityRef reference to the Entity whose
  /// corresponding animation's length should be returned.
  /// @param[in] anim_idx An int index of the animation whose length should be
  /// returned.
  ///
  /// @return Returns the length of the animation, if it exists. Otherwise, it
  /// returns 0.
  motive::MotiveTime AnimLength(const corgi::EntityRef& entity,
                                int anim_idx) const {
    const AnimationData* data = Data<AnimationData>(entity);
    const motive::RigAnim* anim =
        anim_table_.Query(data->anim_table_object, anim_idx);
    return anim == nullptr ? 0 : anim->end_time();
  }

  /// @brief Get the index of the last animation that was played.
  ///
  /// @param[in] entity A const EntityRef reference to the entity whose index
  /// should be returned.
  ///
  /// @return Returns the index of the last animation that was played via
  /// `AnimateFromTable()`, or returns -1 if no animation has ever been played
  /// using that function.
  int LastAnimIdx(const corgi::EntityRef& entity) const {
    return Data<AnimationData>(entity)->last_anim_idx;
  }

  /// @brief Get a reference to the engine that can be used for external
  /// Motivators as well. For the greatest efficiency, there should only be one
  /// MotiveEngine.
  ///
  /// @return Returns a reference to the MotiveEngine for this
  /// AnimationComponent.
  motive::MotiveEngine& engine() { return engine_; }

  /// @brief Get a const reference to the engine that can be used for
  /// external Motivators as well. For the greatest efficiency,
  /// there should only be one MotiveEngine.
  ///
  /// @return Returns a const reference to the MotiveEngine for this
  /// AnimationComponent.
  const motive::MotiveEngine& engine() const { return engine_; }

  /// @brief Get a reference to the animation table that is queried when
  /// `AnimateFromTable()` is called.
  ///
  /// @return Returns a reference to the AnimTable for this AnimationComponent.
  motive::AnimTable& anim_table() { return anim_table_; }

  /// @brief Get a const reference to the animation table that is queried when
  /// `AnimateFromTable()` is called.
  ///
  /// @return Returns a const reference to the AnimTable for this
  /// AnimationComponent.
  const motive::AnimTable& anim_table() const { return anim_table_; }

 private:
  /// @brief Sets the motivator up for any of the animations on the `object`.
  void InitializeMotivator(const corgi::EntityRef& entity);

  /// @var engine_
  ///
  /// @brief Holds MotiveProcessors that, in turn, hold the animation state.
  /// Calling AdvanceFrame() on this updates all the animations at once.
  motive::MotiveEngine engine_;

  /// @var anim_table_
  ///
  /// @brief Holds a table of animations.
  /// It is a simple array of arrays. The top array is indexed by
  /// `anim_table_object` in `AnimationData`. The bottom array is indexed by
  /// `anim_idx` in the `AnimateFromTable()` call.
  motive::AnimTable anim_table_;
};
/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::AnimationComponent,
                         corgi::component_library::AnimationData)

#endif  // CORGI_COMPONENT_LIBRARY_ANIMATION_H_
