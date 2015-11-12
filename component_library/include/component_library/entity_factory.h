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

#ifndef ENTITY_FACTORY_H_
#define ENTITY_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "entity/entity_manager.h"
#include "flatbuffers/flatbuffers.h"
#include "library_components_generated.h"

namespace corgi {
namespace component_library {

// An EntityFactory that builds entities based on prototypes, using FlatBuffers
// to specify the raw data for entities.

class EntityFactory : public corgi::EntityFactoryInterface {
 public:
  // Equivalent to the "*_NONE" value of the Flatbuffer union enum.
  static const unsigned int kDataTypeNone = 0;

  // Return value for LoadEntitiesFromFile and LoadEntityListFromMemory if
  // unable to read the entity list.
  static const int kErrorLoadingEntities = -1;

  EntityFactory()
      : max_component_id_(corgi::kInvalidComponent),
        debug_entity_creation_(false) {}

  // Add a file from which you can then load entity prototype definitions.
  bool AddEntityLibrary(const char* entity_library_file);

  // Returns true if this pointer points to something we will be keeping
  // in memory for you. If true, this pointer will remain valid for the
  // lifetime of the EntityFactory.
  bool WillBeKeptInMemory(const void* pointer);

  // Load entities from an entity list file.
  // Returns the number of entities loaded, or kErrorLoadingEntities if there
  // was an error.
  virtual int LoadEntitiesFromFile(const char* filename,
                                   corgi::EntityManager* entity_manager);

  // Load a list of entities from an in-memory buffer.
  // This is what LoadEntitiesFromFile calls to do the heavy lifting.
  // If you want a list of the loaded entities, pass in a vector to fill in;
  // otherwise, just pass in null.
  // Returns the number of entities loaded, or kErrorLoadingEntities if there
  // was an error.
  int LoadEntityListFromMemory(const void* raw_entity_list,
                               corgi::EntityManager* entity_manager,
                               std::vector<corgi::EntityRef>* entities_loaded);

  // Override a cached file with data from memory; this will persist until exit.
  void OverrideCachedFile(const char* filename,
                          std::unique_ptr<std::string> new_data);

  // Initialize an entity from an entity definition. This is what
  // LoadRawEntityList calls for each entity definition.
  corgi::EntityRef CreateEntityFromData(const void* data,
                                         corgi::EntityManager* entity_manager);

  // Initialize an entity by prototype name.
  virtual corgi::EntityRef CreateEntityFromPrototype(
      const char* prototype_name, corgi::EntityManager* entity_manager);

  // When you register each component to the entity system, it will get
  // a component ID. This factory needs to know the component ID assigned
  // for each component data type (the data_type() in the flatbuffer union).
  void SetComponentType(corgi::ComponentId component_id,
                        unsigned int data_type, const char* table_name);

  // Get the component for a given data type specifier.
  corgi::ComponentId DataTypeToComponentId(unsigned int data_type) {
    if (data_type >= data_type_to_component_id_.size())
      return corgi::kInvalidComponent;
    return data_type_to_component_id_[data_type];
  }
  // Get the data type specifier for a given component.
  unsigned int ComponentIdToDataType(corgi::ComponentId component_id) {
    if (component_id >= component_id_to_data_type_.size()) return kDataTypeNone;
    return component_id_to_data_type_[component_id];
  }

  // Get the data type specifier for a given component.
  const char* ComponentIdToTableName(corgi::ComponentId component_id) {
    if (component_id >= component_id_to_table_name_.size()) return "";
    return component_id_to_table_name_[component_id].c_str();
  }

  // Serialize an entity into whatever binary type you are using for them.
  // Calls CreateEntityDefinition(), which you implement, to do the work.
  virtual bool SerializeEntity(corgi::EntityRef& entity,
                               corgi::EntityManager* entity_manager,
                               std::vector<uint8_t>* entity_serialized_output);

  // After you call SerializeEntity on a few entities, you'll probably want
  // to put them in a proper list. This function does that, via
  // a CreateEntityList() that you implement.
  virtual bool SerializeEntityList(
      const std::vector<std::vector<uint8_t>>& entity_definitions,
      std::vector<uint8_t>* entity_list_serialized);

  corgi::ComponentId max_component_id() { return max_component_id_; }

  bool debug_entity_creation() const { return debug_entity_creation_; }
  void set_debug_entity_creation(bool b) { debug_entity_creation_ = b; }

  // Here are the virtual methods you must override to create your own entity
  // factory. They are mainly the ones concerned with reading and writing your
  // application's specific Flatbuffer format.

  // You MUST override this function, which handles reading an entity list
  // and extracting the individual entity data definitions.
  // Return true if you parsed the list successfully or false if not.
  virtual bool ReadEntityList(const void* entity_list,
                              std::vector<const void*>* entity_defs) = 0;

  // You MUST override this function, which handles reading an entity definition
  // and extracting the individual component data definitions. In your output,
  // you should index component_defs by component ID, and any components not
  // specified by this entity's definition should be set to nullptr. Return
  // true if you were able to parse the entity definition, or false if not.
  virtual bool ReadEntityDefinition(
      const void* entity_definition,
      std::vector<const void*>* component_defs) = 0;

  // You MUST override this function, which creates an entity list that contains
  // a single entity definition, which contains a single component definition:
  // an MetaDef with "prototype" set to the requested prototype name.
  virtual bool CreatePrototypeRequest(const char* prototype_name,
                                      std::vector<uint8_t>* request) = 0;

  // You MUST override this function, which handles building a single entity
  // definition flatbuffer from a list of entity component definitions.
  //
  // Takes vector<void*> indexed by component ID, outputs a vector<uint8_t>
  // containing a single EntityDef
  virtual bool CreateEntityDefinition(
      const std::vector<const void*>& component_data,
      std::vector<uint8_t>* entity_definition) = 0;

  // You MUST override this function, which handles building a entity list
  // flatbuffer from a collection of individual entity flatbuffers.
  //
  // Takes vector<void*> containing all the entities, outputs a vector<uint8_t>
  // containing a flatbuffer with a single EntityDefList in it.
  virtual bool CreateEntityList(const std::vector<const void*>& entity_defs,
                                std::vector<uint8_t>* entity_list) = 0;

  // You MAY override this function, which takes a binary list of entities and
  // outputs a human-readable text version (aka JSON) of the entity list.
  //
  // Takes a void* and outputs a std::string containing the human readable
  // entity file.
  //
  // TODO: Implement this function using reflection.

  // Creates an entity from the entity definition, recursively building up from
  // the prototypes. Returns true if successful or false if not.
  //
  // You MAY override this if you want to change the way prototyping works.
  virtual void LoadEntityData(const void* def,
                              corgi::EntityManager* entity_manager,
                              corgi::EntityRef& entity, bool is_prototype);

  // This factory and its subclasses need to know how to parse the entity
  // flatbuffers using reflection.
  //
  // You will need to specify the .bfbs file for your component data type.
  void SetFlatbufferSchema(const char* binary_schema_filename);

  // Get the Flatbuffer binary schema that you loaded with
  // SetFlatbufferSchema().
  const std::string& flatbuffer_binary_schema_data() const {
    return flatbuffer_binary_schema_data_;
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

}  // namespace component_library
}  // namespace corgi

#endif  // ENTITY_FACTORY_H_
