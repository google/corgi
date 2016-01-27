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

#ifndef CORGI_COMPONENT_LIBRARY_ENTITY_FACTORY_H_
#define CORGI_COMPONENT_LIBRARY_ENTITY_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "corgi/entity_manager.h"
#include "flatbuffers/flatbuffers.h"
#include "library_components_generated.h"

namespace corgi {
namespace component_library {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @class EntityFactory
///
/// @brief An EntityFactory builds Entities based on prototypes, using
/// FlatBuffers to specify the raw data for Entities.
class EntityFactory : public corgi::EntityFactoryInterface {
 public:
  /// @var kDataTypeNone
  ///
  /// @brief This is equivalent to the `*_NONE` value of the FlatBuffer union
  /// enum.
  static const unsigned int kDataTypeNone = 0;

  /// @var kErrorLoadingEntities
  ///
  /// @brief The return value for `LoadEntitiesFromFile` and
  /// `LoadEntityListFromMemory` if the methods are unable to read the
  /// Entity list.
  static const int kErrorLoadingEntities = -1;

  /// @brief The default constructor for an empty EntityFactory.
  EntityFactory()
      : max_component_id_(0),
        debug_entity_creation_(false) {}
  virtual ~EntityFactory() {}

  /// @brief Add a file from which you can load Entity prototype definitions.
  ///
  /// @param[in] entity_library_file A UTF-8 C-string representing the
  /// filename of the Entity library file that contains the prototype
  /// definitions.
  ///
  /// @return Returns `true` if the file was added successfully. Otherwise it
  /// returns `false`.
  bool AddEntityLibrary(const char* entity_library_file);

  /// @brief Check if the given pointer points to something that will be kept
  /// in memory for the lifetime of the EntityFactory.
  ///
  /// @param[in] pointer A void pointer that should be checked if its data
  /// will be kept in memory.
  ///
  /// @return Returns `true` if the given pointer points to something that will
  /// be kept in memory for the of the EntityFactory. Otherwise it returns
  /// `false`.
  bool WillBeKeptInMemory(const void* pointer);

  /// @brief Load Entities from a given Entity list file.
  ///
  /// @param[in] filename A UTF-8 C-string representing the filename of the
  /// Entity list file to load the Entities from.
  /// @param[in, out] entity_manager An EntityManager pointer to the
  /// EntityManager that should load all the Entities from `filename`.
  ///
  /// @return Returns the number of Entities that were loaded, or returns
  /// `kErrorLoadingEntities` if there was en error.
  virtual int LoadEntitiesFromFile(const char* filename,
                                   corgi::EntityManager* entity_manager);

  /// @brief Loads a list of Entities from an in-memory buffer.
  ///
  /// @note This is a helper method used by `LoadEntitiesFromFile` to do a lot
  /// of the work actually loading the Entities.
  ///
  /// @param[in] raw_entity_list A void pointer to the Entity list.
  /// @param[in, out] entity_manager An EntityManager pointer to the
  /// EntityManager that should load all the Entities from memory.
  /// @param[out] entities_loaded An optional parameter to obtain the list of
  /// the loaded Entities. Pass in a pointer to a std::vector of EntityRefs to
  /// capture this output. Otherwise, pass in `NULL` to ignore this parameter.
  ///
  /// @return Returns the number of Entities that were loaded, or returns
  /// `kErrorLoadingEntities` if there was an error.
  int LoadEntityListFromMemory(const void* raw_entity_list,
                               corgi::EntityManager* entity_manager,
                               std::vector<corgi::EntityRef>* entities_loaded);

  /// @brief Override a cached file with data from memory that will persist
  /// until exit.
  ///
  /// @param[in] filename A UTF-8 C-string representing the filename of the
  /// cached file whose data should be overridden.
  /// @param[in] new_data A std::unique_ptr that points to a std::string of
  /// data to override the cached file with.
  void OverrideCachedFile(const char* filename,
                          std::unique_ptr<std::string> new_data);

  /// @brief Initialize an Entity from an Entity definition.
  ///
  /// @note This helper method is called by `LoadRawEntityList` for each Entity
  /// definition.
  ///
  /// @param[in] data A void pointer to the data to create an Entity from.
  /// @param[in,out] entity_manager An EntityManager pointer to the
  /// EntityManager that should load the Entity from the `data`.
  ///
  /// @return Returns an EntityRef to the new Entity that was created.
  corgi::EntityRef CreateEntityFromData(const void* data,
                                        corgi::EntityManager* entity_manager);

  /// @brief Initialize an entity from a given prototype.
  ///
  /// @param[in] prototype_name A C-string containing the name of the prototype
  /// to initialize the Entity with.
  /// @param[in,out] entity_manager An EntityManager pointer to the
  /// EntityManager that should create the initialized Entity.
  ///
  /// @return Returns an EntityRef to the new Entity that was initialized
  /// from the prototype.
  virtual corgi::EntityRef CreateEntityFromPrototype(
      const char* prototype_name, corgi::EntityManager* entity_manager);

  /// @brief When you register each Component with the Entity system, it will
  /// get a component ID. This factory needs to know the Component ID assigned
  /// for each Component data type (the data_type() in the flatbuffer union).
  ///
  /// @param[in] component_id The ComponentId of the Component in the Entity
  /// system.
  /// @param[in] data_type An enum for the data type of the Component definition
  /// within the FlatBuffer union.
  /// @param[in] table_name A C-string of the table name of the FlatBuffer
  /// schema table for the Component definition.
  void SetComponentType(corgi::ComponentId component_id, unsigned int data_type,
                        const char* table_name);

  /// @brief Get the Component Id for a given data type specifier.
  ///
  /// @param[in] data_type An int corresponding to the data type specifier.
  ///
  /// @return Returns a ComponentId for the Component with a data type
  /// of `data_type`.
  corgi::ComponentId DataTypeToComponentId(unsigned int data_type) {
    if (data_type >= data_type_to_component_id_.size())
      return corgi::kInvalidComponent;
    return data_type_to_component_id_[data_type];
  }

  /// @brief Get the data type specifier for a Component, given a Component ID.
  ///
  /// @param[in] component_id The ComponentId of the Component whose data type
  /// specifier should be returned.
  ///
  /// @return Returns an int corresponding to the data type specifier.
  unsigned int ComponentIdToDataType(corgi::ComponentId component_id) {
    if (component_id >= component_id_to_data_type_.size()) return kDataTypeNone;
    return component_id_to_data_type_[component_id];
  }

  /// @brief Get the table name for a Component, given a Component ID.
  ///
  /// @param[in] component_id The ComponentId of the Component whose table name
  /// should be returned.
  ///
  /// @return Returns a C-string of the FlatBuffer schema table name for the
  /// Component.
  const char* ComponentIdToTableName(corgi::ComponentId component_id) {
    if (component_id >= component_id_to_table_name_.size()) return "";
    return component_id_to_table_name_[component_id].c_str();
  }

  /// @brief Serialize an Entity into whatever binary type you are using
  /// for them.
  ///
  /// @note Calls `CreateEntityDefinition()`, which you implement, to do the
  /// work.
  ///
  /// @param[in] entity The Entity that should be serialized.
  /// @param[in] entity_manager The EntityManager responsible for the Entity
  /// that should be serialized.
  /// @param[out] entity_serialized_output A vector to capture the output of the
  /// Entity definition.
  ///
  /// @return Returns `true` if the Entity definition was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool SerializeEntity(corgi::EntityRef& entity,
                               corgi::EntityManager* entity_manager,
                               std::vector<uint8_t>* entity_serialized_output);

  /// @brief After you call `SerializeEntity` on a few Entities, you may call
  /// this method to put them into a proper list.
  ///
  /// @note: Calls `CreateEntityList()`, which you implement.
  ///
  /// @param[in] entity_definitions A reference to a vector list of Entity
  /// definitions (vector<uint8_t>) that should be put into a proper Entity
  /// list.
  /// @param[out] entity_list_serialized A vector to capture the output of the
  /// Entity list.
  ///
  /// @return Returns `true` if the Entity list was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool SerializeEntityList(
      const std::vector<std::vector<uint8_t>>& entity_definitions,
      std::vector<uint8_t>* entity_list_serialized);

  /// @brief Get the maximum component ID.
  ///
  /// @return Returns the highest component ID.
  corgi::ComponentId max_component_id() { return max_component_id_; }

  /// @brief Check if debug logging is enabled.
  ///
  /// @return Returns `true` if debug logging is enabled. Otherwise, it
  /// returns `false`.
  bool debug_entity_creation() const { return debug_entity_creation_; }

  /// @brief Enable debug logging during Entity creation.
  ///
  /// @param[in] b A bool determining if logging should be enabled.
  void set_debug_entity_creation(bool b) { debug_entity_creation_ = b; }

  // Here are the virtual methods you must override to create your own entity
  // factory. They are mainly the ones concerned with reading and writing your
  // application's specific Flatbuffer format.

  /// @brief Handles reading an Entity list and extracting the individual
  /// Entity data definitions.
  ///
  /// @note You MUST override this function to create your own EntityFactory.
  ///
  /// @param[in] entity_list A const void pointer to the start of the list
  /// of Entities.
  /// @param[out] entity_defs A vector that captures the output of the
  /// extracted entity data definitions.
  ///
  /// @return returns `true` if the list was parsed successfully. Otherwise it
  /// returns `false`.
  virtual bool ReadEntityList(const void* entity_list,
                              std::vector<const void*>* entity_defs) = 0;

  /// @brief Handles reading an Entity definition and extracting the individual
  /// Component data definitions.
  ///
  /// In your output, you should index `component_defs` by component ID, and
  /// any Components not specified by this Entity's definition should be set to
  /// `nullptr`.
  ///
  /// @note You MUST override this function to create your own EntityFactory.
  ///
  /// @param[in] entity_definition A const void pointer to the Entity definition
  /// whose Component data definitions should be extracted.
  /// @param[out] component_defs A vector that captures the output of
  /// the extracted Component data definitions.
  ///
  /// @return Returns `true` if the `entity_definition` was parsed successfully.
  /// Otherwise it returns `false`.
  virtual bool ReadEntityDefinition(
      const void* entity_definition,
      std::vector<const void*>* component_defs) = 0;

  /// @brief Creates an Entity list that contains a single Entity definition,
  /// which contains a single Component definition (a `MetaDef` with `prototype`
  /// set to the requested prototype name).
  ///
  /// @note You MUST override this function to create your own EntityFactory.
  ///
  /// @param[in] prototype_name A C-string name of the prototype, which is used
  /// to set the `prototype` field in the MetaComponent.
  /// @param[out] request A vector to capture the output of the prototype
  /// request.
  ///
  /// @return Returns `true` if the request was created successfully. Otherwise
  /// it returns `false`.
  virtual bool CreatePrototypeRequest(const char* prototype_name,
                                      std::vector<uint8_t>* request) = 0;

  /// @brief Handles building a single Entity definition flatbuffer from a list
  /// of an Entity's Component definitions.
  ///
  /// @note You MUST override this function to create your own EntityFactory.
  ///
  /// @param[in] component_data A const reference to a std::vector that contains
  /// the list of the Entity's Component definitions, which can be indexed by
  /// Component ID.
  /// @param[out] entity_definition A vector to capture the output of the Entity
  /// definition.
  ///
  /// @return Returns `true` if the Entity definition was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool CreateEntityDefinition(
      const std::vector<const void*>& component_data,
      std::vector<uint8_t>* entity_definition) = 0;

  /// @brief Handles building an Entity list flatbuffer from a collection of
  /// individual Entity flatbuffers.
  ///
  /// @note You MUST override this function to create your own EntityFactory.
  ///
  /// @param[in] entity_defs A const reference to a std::vector that contains
  /// all the Entity flatbuffers.
  /// @param[out] entity_list A vector to capture the output of the Entity
  /// list.
  ///
  /// @return Returns `true` if the Entity list was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool CreateEntityList(const std::vector<const void*>& entity_defs,
                                std::vector<uint8_t>* entity_list) = 0;

  // You MAY override this function, which takes a binary list of entities and
  // outputs a human-readable text version (aka JSON) of the entity list.
  //
  // Takes a void* and outputs a std::string containing the human readable
  // entity file.
  //
  // TODO: Implement this function using reflection.

  /// @brief Creates an Entity from the Entity definition, recursively building
  /// up from the prototypes.
  ///
  /// @note: You MAY override this function if you wish to change the way that
  /// prototyping works.
  ///
  /// @param[in] def A const void pointer to the Entity definition that will
  /// be used to build the prototypes.
  /// @param[in,out] entity_manager A pointer to the EntityManager that the
  /// Entity will be created with.
  /// @param[out] entity A reference to the Entity that is being created.
  /// @param[in] is_prototype A bool determining if this is a prototype.
  virtual void LoadEntityData(const void* def,
                              corgi::EntityManager* entity_manager,
                              corgi::EntityRef& entity, bool is_prototype);

  /// @brief This factory and its subclasses need to know how to parse the
  /// Entity FlatBuffers using reflection.
  ///
  /// You will need to specify the .bfbs file for your Component data type.
  ///
  /// @param[in] binary_schema_filename A C-string of the filename for the
  /// FlatBuffer schema file.
  void SetFlatbufferSchema(const char* binary_schema_filename);

  /// @brief Get the FlatBuffer binary schema that you loaded
  /// with `SetFlatbufferSchema()`.
  ///
  /// @return Returns a const reference to the std::string name
  /// of the FlatBuffer schema.
  const std::string& flatbuffer_binary_schema_data() const {
    return flatbuffer_binary_schema_data_;
  }

  /// @brief Get the map with all the current prototypes.
  ///
  /// @return Returns a const reference to the std::unordered_map with the
  /// prototype data
  const std::unordered_map<std::string, const void*>& prototype_data() const {
    return prototype_data_;
  }

 private:
  // Storage for the Flatbuffer binary schema.
  std::string flatbuffer_binary_schema_data_;

  // Storage for entity files.
  std::unordered_map<std::string, std::unique_ptr<std::string>> loaded_files_;

  // Storage for old entity files we don't need any more; these will be cleared
  // out once there are no entities referencing them.
  std::vector<std::unique_ptr<std::string>> stale_files_;

  // Index the prototype library's entity definitions by prototype name.
  std::unordered_map<std::string, const void*> prototype_data_;

  // Keep a list of previously-built entity definitions that we use to
  // create an entity by prototype name.
  std::unordered_map<std::string, std::vector<uint8_t>> prototype_requests_;

  // Look up ComponentId from data type, and vice versa.
  std::vector<corgi::ComponentId> data_type_to_component_id_;
  std::vector<unsigned int> component_id_to_data_type_;

  // Look up the table name from the data type.
  std::vector<std::string> component_id_to_table_name_;

  // The highest component ID we've seen registered.
  corgi::ComponentId max_component_id_;

  // Enable debug output for initializing entities.
  bool debug_entity_creation_;
};
/// @}

}  // namespace component_library
}  // namespace corgi

#endif  // CORGI_COMPONENT_LIBRARY_ENTITY_FACTORY_H_
