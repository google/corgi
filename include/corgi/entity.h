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

#ifndef CORGI_ENTITY_H_
#define CORGI_ENTITY_H_

#include "corgi/entity_common.h"

namespace corgi {

/// @file
/// @addtogroup corgi_entity
/// @{
///
/// @class Entity
///
/// @brief The basic Entity class for CORGI. It contains an array
/// of index values, which are used by Components for tracking their data
/// associated with an Entity. It also contains a boolean for tracking if
/// this Entity has been marked for deletion.
class Entity {
 public:
  /// @brief Constructor for creating an Entity.
  Entity() : marked_for_deletion_(false) {
    for (int i = 0; i < kMaxComponentCount; i++) {
      componentDataIndex_[i] = kUnusedComponentIndex;
    }
  }

  /// @brief Gets the index of the data in the corresponding Component
  /// system.
  ///
  /// @param[in] componentId The ID of the Component whose data
  /// index will be returned.
  ///
  /// @return Returns an int index of the data in the corresponding
  /// Component system.
  int GetComponentDataIndex(ComponentId componentId) const {
    return componentDataIndex_[componentId];
  }

  /// @brief Sets the index for the data associated with this Entity in the
  /// given Component.
  ///
  /// @param[in] componentId The ID of the Component whose data index will
  /// be set.
  /// @param[in] value The ComponentIndex value to set for the given Component.
  void SetComponentDataIndex(ComponentId componentId, ComponentIndex value) {
    componentDataIndex_[componentId] = value;
  }

  /// @brief A utility function for checking if this Entity is associated
  /// with a given Component.
  ///
  /// @param[in] componentId The ID of the Component to check whether or not
  /// it is associated with this Entity.
  ///
  /// @return Returns a bool of whether or not this Entity is associated
  /// with the given Component.
  bool IsRegisteredForComponent(ComponentId componentId) const {
    return componentDataIndex_[componentId] != kUnusedComponentIndex;
  }

  /// @brief An accessor function to check if this Entity has been
  /// marked for deletion.
  ///
  /// @return Returns a bool indicating whether or not this Entity
  /// is marked for deletion.
  bool marked_for_deletion() const { return marked_for_deletion_; }

  /// @brief A mutator function to set an Entity as marked for deletion.
  ///
  /// @param[in] marked_for_deletion A bool corresponding to whether the Entity
  /// should be marked for deletion or not.
  void set_marked_for_deletion(bool marked_for_deletion) {
    marked_for_deletion_ = marked_for_deletion;
  }

 private:
  ComponentIndex componentDataIndex_[kMaxComponentCount];
  bool marked_for_deletion_;
};
/// @}

}  // corgi

#endif  // CORGI_ENTITY_H_
