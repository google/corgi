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

#include "btBulletDynamicsCommon.h"
#include "entity/component.h"
#include "event/event_manager.h"
#include "fplbase/asset_manager.h"
#include "flatbuffers/reflection.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"
#include "library_components_generated.h"
#include "mathfu/glsl_mappings.h"

namespace fpl {
namespace component_library {

const int kMaxPhysicsBodies = 5;

// Data describing a single Bullet rigid body shape.
struct RigidBodyData {
  RigidBodyData() {}
  mathfu::vec3 offset;
  short collision_type;
  short collides_with;
  std::string user_tag;
  std::unique_ptr<btCollisionShape> shape;
  std::unique_ptr<btMotionState> motion_state;
  std::unique_ptr<btRigidBody> rigid_body;
  bool should_export;  // Whether the shape should be included on export.

  RigidBodyData& operator=(RigidBodyData&& src) {
    offset = std::move(src.offset);
    collision_type = std::move(src.collision_type);
    collides_with = std::move(src.collides_with);
    user_tag = std::move(src.user_tag);
    shape = std::move(src.shape);
    motion_state = std::move(src.motion_state);
    rigid_body = std::move(src.rigid_body);
    should_export = std::move(src.should_export);
    return *this;
  }

 private:
  RigidBodyData& operator=(const RigidBodyData&);
  RigidBodyData(const RigidBodyData&);
};

static inline btVector3 ToBtVector3(const mathfu::vec3& v) {
  return btVector3(v.x(), v.y(), v.z());
}

static inline btVector3 ToBtVector3(const fpl::Vec3& v) {
  return btVector3(v.x(), v.y(), v.z());
}

static inline fpl::Vec3 BtToFlatVec3(const btVector3& v) {
  return fpl::Vec3(v.x(), v.y(), v.z());
}

static inline mathfu::vec3 BtToMathfuVec3(const btVector3& v) {
  return mathfu::vec3(v.x(), v.y(), v.z());
}

static inline btQuaternion ToBtQuaternion(const mathfu::quat& q) {
  // Bullet assumes a right handed system, while mathfu is left, so the axes
  // need to be negated.
  return btQuaternion(-q.vector().x(), -q.vector().y(), -q.vector().z(),
                      q.scalar());
}

static inline mathfu::quat BtToMathfuQuat(const btQuaternion& q) {
  // As above, the axes need to be negated.
  return mathfu::quat(q.getW(), -q.getX(), -q.getY(), -q.getZ());
}

// Data for scene object components.
struct PhysicsData {
 public:
  PhysicsData() : body_count(0), enabled(false) {}

  mathfu::vec3 Velocity() const {
    // Only the first body can be non-kinematic, and thus use velocity.
    return BtToMathfuVec3(rigid_bodies[0].rigid_body->getLinearVelocity());
  }
  void SetVelocity(const mathfu::vec3& velocity) {
    rigid_bodies[0].rigid_body->setLinearVelocity(ToBtVector3(velocity));
  }
  mathfu::vec3 AngularVelocity() const {
    return BtToMathfuVec3(rigid_bodies[0].rigid_body->getAngularVelocity());
  }
  void SetAngularVelocity(const mathfu::vec3& velocity) {
    rigid_bodies[0].rigid_body->setAngularVelocity(ToBtVector3(velocity));
  }
  int RigidBodyIndex(const std::string& user_tag) const {
    for (int i = 0; i < body_count; ++i) {
      if (user_tag == rigid_bodies[i].user_tag) return i;
    }
    return -1;
  }
  void GetAabb(int rigid_body_idx, mathfu::vec3* min, mathfu::vec3* max) const {
    btVector3 bt_min;
    btVector3 bt_max;
    rigid_bodies[rigid_body_idx].rigid_body->getAabb(bt_min, bt_max);
    *min = BtToMathfuVec3(bt_min);
    *max = BtToMathfuVec3(bt_max);
  }

  // The rigid bodies associated with the entity. Note that only the first one
  // can be set to not be kinematic, all subsequent ones are forced to be.
  RigidBodyData rigid_bodies[kMaxPhysicsBodies];
  std::unique_ptr<btTriangleMesh> triangle_mesh;
  int body_count;
  bool enabled;

  PhysicsData(PhysicsData&& src) {
    body_count = std::move(src.body_count);
    enabled = std::move(src.enabled);
    triangle_mesh = std::move(src.triangle_mesh);
    for (size_t i = 0; i < kMaxPhysicsBodies; i++) {
      rigid_bodies[i] = std::move(src.rigid_bodies[i]);
    }
  }
  PhysicsData& operator=(PhysicsData&& src) {
    body_count = std::move(src.body_count);
    enabled = std::move(src.enabled);
    triangle_mesh = std::move(src.triangle_mesh);
    for (size_t i = 0; i < kMaxPhysicsBodies; i++) {
      rigid_bodies[i] = std::move(src.rigid_bodies[i]);
    }
    return *this;
  }

 private:
  PhysicsData(const PhysicsData&);
  PhysicsData& operator=(const PhysicsData&);
};

// Used by Bullet to render the physics scene as a wireframe.
class PhysicsDebugDrawer : public btIDebugDraw {
 public:
  virtual void drawLine(const btVector3& from, const btVector3& to,
                        const btVector3& color);
  virtual int getDebugMode() const { return DBG_DrawWireframe; }

  virtual void drawContactPoint(const btVector3& /*pointOnB*/,
                                const btVector3& /*normalOnB*/,
                                btScalar /*distance*/, int /*lifeTime*/,
                                const btVector3& /*color*/) {}
  virtual void reportErrorWarning(const char* /*warningString*/) {}
  virtual void draw3dText(const btVector3& /*location*/,
                          const char* /*textString*/) {}
  virtual void setDebugMode(int /*debugMode*/) {}

  Shader* shader() { return shader_; }
  void set_shader(Shader* shader) { shader_ = shader; }

  Renderer* renderer() { return renderer_; }
  void set_renderer(Renderer* renderer) { renderer_ = renderer; }

 private:
  Shader* shader_;
  Renderer* renderer_;
};

class PhysicsComponent : public entity::Component<PhysicsData> {
 public:
  PhysicsComponent() {}
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
                          short collides_with, float mass, float restitution);

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
  float max_steps() const { return max_steps_; }

 private:
  void ClearPhysicsData(const entity::EntityRef& entity);
  void UpdatePhysicsObjectsTransform(const entity::EntityRef& entity,
                                     bool kinematic_only);

  event::EventManager* event_manager_;

  std::unique_ptr<btDiscreteDynamicsWorld> bullet_world_;
  std::unique_ptr<btBroadphaseInterface> broadphase_;
  std::unique_ptr<btDefaultCollisionConfiguration> collision_configuration_;
  std::unique_ptr<btCollisionDispatcher> collision_dispatcher_;
  std::unique_ptr<btSequentialImpulseConstraintSolver> constraint_solver_;

  PhysicsDebugDrawer debug_drawer_;

  float gravity_;
  int max_steps_;
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::PhysicsComponent,
                              fpl::component_library::PhysicsData)

#endif  // COMPONENT_LIBRARY_PHYSICS_H_
