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

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @struct MetaData
///
/// @brief Holds specific metadata for an Entity.
struct MetaData {
  MetaData() {}
  /// @var entity_id
  ///
  /// @brief A std::string to uniquely identify this Entity.
  std::string entity_id;

  /// @var prototype
  ///
  /// @brief If this Entity is based on a prototype, the prototype's Entity ID
  /// is stored here here.
  std::string prototype;

  /// @var source_file
  ///
  /// @brief If the Entity came from a source file, the std::string name of the
  /// file is stored here.
  std::string source_file;

  /// @var comment
  ///
  /// @brief A human-readable comment to remember what this entity is for.
  std::string comment;

  /// @var components_from_prototype
  ///
  /// @brief Used to keep track of which of this Entity's Components
  /// came from the prototype.
  std::set<corgi::ComponentId> components_from_prototype;
};

/// @class MetaComponent
///
/// @brief A Component used to track the metadata about the Entities
/// themselves.
class MetaComponent : public corgi::Component<MetaData> {
 public:
  /// @brief The destructor for MetaComponent.
  virtual ~MetaComponent() {}

  /// @brief Deserialize a flat binary buffer to create and populate an Entity's
  /// MetaData from raw data.
  ///
  /// @brief entity An EntityRef reference that points to an Entity whose
  /// MetaData is being populated from raw data.
  /// @brief raw_data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);

  /// @brief Adds the comment from the prototype data to an Entity's MetaData.
  ///
  /// @param[in] entity The Entity whose MetaData should be updated.
  /// @param[in] meta_def A const pointer to the MetaDef whose data
  /// should be added to the Entity's MetaData.
  void AddFromPrototypeData(corgi::EntityRef& entity,
                            const corgi::MetaDef* meta_def);

  /// @brief Adds a source file name to an Entity's MetaData.
  ///
  /// @param[in] entity The Entity whose MetaData should be updated.
  /// @param[in] source_file A const referenec to the std::string that contains
  /// the name of the source file that this Entity came from.
  void AddWithSourceFile(corgi::EntityRef& entity,
                         const std::string& source_file);

  /// @brief Serializes a MetaComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef referene to an Entity whose corresponding
  /// MetaData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a
  /// flat binary buffer.
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  /// @brief Adds the given Entity to the dictionary tracked by this Component.
  ///
  /// @param[in] entity The EntityRef to the Entity to add to the dictionary.
  virtual void InitEntity(corgi::EntityRef& entity);

  /// @brief Removes the given Entity from the dictionary tracked by this
  /// Component.
  ///
  /// @param[in] entity The EntityRef to the Entity that should be removed
  /// from the dictionary.
  virtual void CleanupEntity(corgi::EntityRef& entity);

  /// @brief Does nothing. This is only implemented as part of the
  /// ComponentInterface.
  virtual void UpdateAllEntities(corgi::WorldTime /*delta_time*/) {}

  /// @brief Get the ID for this Entity.
  ///
  /// @note If this Entity does not already have an ID, a random one
  /// will be generated.
  ///
  /// @param[in] entity A const EntityRef reference to the Entity whose
  /// ID should be returned.
  ///
  /// @return Returns a std::string containing the Entity ID.
  const std::string& GetEntityID(const corgi::EntityRef& entity);

  /// @brief Get an Entity from the dictionary at a given key.
  ///
  /// @note If an invalid Entity is found, it gets silently removed.
  ///
  /// @param[in] key A const reference to the std::string Entity ID that is
  /// used as the index into the dictionary to lookup the Entity.
  ///
  /// @return Returns an EntityRef to the Entity at the given key. If
  /// the `key` or Entity were invalid, it returns an empty EntityRef.
  corgi::EntityRef GetEntityFromDictionary(const std::string& key);

 private:
  void AddEntityToDictionary(const std::string& key,
                             const corgi::EntityRef& entity);
  void RemoveEntityFromDictionary(const std::string& key);
  void GenerateRandomEntityID(std::string* output);

  std::unordered_map<std::string, corgi::EntityRef> entity_dictionary_;
  std::string empty_string;
};
/// @}

}  // namespace component_library
}  // namespace corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::MetaComponent,
                         corgi::component_library::MetaData)

#endif  // CORGI_COMPONENT_LIBRARY_META_H_
