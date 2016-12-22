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

#ifndef CORGI_COMPONENT_LIBRARY_TRANSFORM_H_
#define CORGI_COMPONENT_LIBRARY_TRANSFORM_H_

#include <set>
#include <vector>

#include "corgi/component.h"
#include "fplutil/intrusive_list.h"
#include "library_components_generated.h"
#include "mathfu/constants.h"
#include "mathfu/glsl_mappings.h"
#include "mathfu/matrix_4x4.h"

namespace corgi {
namespace component_library {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @struct TransformData
///
/// @brief Holds all transform data for Entities, such as
/// position, scale, and orientation.
struct TransformData {
  /// @brief The default constructor for TransformData.
  TransformData();

  /// @brief Move assignment operator for the TransformData.
  ///
  /// @param[in] other The other TransformData whose contents should be
  /// moved into this TransformData.
  ///
  /// @return Returns a reference to the TransformData.
  TransformData& operator=(TransformData&& other) {
    position = std::move(other.position);
    scale = std::move(other.scale);
    orientation = std::move(other.orientation);
    world_transform = std::move(other.world_transform);
    owner = std::move(other.owner);
    parent = std::move(other.parent);
    child_ids = std::move(other.child_ids);
    pending_child_ids = std::move(other.pending_child_ids);
    child_node = std::move(other.child_node);
    children = std::move(other.children);
    return *this;
  }

  /// @brief Move constructor for the TransformData.
  ///
  /// @param[in] other The other TransformData whose contents should be
  /// moved into this TransformData.
  TransformData(TransformData&& other) : children(&TransformData::child_node) {
    *this = std::move(other);  // Calls the move assignment operator.
  }

  /// @brief The position of this Entity.
  mathfu::vec3 position;

  /// @brief The scale of the Entity.
  mathfu::vec3 scale;

  /// @brief The orientation of the Entity.
  mathfu::quat orientation;

  /// @brief The transform to place the Entity in world space.
  mathfu::mat4 world_transform;

  /// @brief An EntityRef reference to the Entity that owns this
  /// Component data.
  corgi::EntityRef owner;

  /// @brief An EntityRef reference to the parent Entity.
  corgi::EntityRef parent;

  /// @brief A set of std::string child IDs that will need to be exported.
  std::set<std::string> child_ids;

  /// @brief Child IDs we will be linking up on the next update; we couldn't
  /// link them before because they may not have been loaded.
  std::vector<std::string> pending_child_ids;

  /// @brief The node inside the intrusive list for this Entity.
  fplutil::intrusive_list_node child_node;

  /// @brief The intrusive list of this Entity's children.
  fplutil::intrusive_list<TransformData> children;

  // We construct the matrix by hand here, because we know that it will
  // always be a composition of rotation, scale, and translation, so we
  // can do things a bit more cleanly than 3 4x4 matrix multiplications.
  /// @brief Construct the transform matrix from the reotation, scale,
  /// and translation.
  ///
  /// @return Returns a mathfu::mat4 containing the transform matrix.
  mathfu::mat4 GetTransformMatrix() {
    // Start with rotation:
    mathfu::mat3 rot = orientation.ToMatrix();

    // Break it up into columns:
    mathfu::vec4 c0 = mathfu::vec4(rot[0], rot[3], rot[6], 0);
    mathfu::vec4 c1 = mathfu::vec4(rot[1], rot[4], rot[7], 0);
    mathfu::vec4 c2 = mathfu::vec4(rot[2], rot[5], rot[8], 0);
    mathfu::vec4 c3 = mathfu::vec4(0, 0, 0, 1);

    // Apply scale:
    c0 *= scale.x;
    c1 *= scale.y;
    c2 *= scale.z;

    // Apply translation:
    c3[0] = position.x;
    c3[1] = position.y;
    c3[2] = position.z;

    // Compose and return result:
    return mathfu::mat4(c0, c1, c2, c3);
  }

 private:
  TransformData(const TransformData&);
  TransformData& operator=(const TransformData&);
};

/// @class TransformComponent
///
/// @brief A Component that handles the transformations for every
/// Entity that registers with it.
class TransformComponent : public corgi::Component<TransformData> {
 public:
  /// @brief Destructor for TransformComponent.
  virtual ~TransformComponent() {}

  /// @brief Get the world position for a given Entity
  ///
  /// @param[in] entity An EntityRef to the Entity whose world
  /// position should be returned.
  ///
  /// @return Returns the world position of the Entity as a mathfu::vec3.
  mathfu::vec3 WorldPosition(corgi::EntityRef entity);

  /// @brief Get the world orientation for a given Entity.
  ///
  /// @param[in] entity An EntityRef to the Entity whose world
  /// orientation should be returned.
  ///
  /// @return Returns the world orientation for this Entity as a mathfu::quat.
  mathfu::quat WorldOrientation(corgi::EntityRef entity);

  /// @brief Get the world transform for a given Entity.
  ///
  /// @param[in] entity An EntityRef to the Entity whose world
  /// transform should be returned.
  ///
  /// @return Returns the world transform as a mathfu::mat4.
  mathfu::mat4 WorldTransform(corgi::EntityRef entity);

  /// @brief Get the topmost parent of a given Entity.
  ///
  /// @param[in] entity An EntityRef to the Entity whose topmost parent
  /// should be returned.
  ///
  /// @return Returns an EntityRef to the topmost parent of this Entity. If
  /// not parents exist, then it returns the `entity` itself.
  corgi::EntityRef GetRootParent(const corgi::EntityRef& entity) const;

  /// @brief Deserialize a flat binary buffer to create and populate an Entity
  /// from raw data
  ///
  /// @param[in,out] entity An EntityRef reference that points to an Entity that
  /// is being added from the raw data.
  /// @param[in] raw_data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);

  /// @brief Serializes a TransformComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to an Entity whose corresponding
  /// TransformData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a
  /// flat binary buffer.
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  /// @brief Initialize a new Entity and associate it with its TransformData.
  ///
  /// @param[in] entity An EntityRef to the Entity that should be associated
  /// with its corresponding TransformData.
  virtual void InitEntity(corgi::EntityRef& entity);

  /// @brief Remove the Entity and cleanup any children it may have.
  ///
  /// @param[in] entity An EntityRef reference to the Entity that should
  /// have itself and any children removed.
  virtual void CleanupEntity(corgi::EntityRef& entity);

  /// @brief Update the world position for all Entities registered with
  /// this Component.
  ///
  /// @param[in] delta_time The delta time since the last call to this method.
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  /// @brief Add an Entity as a child to another Entity.
  ///
  /// @param[in] child An EntityRef to the Entity that should be added
  /// as a child of `parent`.
  /// @param[in] parent An EntityRef to the Entity that should have `child`
  /// added as a child.
  void AddChild(corgi::EntityRef& child, corgi::EntityRef& parent);

  /// @brief Remove an Entity from the intrusive list that it belongs to.
  ///
  /// @param[in] entity An EntityRef to the Entity that should be removed,
  /// as a child, in the intrusive list.
  void RemoveChild(corgi::EntityRef& entity);

  /// @brief After loading Entities, fix up any of their child links.
  void PostLoadFixup();

  /// @brief Take any pending child IDs and set up the child links.
  ///
  /// @param[in] entity An EntityRef to the parent Entity whose child
  /// links should be updated.
  void UpdateChildLinks(corgi::EntityRef& entity);

  /// @brief Get the first Entity with a Component, given an ID.
  ///
  /// @note Searches in breadth-first order.
  ///
  /// @param[in] entity The EntityRef to the parent Entity whose children
  /// should be searched.
  /// @param[in] id The ComponentId to identify the Component that should
  /// be checked during the search.
  ///
  /// @return Returns an EntityRef to the first Entity that has the
  /// Component with ID `id`. Otherwise, it returns an invalid
  /// EntityRef, if no such child exists.
  corgi::EntityRef ChildWithComponent(const corgi::EntityRef& entity,
                                      corgi::ComponentId id) const {
    return ChildWithComponents(entity, &id, 1);
  }

  /// @brief Get the first Entity with all the Components given by an array
  /// of IDs.
  ///
  /// @note Searches in breadth-first order.
  ///
  /// @param[in] entity The EntityRef to the parent Entity whose children
  /// should be searched.
  /// @param[in] ids An array of ComponentId to identify all the Components
  /// that should be checked during the search.
  /// @param[in] num_ids The length of the `ids` array.
  ///
  /// @return Returns an EntityRef to the first Entity that has all
  /// the Components in the `ids` array. Otherwise, it returns an invalid
  /// EntityRef, if no such child exists.
  corgi::EntityRef ChildWithComponents(const corgi::EntityRef& entity,
                                       const corgi::ComponentId* ids,
                                       size_t num_ids) const;

 private:
  void UpdateWorldPosition(corgi::EntityRef& entity,
                           const mathfu::mat4& transform);
};
/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::TransformComponent,
                         corgi::component_library::TransformData)

#endif  // CORGI_COMPONENT_LIBRARY_TRANSFORM_H_
