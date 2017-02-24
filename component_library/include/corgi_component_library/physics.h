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

#ifndef CORGI_COMPONENT_LIBRARY_PHYSICS_H_
#define CORGI_COMPONENT_LIBRARY_PHYSICS_H_

#include <memory>
#include <vector>

#include "breadboard/event.h"
#include "corgi/component.h"
#include "flatbuffers/reflection.h"
#include "fplbase/asset_manager.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"
#include "library_components_generated.h"
#include "mathfu/glsl_mappings.h"

/// @cond COMPONENT_LIBRARY_INTERNAL
class btBroadphaseInterface;
class btCollisionDispatcher;
class btCollisionShape;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btMotionState;
class btRigidBody;
class btSequentialImpulseConstraintSolver;
class btTriangleMesh;

namespace corgi {
namespace component_library {

BREADBOARD_DECLARE_EVENT(kCollisionEventId)

class PhysicsComponent;
class PhysicsDebugDrawer;
/// @endcond

/// @file
/// @addtogroup corgi_component_library
/// @{

/// @var kMaxPhysicsBodies
///
/// @brief The maximum number of physics bodies per Entity.
static const int kMaxPhysicsBodies = 5;

/// @var kDefaultPhysicsGravity
///
/// @brief The constant for gravity.
static const float kDefaultPhysicsGravity = -9.8f;

/// @var kDefaultPhysicsMaxSteps
///
/// @brief The default number of max steps to advance per frame.
static const int kDefaultPhysicsMaxSteps = 5;

/// @struct CollisionData
///
/// @brief Data describing which Entities were involed in a collision and where.
struct CollisionData {
  /// @var this_entity
  ///
  /// @brief The first Entity involved in the collision.
  corgi::EntityRef this_entity;

  /// @var this_position
  ///
  /// @brief The position of the first Entity involved in the collision.
  mathfu::vec3 this_position;

  /// @var this_tag
  ///
  /// @brief A std::string tag to identify the first Entity involved in the
  /// collision.
  std::string this_tag;

  /// @var other_entity
  ///
  /// @brief The second Entity involved in the collision.
  corgi::EntityRef other_entity;

  /// @var other_position
  ///
  /// @brief The position of the second Entity involved in the collision.
  mathfu::vec3 other_position;

  /// @var other_tag
  ///
  /// @brief A std::string tag to identify the second Entity involved in the
  /// collision.
  std::string other_tag;
};

/// @typedef CollisionCallback
///
/// @brief A function pointer for the callback after a collision.
typedef void (*CollisionCallback)(CollisionData* collision_data,
                                  void* user_data);

/// @struct RigidBodyData
///
/// @brief Data describing a single Bullet rigid body shape.
struct RigidBodyData {
  /// @brief Constructor for RigidBodyData.
  RigidBodyData();
  /// @brief Destructor for RigidBodyData.
  ~RigidBodyData();

  /// @var offset
  ///
  /// @brief The position offset from the origin of the
  /// TransformComponent to the center.
  mathfu::vec3 offset;

  /// @var collision_type
  ///
  /// @brief A bit field determining what type of collision object this is.
  short collision_type;

  /// @var collides_with
  ///
  /// @brief A bit field determining what types of objects it can collide into.
  short collides_with;

  /// @var user_tag
  ///
  /// @brief A user-defined C-string tag to identify this rigid body.
  std::string user_tag;

  /// @var shape
  ///
  /// @brief The btCollisionShape of the RigidBodyData.
  std::unique_ptr<btCollisionShape> shape;

  /// @var motion_state
  ///
  /// @brief The btMotionState of the RigidBodyData.
  std::unique_ptr<btMotionState> motion_state;

  /// @var rigid_body
  ///
  /// @brief The btRigidBody of the RigidBodyData.
  std::unique_ptr<btRigidBody> rigid_body;

  /// @var should_export
  ///
  /// @brief Should the shape be included on export.
  bool should_export;

  /// @brief The move assignment operator for RigidBodyData.
  ///
  /// @param[in] src The other RigidBodyData to move into this RigidBodyData.
  RigidBodyData& operator=(RigidBodyData&& src);

 private:
  RigidBodyData& operator=(const RigidBodyData&);
  RigidBodyData(const RigidBodyData&);
};

/// @struct PhysicsData
///
/// @brief Data for scene object Components.
struct PhysicsData {
  /// @cond COMPONENT_LIBRARY_INTERNAL
  friend PhysicsComponent;
  /// @endcond

 public:
  /// @brief Constructor for PhysicsData.
  PhysicsData();

  /// @brief Destructor for PhysicsData.
  ~PhysicsData();

  /// @brief The move constructor for PhysicsData.
  ///
  /// @param[in] src The other PhysicsData to move into this PhysicsData.
  PhysicsData(PhysicsData&& src);

  /// @brief The move assignment operator for PhysicsData.
  ///
  /// @param[in] src The other PhysicsData to move into this PhysicsData.
  PhysicsData& operator=(PhysicsData&& src);

  /// @brief Get the velocity of the Entity.
  ///
  /// @return Returns the velocity of this Entity as a mathfu::vec3.
  mathfu::vec3 Velocity() const;

  /// @brief Set the velocity of the Entity.
  ///
  /// @param[in] velocity A const mathfu::vec3 reference to the velocity
  /// to set for this Entity.
  void SetVelocity(const mathfu::vec3& velocity);

  /// @brief Get the angular velocity of the Entity.
  ///
  /// @return Returns the angular velocity of the Entity as a mathfu::vec3.
  mathfu::vec3 AngularVelocity() const;

  /// @brief Set the angular velocity of the Entity.
  ///
  /// @param[in] velocity A const mathfu::vec3 reference to the angular velocity
  /// to set for the Entity.
  void SetAngularVelocity(const mathfu::vec3& velocity);

  /// @brief Get the index for the RigidBody for a given tag.
  ///
  /// @param[in] user_tag A const reference to a std::string of the tag for
  /// the desired rigid body.
  ///
  /// @return Returns an int corresponding to the index of the rigid body.
  int RigidBodyIndex(const std::string& user_tag) const;

  /// @brief Get the axis-aligned bounding box (AABB) of a rigid body at a given
  /// index.
  ///
  /// @param[in] rigid_body_idx The index of the rigid body whose AABB should
  /// be returned though `min` and `max`.
  /// @param[out] min A mathfu::vec3 pointer that captures the output of the
  /// mininum corner of the AABB.
  /// @param[out] max A mathfu::vec3 pointer that captures the output of the
  /// maximum corner of the AABB.
  void GetAabb(int rigid_body_idx, mathfu::vec3* min, mathfu::vec3* max) const;

  /// @brief Check if this physics is enabled for this Entity.
  ///
  /// @return Returns `true` if the physics is enabled. Otherwise, return
  /// `false`.
  bool enabled() const { return enabled_; }

  /// @brief Get the number of physics shapes for this Entity.
  ///
  /// @return Returns an int corresponding to the body count.
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
  float gravity_multiplier_;
};

/// @class PhysicsComponent
///
/// @brief The Component that manages the physics for every Entity
/// that registers with it.
class PhysicsComponent : public corgi::Component<PhysicsData> {
 public:
  /// @brief Constructor for the PhysicsComponent.
  PhysicsComponent();

  /// @brief Destructor for the PhysicsComponent.
  virtual ~PhysicsComponent();

  /// @brief Deserialize a flat binary buffer to create and populate an Entity
  /// from raw data.
  ///
  /// @param[in,out] entity An EntityRef reference that points to an Entity that
  /// is being added from the raw data.
  /// @param[in] raw_data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);

  /// @brief Serializes a PhysicsComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to an Entity whose corresponding
  /// PhysicsData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a
  /// flat binary buffer.
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  /// @brief Initializes the internal state of the variables for the
  /// PhysicsComponent.
  virtual void Init();

  /// @brief Adds an Entity to the TransformComponent.
  ///
  /// @note PhysicsComponents require that you have a TransformComponent.
  virtual void InitEntity(corgi::EntityRef& /*entity*/);

  /// @brief Disables physics for the given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity that
  /// should be cleaned up by having its physics disabled.
  virtual void CleanupEntity(corgi::EntityRef& entity);

  /// @brief Update the physics data for all Entities registered with
  /// this Component.
  ///
  /// @param[in] delta_time The time delta since the last call.
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  /// @brief Called after every Bullet frame update to handle all the
  /// collisions.
  void ProcessBulletTickCallback();

  /// @brief Update the physics world to match the transform for
  /// a given Entity.
  ///
  /// @param[in] entity The Entity whose physics data should be updated
  /// to match the transform data.
  void UpdatePhysicsFromTransform(const corgi::EntityRef& entity);

  /// @brief Update the physics scale to match the transform for a
  /// given Entity.
  ///
  /// @param[in] entity The Entity whose physics scale should be updated
  /// to match the transform data.
  void UpdatePhysicsScale(const corgi::EntityRef& entity);

  /// @brief Enables physics for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose
  /// physics should be enabled.
  void EnablePhysics(const corgi::EntityRef& entity);

  /// @brief Disables physics for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose
  /// physics should be disabled.
  void DisablePhysics(const corgi::EntityRef& entity);

  /// @brief Activate the rigid bodies for a given Entity to force reevaluation
  /// within the scene.
  ///
  /// @param[in] entity An EntityRef reference to the Entity
  /// whose rigid bodies should be activated.
  void AwakenEntity(const corgi::EntityRef& entity);

  /// @brief Activates the rigid bodies in every Entity to force reevaluation
  /// within the scene.
  void AwakenAllEntities();

  /// @brief Initialize the data needed to generate a static mesh.
  ///
  /// Adds the entity to the PhysicsComponent, if necessary.
  ///
  /// @param[in] entity An EntityRef to the entity that should be added to the
  /// PhysicsComponent, if necessary.
  void InitStaticMesh(corgi::EntityRef& entity);

  /// @brief Add a triangle to the static mesh for the given entity.
  ///
  /// @note Note that `InitStaticMesh()` needs to be called beforehand.
  ///
  /// @param[in] entity An EntityRef to the Entity whose PhysicsData should
  /// add the new static mesh.
  /// @param[in] pt0 The first vertex for the triangle.
  /// @param[in] pt1 The second vertex for the triangle.
  /// @param[in] pt2 The third vertex for the triangle.
  void AddStaticMeshTriangle(const corgi::EntityRef& entity,
                             const mathfu::vec3& pt0, const mathfu::vec3& pt1,
                             const mathfu::vec3& pt2);

  /// @brief Generates a static mesh shape and adds it to the world, based on
  /// the previously added static mesh triangles.
  ///
  /// @param[in] entity An EntityRef to the Entity whose PhysicsData contains
  /// the static mesh.
  /// @param[in] collision_type A bit field determining the type of collision
  /// for the rigid body.
  /// @param[in] collides_with A bit field determining what type of objects
  /// the rigid body can collide into.
  /// @param[in] mass The mass of the rigid body.
  /// @param[in] restitution The restitution for the rigid body.
  /// @param[in] user_tag A C-string provided by the user to identify the
  /// rigid body.
  void FinalizeStaticMesh(const corgi::EntityRef& entity, short collision_type,
                          short collides_with, float mass, float restitution,
                          const std::string& user_tag);

  /// @brief Generate an AABB based on the rendermesh that collides with the
  /// raycast layer.
  ///
  /// @note If the entity already collides with the raycast layer, no
  /// change occurs. If there is no rendermesh, a unit cube is used instead.
  ///
  /// @param[in] entity An EntityRef to the Entity whose AABB should be
  /// generated for raycasting.
  /// @param[in] result_exportable A bool flag determining if the resulting
  /// shape should be exported.
  void GenerateRaycastShape(corgi::EntityRef& entity, bool result_exportable);

  /// @brief Performs a raycast into the world, returning the first Entity hit.
  ///
  /// @param[in] start The start point (origin) of the ray.
  /// @param[in] end The end point of the ray.
  ///
  /// @return Returns an EntityRef to the first Entity that was hit.
  corgi::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end);

  /// @brief Performs a raycast into the world, returning the first Entity hit.
  ///
  /// @param[in] start The start point (origin) of the ray.
  /// @param[in] end The end point of the ray.
  /// @param[in] layer_mask A bit field used to specify which layers the raycast
  /// could hit.
  ///
  /// @return Returns an EntityRef to the first Entity that was hit.
  corgi::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                 short layer_mask);

  /// @brief Performs a raycast into the world, returning the first Entity hit.
  ///
  /// @param[in] start The start point (origin) of the ray.
  /// @param[in] end The end point of the ray.
  /// @param[out] hit_point Captures the output of the value where the ray hit
  /// the Entity in world position, as a vec3.
  ///
  /// @return Returns an EntityRef to the first Entity that was hit.
  corgi::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                 mathfu::vec3* hit_point);

  /// @brief Performs a raycast into the world, returning the first Entity hit.
  ///
  /// @param[in] start The start point (origin) of the ray.
  /// @param[in] end The end point of the ray.
  /// @param[in] layer_mask A bit field used to specify which layers the raycast
  /// could hit.
  /// @param[out] hit_point Captures the output of the value where the ray hit
  /// the Entity in world position, as a vec3.
  ///
  /// @return Returns an EntityRef to the first Entity that was hit.
  corgi::EntityRef RaycastSingle(mathfu::vec3& start, mathfu::vec3& end,
                                 short layer_mask, mathfu::vec3* hit_point);

  /// @brief Get the gravity value of the given Entity.
  ///
  /// @param[in] entity The Entity to get the custom gravity for.
  /// @return The gravity value of the Entity.
  float GravityForEntity(const corgi::EntityRef& entity) const;

  /// @brief Render the entire physics worlds using Bullet's default
  /// debugging tools.
  ///
  /// @param[in] renderer A pointer to the fplbase::Renderer to handle the
  /// rendering.
  /// @param[in] camera_transform The camera transform for the view projection.
  void DebugDrawWorld(fplbase::Renderer* renderer,
                      const mathfu::mat4& camera_transform);

  /// @brief Render a given Entity's physics using Bullet's default debugging
  /// tools.
  ///
  /// @param[in] renderer A pointer to the fplbase::Renderer to handle the
  /// rendering.
  /// @param[in] camera_transform The camera transform for the view projection.
  /// @param[in] entity The Entity whose physics should be rendered.
  /// @param[in] color The color to draw the object in RGB.
  void DebugDrawObject(fplbase::Renderer* renderer,
                       const mathfu::mat4& camera_transform,
                       const corgi::EntityRef& entity,
                       const mathfu::vec3& color);

  /// @brief Get the bullet world.
  ///
  /// @return Returns the bullet world as a pointer to
  /// `btDiscreteDynamicsWorld`.
  btDiscreteDynamicsWorld* bullet_world() { return bullet_world_.get(); }

  /// @brief Set the gravity value.
  ///
  /// @param[in] gravity A float to set as the new gravity value.
  void set_gravity(float gravity) { gravity_ = gravity; }

  /// @brief Get the gravity value.
  ///
  /// @return Returns the gravity as a float.
  float gravity() const { return gravity_; }

  /// @brief Set the max steps.
  ///
  /// @param[in] max_steps An int to set as the new max steps value.
  void set_max_steps(int max_steps) { max_steps_ = max_steps; }

  /// @brief Get the max steps.
  ///
  /// @return Returns the max steps as an int.
  int max_steps() const { return max_steps_; }

  /// @brief Get the collision data.
  ///
  /// @return Returns a reference to the CollisionData.
  CollisionData& collision_data() { return collision_data_; }

  /// @brief Sets the callback function for collisions.
  ///
  /// @param[in] callback The event callback to call when a collision occurs.
  /// If a callback is registered, then it is called in addition to evaluating
  /// the graph.
  /// @param[in] user_data A void pointer to any data provided to for the
  /// `callback`.
  void set_collision_callback(CollisionCallback callback, void* user_data) {
    collision_callback_ = callback;
    collision_user_data_ = user_data;
  }

 private:
  void ClearPhysicsData(const corgi::EntityRef& entity);
  void UpdatePhysicsObjectsTransform(const corgi::EntityRef& entity,
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
/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::PhysicsComponent,
                         corgi::component_library::PhysicsData)

#endif  // CORGI_COMPONENT_LIBRARY_PHYSICS_H_
