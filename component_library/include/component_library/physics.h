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

#ifndef COMPONENT_LIBRARY_PHYSICS_H_
#define COMPONENT_LIBRARY_PHYSICS_H_

#include <memory>
#include <vector>
#include "breadboard/event.h"
#include "entity/component.h"
#include "flatbuffers/reflection.h"
#include "fplbase/asset_manager.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"
#include "library_components_generated.h"
#include "mathfu/glsl_mappings.h"

class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionShape;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btMotionState;
class btRigidBody;
class btSequentialImpulseConstraintSolver;
class btTriangleMesh;

namespace fpl {
namespace component_library {

BREADBOARD_DECLARE_EVENT(kCollisionEventId)

class PhysicsComponent;
class PhysicsDebugDrawer;

static const int kMaxPhysicsBodies = 5;

// Data describing which entities were involed in a collision and where.
struct CollisionData {
  entity::EntityRef this_entity;
  mathfu::vec3 this_position;
  std::string this_tag;

  entity::EntityRef other_entity;
  mathfu::vec3 other_position;
  std::string other_tag;
};

typedef void (*CollisionCallback)(CollisionData* collision_data,
                                  void* user_data);

// Data describing a single Bullet rigid body shape.
struct RigidBodyData {
  RigidBodyData();
  ~RigidBodyData();
  mathfu::vec3 offset;
  short collision_type;
  short collides_with;
  std::string user_tag;
  std::unique_ptr<btCollisionShape> shape;
  std::unique_ptr<btMotionState> motion_state;
  std::unique_ptr<btRigidBody> rigid_body;
  bool should_export;  // Whether the shape should be included on export.

  RigidBodyData& operator=(RigidBodyData&& src);

 private:
  RigidBodyData& operator=(const RigidBodyData&);
  RigidBodyData(const RigidBodyData&);
};

// Data for scene object components.
struct PhysicsData {
  friend PhysicsComponent;

 public:
  PhysicsData();
  ~PhysicsData();
  PhysicsData(PhysicsData&& src);
  PhysicsData& operator=(PhysicsData&& src);

  mathfu::vec3 Velocity() const;
  void SetVelocity(const mathfu::vec3& velocity);
  mathfu::vec3 AngularVelocity() const;
  void SetAngularVelocity(const mathfu::vec3& velocity);
  int RigidBodyIndex(const std::string& user_tag) const;
  void GetAabb(int rigid_body_idx, mathfu::vec3* min, mathfu::vec3* max) const;

  bool enabled() const { return enabled_; }
  int body_count() const { return body_count_; }

 private:
  PhysicsData(const PhysicsData&);
  PhysicsData& operator=(const PhysicsData&);

  // The rigid bodies associated with the entity. Note that only the first one
  // can be set to not be kinematic, all subsequent ones are forced to be.
  RigidBodyData rigid_bodies_[kMaxPhysicsBodies];
  std::unique_ptr<btTriangleMesh> triangle_mesh_;
  int body_count_;
  bool enabled_;
};

class PhysicsComponent : public entity::Component<PhysicsData> {
 public:
  PhysicsComponent();
  virtual ~PhysicsComponent();

  virtual void AddFromRawData(entity::EntityRef& entity, const void* raw_data);
  // TODO: Implement ExportRawData function for editor (b/21589546)
  virtual RawDataUniquePtr ExportRawData(const entity::EntityRef& entity) const;

  virtual void Init();
  virtual void InitEntity(entity::EntityRef& /*entity*/);
  virtual void CleanupEntity(entity::EntityRef& entity);
  virtual void UpdateAllEntities(entity::WorldTime delta_time);

  void ProcessBulletTickCallback();

  void UpdatePhysicsFromTransform(const entity::EntityRef& entity);
  void UpdatePhysicsScale(const entity::EntityRef& entity);

  void EnablePhysics(const entity::EntityRef& entity);
  void DisablePhysics(const entity::EntityRef& entity);

  // Initialize the data needed to generate a static mesh. Adds the entity to
  // the physics component if necessary.
  void InitStaticMesh(entity::EntityRef& entity);
  // Add a triangle to the static mesh for the given entity. Note that Prepare
  // needs to be called beforehand.
  void AddStaticMeshTriangle(const entity::EntityRef& entity,
                             const mathfu::vec3& pt0, const mathfu::vec3& pt1,
                             const mathfu::vec3& pt2);
  // Generates a static mesh shape and adds it to the world, based on the
  // previously added static mesh triangles.
  void FinalizeStaticMesh(const entity::EntityRef& entity, short collision_type,
                          short collides_with, float mass, float restitution,
                          const std::string& user_tag);

  // Generate an AABB based on the rendermesh that collides with the raycast
  // layer. Note, if the entity already collides with the raycast layer, no
  // change occurs. If there is no rendermesh, a unit cube is used instead.
  // Also sets whether the resulting shape should be exported.
  void GenerateRaycastShape(entity::EntityRef& entity, bool result_exportable);
  // Performs a raycast into the world, returning the first entity hit.
  // Optionally takes a layer_mask, which the shape much collide with to be
  // raycast against, and output vector, where it will store the world position
  // the collision with the ray occurred.
  entity::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end);
  entity::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                  short layer_mask);
  entity::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                  mathfu::vec3* hit_point);
  entity::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                  short layer_mask, mathfu::vec3* hit_point);
  void DebugDrawWorld(Renderer* renderer, const mathfu::mat4& camera_transform);
  void DebugDrawObject(Renderer* renderer, const mathfu::mat4& camera_transform,
                       const entity::EntityRef& entity,
                       const mathfu::vec3& color);

  btDiscreteDynamicsWorld* bullet_world() { return bullet_world_.get(); }

  void set_gravity(float gravity) { gravity_ = gravity; }
  float gravity() const { return gravity_; }

  void set_max_steps(int max_steps) { max_steps_ = max_steps; }
  int max_steps() const { return max_steps_; }

  CollisionData& collision_data() { return collision_data_; }

  void set_collision_callback(CollisionCallback callback, void* user_data) {
    collision_callback_ = callback;
    collision_user_data_ = user_data;
  }

 private:
  void ClearPhysicsData(const entity::EntityRef& entity);
  void UpdatePhysicsObjectsTransform(const entity::EntityRef& entity,
                                     bool kinematic_only);

  // Collision data is cached so that the event graphs can operate on it.
  CollisionData collision_data_;

  // An event callback to call when a collision occurs. If a callback is
  // registered, it is called in addition to evaluating the graph.
  CollisionCallback collision_callback_;
  void* collision_user_data_;

  std::unique_ptr<btDiscreteDynamicsWorld> bullet_world_;
  std::unique_ptr<btBroadphaseInterface> broadphase_;
  std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration_;
  std::unique_ptr<btCollisionDispatcher> collision_dispatcher_;
  std::unique_ptr<btSequentialImpulseConstraintSolver> constraint_solver_;

  std::unique_ptr<PhysicsDebugDrawer> debug_drawer_;

  float gravity_;
  int max_steps_;
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::PhysicsComponent,
                              fpl::component_library::PhysicsData)

#endif  // COMPONENT_LIBRARY_PHYSICS_H_
