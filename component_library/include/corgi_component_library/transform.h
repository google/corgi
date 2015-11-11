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

// Data for scene object components.
struct TransformData {
  TransformData();
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
  TransformData(TransformData&& other) : children(&TransformData::child_node) {
    *this = std::move(other);  // Calls the move assignment operator.
  }

  mathfu::vec3 position;
  mathfu::vec3 scale;
  mathfu::quat orientation;
  mathfu::mat4 world_transform;

  // A reference to the entity that owns this component data.
  corgi::EntityRef owner;

  // A reference to the parent entity.
  corgi::EntityRef parent;

  // Child IDs we will need to export.
  std::set<std::string> child_ids;

  // Child IDs we will be linking up on the next update; we couldn't link
  // them before because they may not have been loaded.
  std::vector<std::string> pending_child_ids;

  // The list of children.
  fplutil::intrusive_list_node child_node;
  fplutil::intrusive_list<TransformData> children;

  // We construct the matrix by hand here, because we know that it will
  // always be a composition of rotation, scale, and translation, so we
  // can do things a bit more cleanly than 3 4x4 matrix multiplications.
  mathfu::mat4 GetTransformMatrix() {
    // Start with rotation:
    mathfu::mat3 rot = orientation.ToMatrix();

    // Break it up into columns:
    mathfu::vec4 c0 = mathfu::vec4(rot[0], rot[3], rot[6], 0);
    mathfu::vec4 c1 = mathfu::vec4(rot[1], rot[4], rot[7], 0);
    mathfu::vec4 c2 = mathfu::vec4(rot[2], rot[5], rot[8], 0);
    mathfu::vec4 c3 = mathfu::vec4(0, 0, 0, 1);

    // Apply scale:
    c0 *= scale.x();
    c1 *= scale.y();
    c2 *= scale.z();

    // Apply translation:
    c3[0] = position.x();
    c3[1] = position.y();
    c3[2] = position.z();

    // Compose and return result:
    return mathfu::mat4(c0, c1, c2, c3);
  }

 private:
  TransformData(const TransformData&);
  TransformData& operator=(const TransformData&);
};

class TransformComponent : public corgi::Component<TransformData> {
 public:
  virtual ~TransformComponent() {}

  mathfu::vec3 WorldPosition(corgi::EntityRef entity);
  mathfu::quat WorldOrientation(corgi::EntityRef entity);
  mathfu::mat4 WorldTransform(corgi::EntityRef entity);

  // Returns the topmost parent of this entity.  Returns the entity itself
  // if it has no parents.
  corgi::EntityRef GetRootParent(const corgi::EntityRef& entity) const;

  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  virtual void InitEntity(corgi::EntityRef& entity);
  virtual void CleanupEntity(corgi::EntityRef& entity);
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  void AddChild(corgi::EntityRef& child, corgi::EntityRef& parent);
  void RemoveChild(corgi::EntityRef& entity);

  void PostLoadFixup();

  // Take any pending child IDs and set up the child links.
  void UpdateChildLinks(corgi::EntityRef& entity);

  // Returns `entity` or the first child of entity (in breadth-first order)
  // that has component `id`. Or returns an invalid EntityRef if no such child
  // exists.
  corgi::EntityRef ChildWithComponent(const corgi::EntityRef& entity,
                                      corgi::ComponentId id) const {
    return ChildWithComponents(entity, &id, 1);
  }

  // Returns `entity` or the first child of entity (in breadth-first order)
  // that has all components in array `ids`. Or returns an invalid EntityRef
  // if no such child exists.
  corgi::EntityRef ChildWithComponents(const corgi::EntityRef& entity,
                                       const corgi::ComponentId* ids,
                                       size_t num_ids) const;

 private:
  void UpdateWorldPosition(corgi::EntityRef& entity,
                           const mathfu::mat4& transform);
};

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::TransformComponent,
                         corgi::component_library::TransformData)

#endif  // CORGI_COMPONENT_LIBRARY_TRANSFORM_H_
