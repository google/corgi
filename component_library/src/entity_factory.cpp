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

#include <set>

#include "corgi_component_library/entity_factory.h"
#include "corgi_component_library/meta.h"
#include "fplbase/utilities.h"

namespace corgi {
namespace component_library {

using component_library::MetaComponent;
using component_library::MetaData;
using corgi::MetaDef;
using fplbase::LogError;
using fplbase::LogInfo;

bool EntityFactory::AddEntityLibrary(const char* entity_library_filename) {
  std::string* library_data = new std::string;
  if (!fplbase::LoadFile(entity_library_filename, library_data)) {
    LogInfo("EntityFactory: Couldn't load entity library %s",
            entity_library_filename);
    delete library_data;
    return false;
  }
  loaded_files_[entity_library_filename].reset(library_data);

  std::vector<const void*> entities;
  if (!ReadEntityList(loaded_files_[entity_library_filename]->c_str(),
                      &entities)) {
    LogInfo("EntityFactory: Couldn't read entity library list '%s'",
            entity_library_filename);
    return false;
  }
  if (debug_entity_creation()) {
    LogInfo("EntityFactory: Reading %d prototypes from file %s",
            entities.size(), entity_library_filename);
  }
  unsigned int meta_id = MetaComponent::GetComponentId();

  for (size_t i = 0; i < entities.size(); i++) {
    std::vector<const void*> components;
    if (!ReadEntityDefinition(entities[i], &components)) {
      LogInfo("EntityFactory: Library entity %d read error, skipping", i);
      continue;  // try reading the rest of the prototypes
    }

    // Read the name from the entity component definition.
    const MetaDef* meta_def = static_cast<const MetaDef*>(components[meta_id]);
    if (meta_def == nullptr || meta_def->entity_id() == nullptr) {
      LogInfo("EntityFactory: Library entity %d has no entity_id, skipping", i);
      continue;
    }

    if (debug_entity_creation()) {
      LogInfo("EntityFactory: Loaded prototype %s from file %s",
              meta_def->entity_id()->c_str(), entity_library_filename);
    }
    // This entity has an entity ID, so index the pointer to it.
    prototype_data_[meta_def->entity_id()->c_str()] = entities[i];
  }
  return true;
}

bool EntityFactory::WillBeKeptInMemory(const void* pointer) {
  if (pointer == nullptr)
    return true;  // nullptr will never become less null than it is now...
  for (auto iter = loaded_files_.begin(); iter != loaded_files_.end(); ++iter) {
    if (pointer >= iter->second->c_str() &&
        pointer < iter->second->c_str() + iter->second->length())
      // We are from this already-loaded file.
      return true;
  }
  // Nope, not from a loaded file!
  return false;
}

int EntityFactory::LoadEntitiesFromFile(const char* filename,
                                        corgi::EntityManager* entity_manager) {
  LogInfo("EntityFactory::LoadEntitiesFromFile: Reading %s", filename);
  if (loaded_files_.find(filename) == loaded_files_.end()) {
    std::string* data = new std::string();
    if (!fplbase::LoadFile(filename, data)) {
      LogInfo("EntityFactory::LoadEntitiesFromFile: Couldn't open file %s",
              filename);
      delete data;
      return kErrorLoadingEntities;
    }
    loaded_files_[filename].reset(data);
  }

  std::vector<corgi::EntityRef> entities_loaded;
  int total = LoadEntityListFromMemory(loaded_files_[filename]->c_str(),
                                       entity_manager, &entities_loaded);

  // Go through all the entities we just created and mark them with their source
  // file.
  MetaComponent* meta_component = entity_manager->GetComponent<MetaComponent>();
  for (size_t i = 0; i < entities_loaded.size(); i++) {
    corgi::EntityRef& entity = entities_loaded[i];
    // Track which source file this entity came from, so we can later output
    // it to the same file if we save it in the meta.
    if (meta_component != nullptr) {
      entity_manager->GetComponent<MetaComponent>()->AddWithSourceFile(
          entity, filename);
    }
  }
  LogInfo("EntityFactory::LoadEntitiesFromFile: Loaded %d entities from %s",
          total, filename);
  return total;
}

int EntityFactory::LoadEntityListFromMemory(
    const void* entity_list, corgi::EntityManager* entity_manager,
    std::vector<corgi::EntityRef>* entities_loaded) {
  // First, if there are currently no entities loaded, it's safe to clear out
  // stale entity files.
  if (entity_manager->begin() == entity_manager->end()) {
    stale_files_.clear();
  }
  std::vector<const void*> entities;
  if (!ReadEntityList(entity_list, &entities)) {
    LogInfo("EntityFactory: Couldn't read entity list");
    return kErrorLoadingEntities;
  }

  if (entities_loaded != nullptr) entities_loaded->reserve(entities.size());
  int total = 0;
  for (size_t i = 0; i < entities.size(); i++) {
    total++;
    corgi::EntityRef entity = entity_manager->CreateEntityFromData(entities[i]);
    if (entity && entities_loaded != nullptr) {
      entities_loaded->push_back(entity);
    }
  }
  return total;
}

// Override a cached file with data from memory.
void EntityFactory::OverrideCachedFile(const char* filename,
                                       std::unique_ptr<std::string> new_data) {
  if (loaded_files_.find(filename) != loaded_files_.end()) {
    stale_files_.push_back(std::move(loaded_files_[filename]));
  }
  loaded_files_[filename] = std::move(new_data);
}

void EntityFactory::LoadEntityData(const void* def,
                                   corgi::EntityManager* entity_manager,
                                   corgi::EntityRef& entity,
                                   bool is_prototype) {
  unsigned int meta_id = MetaComponent::GetComponentId();
  MetaComponent* meta_component = entity_manager->GetComponent<MetaComponent>();

  std::vector<const void*> components;
  if (!ReadEntityDefinition(def, &components)) {
    LogError("EntityFactory::LoadEntityData: Couldn't load entity data.");
    return;
  }

  const MetaDef* meta_def = static_cast<const MetaDef*>(components[meta_id]);
  if (meta_def != nullptr && meta_def->prototype() != nullptr &&
      *meta_def->prototype()->c_str() != '\0') {
    const char* prototype_name = meta_def->prototype()->c_str();
    if (prototype_data_.find(prototype_name) != prototype_data_.end()) {
      if (debug_entity_creation()) {
        LogInfo("EntityFactory::LoadEntityData: Loading prototype: %s",
                prototype_name);
      }
      LoadEntityData(prototype_data_[prototype_name], entity_manager, entity,
                     true);
    } else {
      LogError("EntityFactory::LoadEntityData: Invalid prototype: '%s'",
               prototype_name);
    }
  }

  std::set<corgi::ComponentId> overridden_components;

  for (size_t i = 0; i < components.size(); i++) {
    const void* component_data = components[i];
    if (component_data != nullptr) {
      if (debug_entity_creation()) {
        LogInfo("...reading %s from %s",
                ComponentIdToTableName(static_cast<corgi::ComponentId>(i)),
                is_prototype ? "prototype" : "entity");
      }
      overridden_components.insert(static_cast<corgi::ComponentId>(i));
      if (is_prototype && i == meta_id) {
        // MetaDef from prototypes gets loaded special.
        meta_component->AddFromPrototypeData(
            entity, static_cast<const MetaDef*>(component_data));
      } else {
        corgi::ComponentInterface* component =
            entity_manager->GetComponent(static_cast<corgi::ComponentId>(i));
        assert(component != nullptr);
        component->AddFromRawData(entity, component_data);
      }
    }
  }

  // Final clean-up on the top-level entity load: keep track of which parts
  // came from the initial entity and which came from prototypes.
  if (!is_prototype) {
    MetaData* meta_data =
        entity_manager->GetComponent<MetaComponent>()->AddEntity(entity);
    for (int component_id = 0; component_id <= max_component_id();
         component_id++) {
      // If we don't already have a MetaComponent, we should get one added.
      corgi::ComponentInterface* component_ptr =
          entity_manager->GetComponent(component_id);
      if (component_ptr != nullptr && component_ptr->HasDataForEntity(entity) &&
          overridden_components.find(component_id) ==
              overridden_components.end()) {
        meta_data->components_from_prototype.insert(component_id);
      }
    }
  }
}

// Factory method for the entity manager, for converting data (in our case.
// flatbuffer definitions) into entities and sticking them into the system.
corgi::EntityRef EntityFactory::CreateEntityFromData(
    const void* data, corgi::EntityManager* entity_manager) {
  assert(data != nullptr);
  if (debug_entity_creation()) {
    LogInfo("EntityFactory::CreateEntityFromData: Creating entity...");
  }
  corgi::EntityRef entity = entity_manager->AllocateNewEntity();
  LoadEntityData(data, entity_manager, entity, false);
  return entity;
}

corgi::EntityRef EntityFactory::CreateEntityFromPrototype(
    const char* prototype_name, corgi::EntityManager* entity_manager) {
  if (prototype_requests_.find(prototype_name) == prototype_requests_.end()) {
    std::vector<uint8_t> prototype_request;
    if (CreatePrototypeRequest(prototype_name, &prototype_request)) {
      if (debug_entity_creation()) {
        LogInfo("EntityFactory: Created prototype request for '%s'",
                prototype_name);
      }
      prototype_requests_[prototype_name] = prototype_request;
    } else {
      LogError("EntityFactory::CreatePrototypeRequest(%s) failed",
               prototype_name);
      return corgi::EntityRef();
    }
  }
  std::vector<corgi::EntityRef> entities_loaded;
  LoadEntityListFromMemory(
      static_cast<const void*>(prototype_requests_[prototype_name].data()),
      entity_manager, &entities_loaded);
  if (entities_loaded.size() > 0)
    return entities_loaded[0];
  else
    return corgi::EntityRef();
}

bool EntityFactory::SerializeEntity(
    corgi::EntityRef& entity, corgi::EntityManager* entity_manager,
    std::vector<uint8_t>* entity_serialized_output) {
  auto meta_component = entity_manager->GetComponent<MetaComponent>();

  std::vector<corgi::ComponentInterface::RawDataUniquePtr> exported_data;
  std::vector<const void*> exported_pointers;

  exported_pointers.resize(max_component_id() + 1, nullptr);
  for (corgi::ComponentId component_id = 0; component_id <= max_component_id();
       component_id++) {
    const MetaData* meta_data = meta_component->GetComponentData(entity);
    if (meta_data->components_from_prototype.find(component_id) ==
        meta_data->components_from_prototype.end()) {
      auto component = entity_manager->GetComponent(component_id);
      if (component != nullptr) {
        exported_data.push_back(component->ExportRawData(entity));
        exported_pointers[component_id] = exported_data.back().get();
      }
    }
  }

  return CreateEntityDefinition(exported_pointers, entity_serialized_output);
}

bool EntityFactory::SerializeEntityList(
    const std::vector<std::vector<uint8_t>>& entity_definitions,
    std::vector<uint8_t>* entity_list_serialized) {
  std::vector<const void*> entity_definition_pointers;
  entity_definition_pointers.reserve(entity_definitions.size());
  for (size_t i = 0; i < entity_definitions.size(); i++) {
    entity_definition_pointers.push_back(entity_definitions[i].data());
  }
  return CreateEntityList(entity_definition_pointers, entity_list_serialized);
}

void EntityFactory::SetComponentType(corgi::ComponentId component_id,
                                     unsigned int data_type,
                                     const char* table_name) {
  if (data_type_to_component_id_.size() <= data_type)
    data_type_to_component_id_.resize(data_type + 1, corgi::kInvalidComponent);
  data_type_to_component_id_[data_type] = component_id;

  if (component_id_to_data_type_.size() <= component_id)
    // Note: the int-construtor is required here to avoid taking the address
    //       of kDataTypeNone. If we took the address of kDataTypeNone, then
    //       Clang would required that we explicitly declare it in the .cpp
    //       file. However, Visual Studio complains if we explicitly declare it
    //       in the .cpp file, because we've assigned it a value (=0) in the
    //       header file.
    component_id_to_data_type_.resize(component_id + 1, int(kDataTypeNone));
  component_id_to_data_type_[component_id] = data_type;

  if (component_id_to_table_name_.size() <= component_id)
    component_id_to_table_name_.resize(component_id + 1, "");
  component_id_to_table_name_[component_id] = table_name;

  if (component_id > max_component_id_) max_component_id_ = component_id;

  LogInfo("EntityFactory: ComponentID %d = DataType %d = %s", component_id,
          data_type, table_name);
}

void EntityFactory::SetFlatbufferSchema(const char* binary_schema_filename) {
  if (!fplbase::LoadFile(binary_schema_filename,
                         &flatbuffer_binary_schema_data_)) {
    LogInfo(
        "EntityFactory::SetFlatbufferSchema: Can't load binary schema file %s",
        binary_schema_filename);
  }
}

}  // namespace component_library
}  // namespace corgi
