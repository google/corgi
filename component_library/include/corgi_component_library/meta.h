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

#ifndef CORGI_COMPONENT_LIBRARY_META_H_
#define CORGI_COMPONENT_LIBRARY_META_H_

#include <set>
#include <string>
#include <unordered_map>
#include "corgi/component.h"
#include "library_components_generated.h"

namespace corgi {
namespace component_library {

struct MetaData {
  MetaData() {}
  std::string entity_id;
  std::string prototype;
  std::string source_file;
  std::string comment;
  // Keep track of which of this entity's components came from the prototype.
  std::set<corgi::ComponentId> components_from_prototype;
};

class MetaComponent : public corgi::Component<MetaData> {
 public:
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);
  void AddFromPrototypeData(corgi::EntityRef& entity,
                            const corgi::MetaDef* meta_def);
  void AddWithSourceFile(corgi::EntityRef& entity,
                         const std::string& source_file);
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  virtual void InitEntity(corgi::EntityRef& entity);
  virtual void CleanupEntity(corgi::EntityRef& entity);
  virtual void UpdateAllEntities(corgi::WorldTime /*delta_time*/) {}

  const std::string& GetEntityID(const corgi::EntityRef& entity);

  // Non-const because if we find an invalid entity, it gets silently removed.
  corgi::EntityRef GetEntityFromDictionary(const std::string& key);

 private:
  void AddEntityToDictionary(const std::string& key,
                             const corgi::EntityRef& entity);
  void RemoveEntityFromDictionary(const std::string& key);
  void GenerateRandomEntityID(std::string* output);

  std::unordered_map<std::string, corgi::EntityRef> entity_dictionary_;
  std::string empty_string;
};

}  // namespace component_library
}  // namespace corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::MetaComponent,
                         corgi::component_library::MetaData)

#endif  // CORGI_COMPONENT_LIBRARY_META_H_
