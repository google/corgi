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

/// This is a default implementation of the virtual functions in EntityFactory.
/// If you use the default names for the component FlatBuffers schemas in your
/// project, you can use this as your entity factory by creating a source file,
/// including your generated FlatBuffers headers, and then including this file.
///
/// It provides implementations of the following virtual methods in
/// EntityFactory:
/// * LoadEntitiesFromFile
/// * CreateEntityFromPrototype
/// * ReadEntityList
/// * ReadEntityDefinition
/// * CreatePrototypeRequest
/// * CreateEntityList
/// * LoadEntityData
///
/// This default entity factory assumes the following 4 generated FlatBuffers
/// types defined for serialization of component data:
///
/// * ComponentDataUnion
///     The union of the *Defs you are using in your game (e.g. TransformDef,
///     RenderMeshDef, MyCustomComponentDef)
/// * ComponentDefInstance
///     A table containing one instance of the above union.
/// * EntityDef
///     A list of ComponentDefInstances, defining a single entity's components.
/// * EntityListDef
///     A list of EntityDefs, defining multiple entities.
///
/// These types should be defined in your FlatBuffers schema as follows:
///
/// union ComponentDataUnion {
///   MetaDef,
///   CommonServicesDef,
///   TransformDef,
///   RenderMeshDef,
///   [... list all component definitions here ...]
///   // Note: The only component definition from the component library required
///   // by the default entity factory is MetaDef. The rest are optional and can
///   // be replaced with your own versions if you wish.
/// }
/// table ComponentDefInstance {
///   data:ComponentDataUnion;
/// }
/// table EntityDef {
///   component_list:[ComponentDefInstance];
/// }
/// table EntityListDef {
///   entity_list:[EntityDef];
/// }
/// root_type EntityListDef;
///
/// IMPORTANT: In the CPP file that includes this, you must remember to #define
/// FACTORY_TYPE_NAMESPACE to the namespace your generated FlatBuffers data
/// types are in before including this file, and to include the generated header
/// file before including this file.
///
/// For example, your project's default_entity_factory.cpp file should look
/// something like this:
///
/// /* default_entity_factory.cpp */
/// #include "mygame_components_generated.h"
///
/// #define FACTORY_TYPE_NAMESPACE mygame
/// #include "corgi_component_library/default_entity_factory.inc"

#ifndef FACTORY_TYPE_NAMESPACE
#error You must #define FACTORY_TYPE_NAMESPACE to the namespace your generated \
       FlatBuffers types are in before including this file.
#endif
// The namespace needs to be prepended to the table when we look it up, so
// define the full table name.
#define FPL_STR_EXPAND(tok) #tok
#define FPL_STR(tok) FPL_STR_EXPAND(tok)
#define ENTITY_DEF FPL_STR(FACTORY_TYPE_NAMESPACE) ".EntityDef"

#include "corgi_component_library/default_entity_factory.h"
#include "flatbuffers/reflection.h"

namespace corgi {
namespace component_library {

using FACTORY_TYPE_NAMESPACE::ComponentDataUnion;
using FACTORY_TYPE_NAMESPACE::ComponentDefInstance;
using FACTORY_TYPE_NAMESPACE::EntityDef;
using FACTORY_TYPE_NAMESPACE::EntityListDef;

bool DefaultEntityFactory::ReadEntityList(
    const void* entity_list, std::vector<const void*>* entity_defs) {
  const EntityListDef* list = flatbuffers::GetRoot<EntityListDef>(entity_list);
  entity_defs->clear();
  entity_defs->reserve(list->entity_list()->size());
  for (size_t i = 0; i < list->entity_list()->size(); i++) {
    flatbuffers::uoffset_t index = static_cast<flatbuffers::uoffset_t>(i);
    entity_defs->push_back(list->entity_list()->Get(index));
  }
  return true;
}

bool DefaultEntityFactory::ReadEntityDefinition(
    const void* entity_definition, std::vector<const void*>* component_defs) {
  const EntityDef* def = static_cast<const EntityDef*>(entity_definition);
  component_defs->clear();
  component_defs->resize(max_component_id() + 1, nullptr);
  for (size_t i = 0; i < def->component_list()->size(); i++) {
    flatbuffers::uoffset_t index = static_cast<flatbuffers::uoffset_t>(i);
    const ComponentDefInstance* component_def =
        def->component_list()->Get(index);
    corgi::ComponentId component_id =
        DataTypeToComponentId(component_def->data_type());
    if (component_id == corgi::kInvalidComponent) {
      fplbase::LogError(
          "ReadEntityDefinition: Error, unknown component data type %d\n",
          component_def->data_type());
      return false;
    }
    (*component_defs)[component_id] = component_def->data();
  }
  return true;
}

bool DefaultEntityFactory::CreatePrototypeRequest(
    const char* prototype_name, std::vector<uint8_t>* request) {
  // Create the prototype request for the first time. It will be cached
  // for future use.
  flatbuffers::FlatBufferBuilder fbb;
  auto prototype = fbb.CreateString(prototype_name);
  corgi::MetaDefBuilder builder(fbb);
  builder.add_prototype(prototype);
  auto component = CreateComponentDefInstance(
      fbb, ComponentDataUnion(ComponentIdToDataType(
               ::corgi::component_library::MetaComponent::GetComponentId())),
      builder.Finish().Union());
  std::vector<flatbuffers::Offset<ComponentDefInstance>> component_vec;
  component_vec.push_back(component);
  auto entity_def = CreateEntityDef(fbb, fbb.CreateVector(component_vec));
  std::vector<flatbuffers::Offset<EntityDef>> entity_vec;
  entity_vec.push_back(entity_def);
  auto entity_list = CreateEntityListDef(fbb, fbb.CreateVector(entity_vec));
  fbb.Finish(entity_list);
  *request = std::vector<uint8_t>(fbb.GetBufferPointer(),
                                  fbb.GetBufferPointer() + fbb.GetSize());

  return true;
}

bool DefaultEntityFactory::CreateEntityDefinition(
    const std::vector<const void*>& component_data,
    std::vector<uint8_t>* entity_definition) {
  // Take an individual component data type such as TransformData, and wrap it
  // inside the union in ComponentDefInstance.

  // For each component, we only have a raw pointer to the flatbuffer data as
  // exported by that component. We use our knowledge of the data types for each
  // component ID to get the table type to copy, and to set the union data type
  // to the correct value.
  if (flatbuffer_binary_schema_data() == "") {
    fplbase::LogError(
        "CreateEntityDefinition: No schema loaded, can't CopyTable");
    return false;
  }
  auto schema = reflection::GetSchema(flatbuffer_binary_schema_data().c_str());
  if (schema == nullptr) {
    fplbase::LogError(
        "CreateEntityDefinition: GetSchema() failed, is it a binary schema?");
    return false;
  }

  flatbuffers::FlatBufferBuilder fbb;
  std::vector<flatbuffers::Offset<ComponentDefInstance>> component_list;
  for (corgi::ComponentId i = 0; i <= max_component_id(); i++) {
    // For each non-null component ID, create a ComponentDefInstance and copy
    // the data for that component in, using reflection.
    if (component_data[i] != nullptr) {
      const uint8_t* raw_data = static_cast<const uint8_t*>(component_data[i]);
      ComponentDataUnion data_type =
          static_cast<ComponentDataUnion>(ComponentIdToDataType(i));
      const char* table_name = ComponentIdToTableName(i);
      auto table_def = schema->objects()->LookupByKey(table_name);
      if (table_def != nullptr) {
        component_list.push_back(CreateComponentDefInstance(
            fbb, data_type,
            flatbuffers::CopyTable(fbb, *schema, *table_def,
                                   *flatbuffers::GetAnyRoot(raw_data))
                .o));
      } else {
        fplbase::LogError(
            "CreateEntityDefinition: Unknown table for component %d with data "
            "type %d: '%s'",
            i, data_type, table_name);
      }
    }
  }
  auto entity_def = CreateEntityDef(fbb, fbb.CreateVector(component_list));
  fbb.Finish(entity_def);
  *entity_definition = std::vector<uint8_t>(
      fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
  return true;
}

bool DefaultEntityFactory::CreateEntityList(
    const std::vector<const void*>& entity_defs,
    std::vector<uint8_t>* entity_list_output) {
  // Given a collection of EntityDef flatbuffers, put them all into a single
  // EntityListDef. Similar to CreateEntityDefinition, this function uses
  // flatbuffers deep copy to put the entity definitions in the new flatbuffer.
  if (flatbuffer_binary_schema_data() == "") {
    fplbase::LogError("CreateEntityList: No schema loaded, can't CopyTable");
    return false;
  }
  auto schema = reflection::GetSchema(flatbuffer_binary_schema_data().c_str());
  if (schema == nullptr) {
    fplbase::LogError(
        "CreateEntityList: GetSchema() failed, is it a binary schema?");
    return false;
  }
  auto table_def = schema->objects()->LookupByKey(ENTITY_DEF);
  if (table_def == nullptr) {
    fplbase::LogError("CreateEntityList: Can't look up EntityDef");
    return false;
  }
  flatbuffers::FlatBufferBuilder fbb;
  std::vector<flatbuffers::Offset<EntityDef>> entity_list;
  for (auto entity_def = entity_defs.begin(); entity_def != entity_defs.end();
       ++entity_def) {
    const uint8_t* raw_data = static_cast<const uint8_t*>(*entity_def);
    entity_list.push_back(
        flatbuffers::CopyTable(fbb, *schema, *table_def,
                               *flatbuffers::GetAnyRoot(raw_data))
            .o);
  }

  auto entity_list_def =
      CreateEntityListDef(fbb, fbb.CreateVector(entity_list));
  fbb.Finish(entity_list_def);
  *entity_list_output = std::vector<uint8_t>(
      fbb.GetBufferPointer(), fbb.GetBufferPointer() + fbb.GetSize());
  return true;
}

}  // namespace component_library
}  // namespace corgi
