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
  Entity() : entity_id_(kInvalidEntityId), marked_for_deletion_(false) {}

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


  /// @brief Returns the unique entity id that represents this entity.  Should
  /// generally only be used by internal CORGI functions.  Users of the library
  /// should usually refer to entities as EntityRefs.
  EntityIdType entity_id() const { return entity_id_; }

  /// @brief Sets an entity's unique ID.
  ///
  /// Normally only used internally by the entitymanager for bookkeeping.
  void set_entity_id(EntityIdType entity_id) { entity_id_ = entity_id; }

 private:
  EntityIdType entity_id_;
  bool marked_for_deletion_;
};
/// @}

}  // corgi

#endif  // CORGI_ENTITY_H_
