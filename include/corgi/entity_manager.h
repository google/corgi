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

#ifndef CORGI_ENTITY_MANAGER_H_
#define CORGI_ENTITY_MANAGER_H_

#include "corgi/component_id_lookup.h"
#include "corgi/component_interface.h"
#include "corgi/entity.h"
#include "corgi/entity_common.h"
#include "corgi/vector_pool.h"
#include "corgi/version.h"

namespace corgi {

/// @file
/// @addtogroup corgi_entity_manager
/// @{
///
/// @typedef EntityRef
///
/// @brief This should be used as the primary way to reference an Entity.
///
/// An EntityRef can be treated like a pointer to an Entity. In most cases,
/// it functions interchangeably with a normal pointer, but it also contains
/// extra functionality for determining if the EntityRef is invalid. For
/// instance, if the Entity that the EntityRef points to is deallocated, the
/// EntityRef will no longer be valid (even if new data exists in the same
/// memory location previously held by the deallocated Entity).
///
/// EntityRefs are typically passed by reference or const reference. If an
/// EntityRef is const, then you can only access the underlying Entity data
/// in a read-only manner.
typedef VectorPool<Entity>::VectorPoolReference EntityRef;

class EntityFactoryInterface;
class ComponentInterface;

/// @class EntityManager
/// @brief The EntityManager is the code that manages all
/// Entities and Components in the game. Normally the game will
/// instantiate EntityManager, and then use it to create and control
/// all of its Entities.
///
/// The main uses for this class are:
///  * Creating/destroying new Entities.
///  * Keeping track of all Components.
///  * Spawning and populating new Components from data.
///  * Updating Entities/Components.  (Usually done once per frame.)
class EntityManager {
 public:
  /// @brief Constructor for the EntityManager.
  EntityManager();

  /// @brief Returns the version of the Corgi entity library.
  const CorgiVersion* GetCorgiVersion() { return version_; }

  /// @typedef EntityStorageContainer
  ///
  /// @brief This is used to track all Entities stored by the EntityManager.
  typedef VectorPool<Entity> EntityStorageContainer;

  /// @brief Helper function for marshalling data from a Component.
  ///
  /// @tparam T The data type of the Component data to be returned.
  ///
  /// @param[in] entity The Entity associated with the desired data.
  ///
  /// @return Returns a pointer to the Component data, or returns
  /// a nullptr if no such data exists.
  template <typename T>
  T* GetComponentData(const EntityRef& entity) {
    return static_cast<T*>(
        GetComponentDataAsVoid(entity, ComponentIdLookup<T>::component_id));
  }

  /// @brief A helper function for marshalling data from a Component.
  ///
  /// @tparam T The data type of the Component data to be returned.
  ///
  /// @param[in] entity The Entity associated with the desired data.
  ///
  /// @return Returns a const pointer to the Component data, or returns
  /// a nullptr if no such data exists.
  template <typename T>
  const T* GetComponentData(const EntityRef& entity) const {
    return static_cast<const T*>(
        GetComponentDataAsVoid(entity, ComponentIdLookup<T>::component_id));
  }

  /// @brief A helper function for getting a particular Component, given
  /// the Component's data type.
  ///
  /// @note Asserts if the Component does not exist.
  ///
  /// @tparam T The data type of the Component to be returned.
  ///
  /// @return Returns a pointer to the Component given its data type.
  template <typename T>
  T* GetComponent() {
    ComponentId id =
        static_cast<ComponentId>(ComponentIdLookup<T>::component_id);
    assert(id != kInvalidComponent);
    assert(id < components_.size());
    return static_cast<T*>(components_[id]);
  }

  /// @brief A helper function for getting a particular Component, given
  /// the Component's data type.
  ///
  /// @note Asserts if the Component does not exist.
  ///
  /// @tparam T The data type of the Component to be returned.
  ///
  /// @return Returns a const pointer to the Component given its datatype.
  template <typename T>
  const T* GetComponent() const {
    ComponentId id =
        static_cast<ComponentId>(ComponentIdLookup<T>::component_id);
    assert(id != kInvalidComponent);
    assert(id < components_.size());
    return static_cast<const T*>(components_[id]);
  }

  /// @brief A helper function for adding a Component to an Entity, given
  /// its data type.
  ///
  /// @note Asserts if the Component does not exist.
  ///
  /// @tparam T The data type of the Component that should have the Entity
  /// added to it.
  ///
  /// @param[in] entity An EntityRef that points to the Entity that is being
  /// added to the Component.
  template <typename T>
  void AddEntityToComponent(EntityRef entity) {
    ComponentId id =
        static_cast<ComponentId>(ComponentIdLookup<T>::component_id);
    assert(id != kInvalidComponent);
    assert(id < components_.size());
    AddEntityToComponent(entity, id);
  }

  /// @brief A helper function for getting a particular Component, given the
  /// component ID.
  ///
  /// @param[in] id The component ID for the desired Component.
  ///
  /// @note Asserts if the id is less than kMaxComponentCount.
  ///
  /// @return Returns a pointer to the Component at the given id, which
  /// inherits from the ComponentInterface.
  inline ComponentInterface* GetComponent(ComponentId id) {
    assert(id < components_.size());
    return components_[id];
  }

  /// @brief A helper function for getting a particular Component, given a
  /// component ID.
  ///
  /// @param[in] id The component ID for the desired Component.
  ///
  /// @note Asserts if the id is less than kMaxComponentCount.
  ///
  /// @return Returns a const pointer to the Component at the given id,
  /// which inherits from the ComponentInterface.
  inline const ComponentInterface* GetComponent(ComponentId id) const {
    assert(id < components_.size());
    return components_[id];
  }

  /// @brief Returns the number of components that have been registered
  /// with the entity manager.
  ///
  /// @return The total number of components that are currently registered
  /// with the entity manager.
  inline size_t ComponentCount() const { return components_.size(); }

  /// @brief A helper function to get the component ID for a given Component.
  ///
  /// @tparam T The data type of the Component whose ID should be returned.
  ///
  /// @return Returns the component ID for a given Component.
  template <typename T>
  ComponentId GetComponentId() {
    return ComponentIdLookup<T>::component_id;
  }

  /// @brief Allocates a new Entity (that is registered with no Components).
  ///
  /// @return Returns an EntityRef that points to the new Entity.
  EntityRef AllocateNewEntity();

  /// @brief Deletes an Entity by removing it from the EntityManager's list and
  /// clearing any Component data associated with it.
  ///
  /// @note Deletion is deferred until the end of the frame. If you want to
  /// delete something instantly, use DeleteEntityImmediately.
  ///
  /// @param[in] entity An EntityRef that points to the Entity that will be
  /// deleted at the end of the frame.
  void DeleteEntity(EntityRef entity);

  /// @brief Instantly deletes an Entity.
  ///
  /// @note In general, you should use DeleteEntity (which defers deletion
  /// until the end of the update cycle) unless you have a very good reason
  /// for doing so.
  ///
  /// @param[in] entity An EntityRef that points to the Entity that will be
  /// immediately deleted.
  void DeleteEntityImmediately(EntityRef entity);

  /// @brief Registers a new Component with the EntityManager.
  ///
  /// @tparam T The data type of the Component that is being registered with the
  /// EntityManager.
  ///
  /// @param[in] new_component A Component to be registered with to the
  /// EntityManager.
  ///
  /// @note The new_component must inherit from the ComponentInterface.
  ///
  /// @return Returns the component ID for the new Component.
  template <typename T>
  ComponentId RegisterComponent(T* new_component) {
    static_assert(std::is_base_of<ComponentInterface, T>::value,
                  "'new_component' must inherit from ComponentInterface");
    ComponentId component_id = static_cast<ComponentId>(components_.size());
    ComponentIdLookup<T>::component_id = component_id;
    RegisterComponentHelper(new_component, component_id);
    return component_id;
  }

  /// @brief Removes all Components for an Entity, destroying any data
  /// associated with it.
  ///
  /// @note Normally called by EntityManager prior to deleting an Entity.
  ///
  /// @param[in] entity An EntityRef that points to the Entity whose Components
  /// should be removed.
  void RemoveAllComponents(EntityRef entity);

  /// @brief Iterates through all the registered Components and causes them to
  /// update.
  ///
  /// @param[in] delta_time A WorldTime that represents the timestep since
  /// the last update.
  void UpdateComponents(WorldTime delta_time);

  /// @brief Clears all data from all Components, empties the list
  /// of Components themselves, and then empties the list of Entities.
  /// This basically resets the EntityManager into its original state.
  void Clear();

  /// @brief Returns an iterator to the beginning of the active Entities.
  /// This is suitable for iterating over every active Entity.
  ///
  /// @return Returns a std::iterator to the first active Entity.
  EntityStorageContainer::Iterator begin() { return entities_.begin(); }

  /// @brief Returns an iterator to the last of the active Entities.
  ///
  /// @return Returns a std::iterator to the last active Entity.
  EntityStorageContainer::Iterator end() { return entities_.end(); }

  /// @brief Registers an instance of the EntityFactoryInterface as the
  /// factory object to be used when Entities are created from arbitrary
  /// data. The EntityFactory is responsible for parsing arbitrary data and
  /// correctly transforming it into an Entity.
  ///
  /// @note This should be set before attempting to load Entities from data
  /// files.
  ///
  /// @param[in] entity_factory A pointer to a class that inherits from the
  /// EntityFactoryInterface, which will be registered with the
  /// EntityManager.
  void set_entity_factory(EntityFactoryInterface* entity_factory) {
    entity_factory_ = entity_factory;
  }

  /// @brief Creates an Entity from arbitrary data. This is normally invoked
  /// only by classes that inherit from EntityFactoryInterface.
  ///
  /// Any class implementing CreateEntityFromData should establish the input
  /// format for the 'data' and a way to parse that input 'data' into an Entity.
  ///
  /// @param[in] data A void pointer to the data to be used to create the
  /// Entity.
  ///
  /// @return Returns an EntityRef that points to the newly created Entity.
  EntityRef CreateEntityFromData(const void* data);

  /// @brief Registers an Entity with a Component. This causes the Component
  /// to allocate data for the Entity and includes the Entity in that
  /// Component's update routines.
  ///
  /// @param[in] entity An EntityRef that points to the Entity that should be
  /// registered with the Component.
  /// @param[in] component_id The component ID of the Component that should
  /// be registered with the Entity.
  void AddEntityToComponent(EntityRef entity, ComponentId component_id);

  /// @brief Deletes all the Entities that are marked for deletion.
  ///
  /// @warning Do NOT call this function during any form of Entity update!
  void DeleteMarkedEntities();

 private:
  /// @brief Handles the majority of the work for registering a Component (
  /// aside from some of the template stuff). In particular, it verifies that
  /// the request ID is not already in use, puts a pointer to the new Component
  /// in our components_ array, sets the starting variables, and performs the
  /// initialization on the Component.
  ///
  /// @param[in] new_component A pointer to a class that inherits from the
  /// ComponentInterface, which will be registered with the component_id.
  /// @param[in] component_id The component ID to register with the
  /// new_component.
  void RegisterComponentHelper(ComponentInterface* new_component,
                               ComponentId component_id);

  /// @brief Given a component ID and an Entity, returns all data associated
  /// with that Entity from the given Component.
  ///
  /// @note This function will assert if the inputs are not valid. (e.g. The
  /// Entity is no longer active, or the Component is invalid.)
  ///
  /// @param[in] entity The Entity associated with the desired data.
  /// @param[in] component_id The component ID associated with the desired
  /// data.
  ///
  /// @return Returns the data as a void pointer.
  ///
  /// @note The caller is expected to know how to interpret the data, since it
  /// is returned as a void pointer. Typically a Component is registered with a
  /// struct (or class) of Component data, however this function returns the
  /// data as a void pointer, rather than a specific Component data type.
  void* GetComponentDataAsVoid(EntityRef entity, ComponentId component_id);

  /// @brief Given a component ID and an Entity, returns all data associated
  /// with that Entity from the given Component.
  ///
  /// @note This function will assert if the inputs are not valid. (e.g. The
  /// Entity is no longer active, or the Component is invalid.)
  ///
  /// @param[in] entity The Entity associated with the desired data.
  /// @param[in] component_id The component ID associated with the desired
  /// data.
  ///
  /// @return Returns the data as a void pointer.
  ///
  /// @note The caller is expected to know how to interpret the data, since it
  /// is returned as a void pointer. Typically a Component is registered with a
  /// struct (or class) of Component data, however this function returns the
  /// data as a void pointer, rather than a specific Component data type.
  const void* GetComponentDataAsVoid(EntityRef entity,
                                     ComponentId component_id) const;

  /// @var entities_
  ///
  /// @brief Storage for all the Entities currently tracked by the
  /// EntityManager.
  EntityStorageContainer entities_;

  /// @var components_
  ///
  /// @brief All the Components that are tracked by the system, and are
  /// ready to have Entities added to them.
  std::vector<ComponentInterface*> components_;

  /// @var entities_to_delete_
  ///
  /// @brief A list of all the Entities that we plan to delete at the end of the
  /// frame.
  ///
  /// @note These entities are deleted by a call to DeleteMarkedEntities().
  /// DeleteMarkedEntities should NOT be called called during any form of
  /// Entity update.
  std::vector<EntityRef> entities_to_delete_;

  /// @var entity_factory_
  ///
  /// @brief An EntityFactory used for spawning new Entities from data.
  ///
  /// @note Provided by the calling program.
  EntityFactoryInterface* entity_factory_;

  // Current version of the Corgi Entity Library.
  const CorgiVersion* version_;
};

/// @class EntityFactoryInterface
///
/// @brief An interface for an Entity factory, which creates Entities
/// for a given EntityManager.
class EntityFactoryInterface {
 public:
  /// @brief A destructor of the entity factory interface.
  virtual ~EntityFactoryInterface() {}

  /// @brief Creates an Entity with a given EntityManager, registers it
  /// with all Components specified, and populates the Component data.
  ///
  /// @param[in] data A void pointer to the data used to create
  /// the new Entity.
  /// @param[in] entity_manager A pointer to an EntityManager that
  /// should create the Entity.
  ///
  /// @return Returns an EntityRef pointing to the newly created
  /// Entity.
  virtual EntityRef CreateEntityFromData(const void* data,
                                         EntityManager* entity_manager) = 0;
};
/// @}

}  // corgi

#endif  // CORGI_ENTITY_MANAGER_H_
