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

#include "corgi_component_library/meta.h"
#include <string.h>
#include "corgi_component_library/common_services.h"
#include "corgi_component_library/rendermesh.h"
#include "fplbase/utilities.h"
#include "mathfu/utilities.h"

CORGI_DEFINE_COMPONENT(corgi::component_library::MetaComponent,
                       corgi::component_library::MetaData)

namespace corgi {
namespace component_library {

void MetaComponent::AddFromRawData(corgi::EntityRef& entity,
                                   const void* raw_data) {
  MetaData* meta_data = AddEntity(entity);
  if (raw_data != nullptr) {
    auto meta_def = static_cast<const corgi::MetaDef*>(raw_data);
    if (meta_def->entity_id() != nullptr) {
      if (meta_data->entity_id != "") {
        RemoveEntityFromDictionary(meta_data->entity_id);
      }
      meta_data->entity_id = meta_def->entity_id()->c_str();
      AddEntityToDictionary(meta_data->entity_id, entity);
    }
    if (meta_def->prototype() != nullptr) {
      meta_data->prototype = meta_def->prototype()->c_str();
    }
    if (meta_def->comment() != nullptr) {
      meta_data->comment = meta_def->comment()->c_str();
    }
  }
}
void MetaComponent::AddFromPrototypeData(corgi::EntityRef& entity,
                                         const corgi::MetaDef* meta_def) {
  MetaData* meta_data = AddEntity(entity);
  if (meta_def->comment() != nullptr) {
    meta_data->comment = meta_def->comment()->c_str();
  }
}

void MetaComponent::AddWithSourceFile(corgi::EntityRef& entity,
                                      const std::string& source_file) {
  MetaData* data = AddEntity(entity);
  data->source_file = source_file;
  auto ext = data->source_file.rfind('.');
  if (ext != std::string::npos) {
    data->source_file = data->source_file.substr(0, ext);
  }
}

corgi::ComponentInterface::RawDataUniquePtr MetaComponent::ExportRawData(
    const corgi::EntityRef& entity) const {
  const MetaData* data = GetComponentData(entity);
  if (data == nullptr) return nullptr;

  flatbuffers::FlatBufferBuilder fbb;
  bool defaults = entity_manager_->GetComponent<CommonServicesComponent>()
                      ->export_force_defaults();
  fbb.ForceDefaults(defaults);
  // Const-cast should be okay here because we're just giving
  // something an entity ID before exporting it (if it doesn't already
  // have one).
  auto entity_id =
      fbb.CreateString(const_cast<MetaComponent*>(this)->GetEntityID(entity));
  auto prototype = (defaults || data->prototype != "")
                       ? fbb.CreateString(data->prototype)
                       : 0;
  auto comment =
      (defaults || data->comment != "") ? fbb.CreateString(data->comment) : 0;

  MetaDefBuilder builder(fbb);
  builder.add_entity_id(entity_id);
  if (defaults || prototype.o != 0) {
    builder.add_prototype(prototype);
  }
  if (defaults || comment.o != 0) {
    builder.add_comment(comment);
  }
  fbb.Finish(builder.Finish());
  return fbb.ReleaseBufferPointer();
}

void MetaComponent::InitEntity(corgi::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data != nullptr && data->entity_id != "")
    AddEntityToDictionary(data->entity_id, entity);
}

void MetaComponent::CleanupEntity(corgi::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data != nullptr && data->entity_id != "" &&
      GetEntityFromDictionary(data->entity_id) == entity)
    RemoveEntityFromDictionary(data->entity_id);
}

const std::string& MetaComponent::GetEntityID(const corgi::EntityRef& entity) {
  MetaData* data = GetComponentData(entity);
  if (data == nullptr) {
    return empty_string;
  }
  if (data->entity_id == "") {
    // If this entity doesn't already have an ID, generate a new random one.
    GenerateRandomEntityID(&data->entity_id);
    AddEntityToDictionary(data->entity_id, entity);
  }
  return data->entity_id;
}

void MetaComponent::AddEntityToDictionary(const std::string& key,
                                          const corgi::EntityRef& entity) {
  // Check for duplicate entities so we can warn the user.
  auto i = entity_dictionary_.find(key);
  if (i != entity_dictionary_.end()) {
    // For log, in case data->entity_id points at key.
    const std::string original_key(key);
    MetaData* data = GetComponentData(entity);
    GenerateRandomEntityID(&data->entity_id);
    fplbase::LogError(
        "Duplicate entities with entity ID '%s', randomizing to '%s'. "
        "Check your entity data.",
        original_key.c_str(), data->entity_id.c_str());
    entity_dictionary_[data->entity_id] = entity;
  } else {
    entity_dictionary_[key] = entity;
  }
}
void MetaComponent::RemoveEntityFromDictionary(const std::string& key) {
  auto i = entity_dictionary_.find(key);
  if (i != entity_dictionary_.end()) {
    entity_dictionary_.erase(i);
  }
}
corgi::EntityRef MetaComponent::GetEntityFromDictionary(
    const std::string& key) {
  auto i = entity_dictionary_.find(key);
  if (i == entity_dictionary_.end()) {
    return corgi::EntityRef();
  }
  if (!i->second.IsValid()) {
    // The entity with this key is no longer valid, we'd better remove it.
    RemoveEntityFromDictionary(key);
    return corgi::EntityRef();
  }
  return i->second;
}

static const size_t kMaximumGeneratedEntityIDStringLength = 33;

void MetaComponent::GenerateRandomEntityID(std::string* output) {
  int num_digits = 16;
  const char digits[] = "0123456789abcdef";
  int digit_choices = static_cast<int>(strlen(digits));
  const char separator = '-';
  const int separator_every = 5;

  char name[kMaximumGeneratedEntityIDStringLength + 1];
  size_t i = 0;
  name[i++] = '$';
  for (; i < sizeof(name) - 1; i++) {
    if (i % separator_every != 0) {
      int random_digit = mathfu::RandomInRange(0, digit_choices - 1);
      name[i] = digits[random_digit];
      num_digits--;
    } else {
      name[i] = separator;
    }
    if (num_digits == 0) break;
  }
  name[i + 1] = '\0';
  // Do we actually need to check if the entity ID we generated is
  // already in the entity dictionary? It's pretty low odds...
  *output = std::string(name);
}

}  // namespace component_library
}  // namespace fpl
