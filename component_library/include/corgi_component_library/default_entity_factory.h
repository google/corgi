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

#ifndef DEFAULT_ENTITY_FACTORY_H_
#define DEFAULT_ENTITY_FACTORY_H_

#include <set>
#include <string>
#include <vector>

#include "corgi/entity_manager.h"
#include "corgi_component_library/entity_factory.h"
#include "corgi_component_library/meta.h"
#include "flatbuffers/flatbuffers.h"
#include "fplbase/utilities.h"

namespace corgi {
namespace component_library {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @class DefaultEntityFactory
///
/// @brief Implementations of DefaultEntityFactory can be found in
/// `default_entity_factory.inc`.
class DefaultEntityFactory : public component_library::EntityFactory {
 public:
  /// @brief Destructor for DefaultEntityFactory.
  virtual ~DefaultEntityFactory() {}

  /// @brief Handles reading an Entity list and extracting the individual
  /// Entity data definitions.
  ///
  /// @param[in] entity_list A const void pointer to the start of the list
  /// of Entities.
  /// @param[out] entity_defs A vector that captures the output of the
  /// extracted Entity data definitions.
  ///
  /// @return returns `true` if the list was parsed successfully. Otherwise it
  /// returns `false`.
  virtual bool ReadEntityList(const void* entity_list,
                              std::vector<const void*>* entity_defs);

  /// @brief Handles reading an Entity definition and extracting the individual
  /// Component data definitions.
  ///
  /// @param[in] entity_definition A const void pointer to the Entity definition
  /// whose Component data definitions should be extracted.
  /// @param[out] component_defs A vector that captures the output of
  /// the extracted Component data definitions.
  ///
  /// @return Returns `true` if the `entity_definition` was parsed successfully.
  /// Otherwise it returns `false`.
  virtual bool ReadEntityDefinition(const void* entity_definition,
                                    std::vector<const void*>* component_defs);

  /// @brief Creates an Entity list that contains a single Entity definition,
  /// which contains a single Component definition (a `MetaDef` with `prototype`
  /// set to the requested prototype name).
  ///
  /// @param[in] prototype_name A C-string name of the prototype, which is used
  /// to set the `prototype` field in the MetaComponent.
  /// @param[out] request A vector of bytes to capture the output of the
  /// prototype request FlatBuffer data.
  ///
  /// @return Returns `true` if the request was created successfully. Otherwise
  /// it returns `false`.
  virtual bool CreatePrototypeRequest(const char* prototype_name,
                                      std::vector<uint8_t>* request);

  /// @brief Handles building a single Entity definition flatbuffer from a list
  /// of an Entity's Component definitions.
  ///
  /// @param[in] component_data A const reference to a std::vector that contains
  /// the list of the Entity's Component definitions, which can be indexed by
  /// Component ID.
  /// @param[out] entity_definition A vector of bytes to capture the output of
  /// the Entity definition FlatBuffer data.
  ///
  /// @return Returns `true` if the Entity definition was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool CreateEntityDefinition(
      const std::vector<const void*>& component_data,
      std::vector<uint8_t>* entity_definition);

  /// @brief Handles building an Entity list flatbuffer from a collection of
  /// individual Entity flatbuffers.
  ///
  /// @param[in] entity_defs A const reference to a std::vector that contains
  /// all the Entity flatbuffers.
  /// @param[out] entity_list A vector of bytes to capture the output of the
  /// Entity list FlatBuffer data.
  ///
  /// @return Returns `true` if the Entity list was successfully created.
  /// Otherwise, it returns `false`.
  virtual bool CreateEntityList(const std::vector<const void*>& entity_defs,
                                std::vector<uint8_t>* entity_list);
};
/// @}

}  // namespace component_library
}  // namespace corgi

#endif  // DEFAULT_ENTITY_FACTORY_H_
