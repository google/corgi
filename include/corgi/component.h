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

#ifndef CORGI_COMPONENT_H_
#define CORGI_COMPONENT_H_

#include <unordered_map>
#include "corgi/component_id_lookup.h"
#include "corgi/component_interface.h"
#include "corgi/entity.h"
#include "corgi/entity_common.h"
#include "corgi/entity_manager.h"
#include "corgi/vector_pool.h"

namespace corgi {

/// @file
/// @addtogroup corgi_component
/// @{
///
/// @class Component
/// @brief A Component is an object that encapsulates all data and logic
/// for Entities of a particular type.
///
/// @note On some of the API, EntityRef& parameters are non-const so that
/// the referenced Entity can be changed. The EntityRef itself is never
/// modified.
///
/// @tparam T The structure of data that needs to be associated with each
/// Entity.
template <typename T>
class Component : public ComponentInterface {
 public:
  /// @struct ComponentData
  /// @brief A structure of data that is associated with each Entity.
  ///
  /// It contains the template struct, as well as a pointer back to the
  /// Entity that owns this data.
  struct ComponentData {
    /// @brief The default constructor for an empty ComponentData.
    ComponentData() {}

    /// @var entity
    ///
    /// @brief The Entity associated with this data.
    EntityRef entity;

    /// @var data
    ///
    /// @brief The data to associate with the Entity.
    T data;

    /// @brief Construct a new ComponentData from an existing ComponentData.
    ///
    /// @param[in] src An existing ComponentData whose data should be moved
    /// into the new ComponentData.
    ComponentData(ComponentData&& src) {
      entity = std::move(src.entity);
      data = std::move(src.data);
    }

    /// @brief Move a referenced ComponentData into this ComponentData.
    ///
    /// Move operators are efficient since they allow referenced data to
    /// be moved instead of copied.
    ///
    /// @param[in] src A referenced ComponentData to be moved into this
    /// ComponentData.
    ComponentData& operator=(ComponentData&& src) {
      entity = std::move(src.entity);
      data = std::move(src.data);
      return *this;
    }

   private:
    ComponentData(const ComponentData&);
    ComponentData& operator=(const ComponentData&);
  };

  /// @typedef EntityIterator
  ///
  /// @brief An iterator to iterate through all of the Entities in the
  /// Component.
  typedef typename VectorPool<ComponentData>::Iterator EntityIterator;

  /// @typedef value_type
  ///
  /// @brief The templated data type stored by this Component.
  typedef T value_type;

  /// @brief Construct a Component without an EntityManager.
  Component() : entity_manager_(nullptr) {}

  /// @brief Destructor for a Component.
  virtual ~Component() {}

  /// @brief Provides an alternate way to add Entities if you do not
  /// care about the returned data structure, and if you do not feel like
  /// casting the BaseComponent into something more specific.
  ///
  /// @note AddEntity is a much better function, except we cannot have it in
  /// the interface class, since it needs to know about type T for the return
  /// value.
  ///
  /// @param[in,out] entity An EntityRef reference used to add an Entity.
  virtual void AddEntityGenerically(EntityRef& entity) { AddEntity(entity); }

  /// @brief Adds an Entity to the list that this Component is tracking.
  ///
  /// @param[in,out] entity An EntityRef reference used to add an Entity to the
  /// list of Entities that this Component keeps track of.
  /// @param[in] alloc_location An Enum that specifies whether to allocate from
  /// the beginning or end of the memory pool.
  ///
  /// @return Returns the data structure associated with the Component.
  ///
  /// @note If you have already registered for this Component, this
  /// will just return a reference to the existing data and will not change
  /// anything.
  T* AddEntity(EntityRef& entity, AllocationLocation alloc_location) {
    if (HasDataForEntity(entity)) {
      return GetComponentData(entity);
    }
    // No existing data, so we allocate some and return it:
    ComponentIndex index = static_cast<ComponentIndex>(
        component_data_.GetNewElement(alloc_location).index());
    component_index_lookup_[entity->entity_id()] = index;
    ComponentData* component_data = component_data_.GetElementData(index);
    component_data->entity = entity;
    InitEntity(entity);
    return &(component_data->data);
  }

  /// @brief Adds an Entity to the list that this Component is tracking.
  ///
  /// @note Entities added through this function allocate from the back of the
  /// memory pool.
  ///
  /// @param[in,out] entity An EntityRef reference used to add an Entity to the
  /// list of Entities that this Component keeps track of.
  ///
  /// @return Returns the data structure associated with the Component.
  T* AddEntity(EntityRef& entity) { return AddEntity(entity, kAddToBack); }

  /// @brief Removes an Entity from the list of Entities.
  ///
  /// This is done by marking the Entity as no longer using this Component,
  /// calling the destructor on the data, and returning the memory to the
  /// memory pool.
  ///
  /// @param[in,out] entity An EntityRef reference used to remove an Entity
  /// from the list of Entities that this Component keeps track of.
  virtual void RemoveEntity(EntityRef& entity) {
    // Calling remove when there is no data is generally a sign that
    // something has gone wrong and that something has lost track of which
    // entities are associated with which components.  Use HasDataForEntity()
    // if you want to double-check if data exists before removing it.
    assert(HasDataForEntity(entity));
    RemoveEntityInternal(entity);
    component_data_.FreeElement(GetComponentDataIndex(entity));
    component_index_lookup_.erase(entity->entity_id());
  }

  /// @brief Removes an Entity from the list of Entities.
  ///
  /// This is done by marking the Entity as no longer using this Component,
  /// calling the destructor on the data, and returning the memory to the
  /// memory pool.
  ///
  /// @param[in,out] iter An EntityIterator that references an Entity that
  /// should
  /// be removed.
  ///
  /// @return Returns an iterator to the next Entity after the one that was
  /// just removed.
  virtual EntityIterator RemoveEntity(EntityIterator iter) {
    EntityRef entity = iter->entity;
    RemoveEntityInternal(entity);
    EntityIterator new_iter = component_data_.FreeElement(iter);
    component_index_lookup_.erase(entity->entity_id());
    return new_iter;
  }

  /// @brief Gets an iterator that will iterate over every Entity associated
  /// with the Component, starting from the beginning.
  ///
  /// @return Returns an iterator in the style of std that points to the first
  /// Entity in the list of all Entities in the Component.
  virtual EntityIterator begin() { return component_data_.begin(); }

  /// @brief Gets an iterator that points to the end of the list of all entites
  /// in the Component.
  ///
  /// @return Returns an iterator in the style of std that points to the last
  /// Entity in the list of all Entities in the Component.
  virtual EntityIterator end() { return component_data_.end(); }

  /// @brief Updates all Entities. This is normally called, once per frame,
  /// by the EntityManager.
  virtual void UpdateAllEntities(WorldTime /*delta_time*/) {}

  /// @brief Checks if this component contains any data associated with the
  /// supplied entity.
  virtual bool HasDataForEntity(const EntityRef& entity) {
    return GetComponentDataIndex(entity) != kInvalidComponentIndex;
  }

  /// @brief Gets the data for a given Entity as a void pointer.
  ///
  /// @note When using GetComponentDataAsVoid, the calling function is expected
  /// to know how to handle the data (since it is returned as a void pointer).
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose data should
  /// be returned.
  ///
  /// @return Returns the Entity's data as a void pointer, or returns a nullptr
  /// if the data does not exist.
  virtual void* GetComponentDataAsVoid(const EntityRef& entity) {
    return GetComponentData(entity);
  }

  /// @brief Gets the data for a given Entity as a const void pointer.
  ///
  /// @note When using GetComponentDataAsVoid, the calling function is expected
  /// to know how to handle the data (since it is returned as a const
  /// void pointer).
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose data should
  /// be returned.
  ///
  /// @return Returns the Entity's data as a const void pointer, or returns a
  /// nullptr if the data does not exist.
  virtual const void* GetComponentDataAsVoid(const EntityRef& entity) const {
    return GetComponentData(entity);
  }

  /// @brief Gets the Component data stored at a given index.
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] data_index A size_t representing the index of the desired
  /// Component data.
  ///
  /// @return Returns a pointer of the data structure associated with the
  /// Component data, or returns a nullptr if given an invalid data_index.
  T* GetComponentData(size_t data_index) {
    if (data_index == kInvalidComponentIndex) {
      return nullptr;
    }
    ComponentData* element_data = component_data_.GetElementData(data_index);
    return (element_data != nullptr) ? &(element_data->data) : nullptr;
  }

  /// @brief Gets the data for a given Entity.
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose data should
  /// be returned.
  ///
  /// @return Returns the Entity's data as a pointer of the data structure
  /// associated with the Component data, or returns a nullptr if the data
  /// does not exist.
  T* GetComponentData(const EntityRef& entity) {
    size_t data_index = GetComponentDataIndex(entity);
    if (data_index >= component_data_.Size()) {
      return nullptr;
    }
    return GetComponentData(data_index);
  }

  /// @brief Gets the Component data stored at a given index.
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] data_index A size_t representing the index of the desired
  /// Component data.
  ///
  /// @return Returns a const pointer of the data structure associated with the
  /// Component data, or returns a nullptr if given an invalid data_index.
  const T* GetComponentData(size_t data_index) const {
    return const_cast<Component*>(this)->GetComponentData(data_index);
  }

  /// @brief Gets the data for a given Entity.
  ///
  /// @warning This pointer is NOT stable in memory. Calls to AddEntity and
  /// AddEntityGenerically may force the storage class to resize,
  /// shuffling around the location of this data.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose data should
  /// be returned.
  ///
  /// @return Returns the Entity's data as a const pointer of the data
  /// structure associated with the Component data, or returns a nullptr
  /// if the data does not exist.
  const T* GetComponentData(const EntityRef& entity) const {
    return const_cast<Component*>(this)->GetComponentData(entity);
  }

  /// @brief Clears all tracked Component data.
  void virtual ClearComponentData() {
    for (auto iter = component_data_.begin(); iter != component_data_.end();
         iter = RemoveEntity(iter)) {
    }
  }

  /// @brief A utility function for retrieving the Component data for an
  /// Entity from a specific Component.
  ///
  /// @tparam ComponentDataType The data type of the Component whose data
  /// is returned for the given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose Component
  /// data should be returned.
  ///
  /// @return Returns a pointer to the data for the Component of the given
  /// Entity or returns null if the Entity is not registered with the
  /// Component.
  template <typename ComponentDataType>
  ComponentDataType* Data(const EntityRef& entity) {
    return entity_manager_->GetComponentData<ComponentDataType>(entity);
  }

  /// @brief A utility function for checking if an entity is registered with
  /// a particular component.
  ///
  /// @tparam ComponentDataType The data type of the Component to be checked
  /// for registration.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose Component
  /// data is checked.
  ///
  /// @return Returns true if the entity has been registered with the Component,
  /// false otherwise.
  template <typename ComponentDataType>
  bool IsRegisteredWithComponent(const EntityRef& entity) {
    return entity_manager_
        ->GetComponent(entity_manager_->GetComponentId<ComponentDataType>())
        ->HasDataForEntity(entity);
  }

  /// @brief A utility function for retrieving the Component data for an
  /// Entity from a specific Component.
  ///
  /// @tparam ComponentDataType The data type of the Component whose data
  /// is returned for the given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose Component
  /// data should be returned.
  ///
  /// @return Returns a pointer to the data for the Component of the given
  /// Entity or returns null if the Entity is not registered with the
  /// Component.
  template <typename ComponentDataType>
  ComponentDataType* Data(const EntityRef& entity) const {
    return entity_manager_->GetComponentData<ComponentDataType>(entity);
  }

  /// @brief A utility function for retrieving a reference to a specific
  /// Component object, by type.
  ///
  /// @tparam ComponentDataType The data type of the Component.
  ///
  /// @return Returns a pointer to the data for a specific Component.
  template <typename ComponentDataType>
  ComponentDataType* GetComponent() {
    return static_cast<ComponentDataType*>(entity_manager_->GetComponent(
        ComponentIdLookup<ComponentDataType>::component_id));
  }

  // Virtual methods we inherited from component_interface:

  /// @brief Override this function with code that should be executed when
  /// the Component is added to the EntityManager. (This typically
  /// happens once, at the beginning of the game before any Entities are
  /// added.)
  virtual void Init() {}

  /// @brief Override this function with code that should be executed when an
  /// Entity is added to the Component.
  virtual void InitEntity(EntityRef& /*entity*/) {}

  /// @brief Override this function to return raw data that can be read back
  /// later.
  ///
  /// @warning By default, Components do not support this functionality. If you
  /// wish to support this, you will need to override this function.
  ///
  /// @return By default, this returns a nullptr.
  virtual RawDataUniquePtr ExportRawData(const EntityRef& /*unused*/) const {
    return nullptr;
  }

  /// @brief Override this function with any code that executes when this
  /// Component is removed from the EntityManager. (i.e. Usually when the
  /// game/state is over and everything is shutting down.)
  virtual void Cleanup() {}

  /// @brief Override this function with any code that needs to be executed
  /// when an Entity is removed from this Component.
  virtual void CleanupEntity(EntityRef& /*entity*/) {}

  /// @brief Set the EntityManager for this Component.
  ///
  /// @note The EntityManager is used as the main point of contact
  /// for Components that need to talk to other things.
  ///
  /// @param[in] entity_manager A pointer to an EntityManager to associate
  /// with this Component.
  virtual void SetEntityManager(EntityManager* entity_manager) {
    entity_manager_ = entity_manager;
  }

  /// @brief Get the ID for this Component.
  ///
  /// @return Returns the Component ID for this Component.
  static ComponentId GetComponentId() {
    return static_cast<ComponentId>(ComponentIdLookup<T>::component_id);
  }

  /// @brief Sets the Component ID on the data type.
  ///
  /// @note This is usually only called by the EntityManager.
  ///
  /// @param[in] id The Component ID to set on the data type.
  virtual void SetComponentIdOnDataType(ComponentId id) {
    ComponentIdLookup<T>::component_id = id;
  }

 private:
  /// @brief Allows Components to handle any per-Entity clean up that may
  /// be needed.
  ///
  /// @param[in] entity An EntityRef reference to the Entity that is being
  /// removed and may need to be cleaned up.
  void RemoveEntityInternal(EntityRef& entity) { CleanupEntity(entity); }

 protected:
  /// @brief Get the index of the Component data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity whose data
  /// index will be returned.
  ///
  /// @return Returns a size_t corresponding to the index of the
  /// Component data, or kInvalidComponentIndex if no data could be found.
  size_t GetComponentDataIndex(const EntityRef& entity) const {
    auto result = component_index_lookup_.find(entity->entity_id());
    return (result != component_index_lookup_.end()) ? result->second
                                                     : kInvalidComponentIndex;
  }

  /// @var component_data_
  ///
  /// @brief Storage for all of the data for the Component.
  VectorPool<ComponentData> component_data_;

  /// @var entity_manager_
  ///
  /// @brief A pointer to the EntityManager for this Component. This is the
  /// main point of contact for Components that need to talk to other things.
  EntityManager* entity_manager_;

  /// @var component_index_lookup_
  ///
  /// @brief A map, for translating unique entity IDs into vectorpool
  /// indexes.
  std::unordered_map<EntityIdType, ComponentIndex> component_index_lookup_;
};
/// @}

}  // corgi

#endif  // CORGI_COMPONENT_H_
