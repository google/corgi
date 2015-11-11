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

#ifndef CORGI_COMPONENT_LIBRARY_COMMON_SERVICES_H_
#define CORGI_COMPONENT_LIBRARY_COMMON_SERVICES_H_

#include "breadboard/graph_factory.h"
#include "corgi/component.h"
#include "corgi_component_library/entity_factory.h"
#include "fplbase/asset_manager.h"
#include "fplbase/input.h"
#include "fplbase/renderer.h"
#include "fplbase/utilities.h"

namespace corgi {
namespace component_library {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @struct CommonServicesData
///
/// @brief Holds the data that Components need (e.g. the input system,
/// renderer, etc.).
struct CommonServicesData {};

/// @class CommonServicesComponent
///
/// @brief This is a unique Component, as no Entities will register with it and
/// it contains no per-entity data. Its use is to provide a central location for
/// other Components to easily access game services and managers (since
/// Components themselves do not have direct access to the game state, but do
/// have access to other Components).
class CommonServicesComponent : public corgi::Component<CommonServicesData> {
 public:
  /// @brief The default constructor to create an empty CommonServicesComponent.
  CommonServicesComponent() : export_force_defaults_(false) {}

  /// @brief Destructor for CommonServicesComponent.
  virtual ~CommonServicesComponent() {}

  /// @brief Initializes the CommonServicesComponent with pointers to the
  /// various game services and managers.
  ///
  /// @param[in] asset_manager A pointer to the game's AssetManager.
  /// @param[in] entity_factory A pointer to the game's EntityFactory.
  /// @param[in] graph_factory A pointer to the game's breadboard::GraphFactory.
  /// @param[in] input_system A pointer to the game's input_system.
  /// @param[in] renderer A pointer to the game's Renderer.
  void Initialize(fplbase::AssetManager* asset_manager,
                  EntityFactory* entity_factory,
                  breadboard::GraphFactory* graph_factory,
                  fplbase::InputSystem* input_system,
                  fplbase::Renderer* renderer) {
    asset_manager_ = asset_manager;
    entity_factory_ = entity_factory;
    graph_factory_ = graph_factory;
    input_system_ = input_system;
    renderer_ = renderer;
  }

  /// @return Returns a pointer to the AssetManager.
  fplbase::AssetManager* asset_manager() { return asset_manager_; }

  /// @return Returns a pointer to the breadboard::GraphFactory.
  breadboard::GraphFactory* graph_factory() { return graph_factory_; }

  /// @return Returns a pointer to the InputSystem.
  fplbase::InputSystem* input_system() { return input_system_; }

  /// @return Returns a pointer to the EntityFactory.
  EntityFactory* entity_factory() { return entity_factory_; }

  /// @return Returns a pointer to the Renderer.
  fplbase::Renderer* renderer() { return renderer_; }

  /// @brief This component should never be added to an Entity. It is only
  /// provided as an interface for other components to access common game
  /// resources.
  ///
  /// @warning Asserts when called.
  void AddFromRawData(corgi::EntityRef& /*entity*/, const void* /*raw_data*/) {
    assert(false);
  }

  /// @brief This should be called when Components are exporting their data to a
  /// FlatBuffer. It indicates if the FlatBuffer should include default values
  /// in the serialized data.
  ///
  /// @return Returns `true` if the FlatBuffer serialized data should include
  /// default values. Otherwise returns `false`, indicating that default values
  /// should be excluded from the FlatBuffer serialized data.
  bool export_force_defaults() const { return export_force_defaults_; }

  /// @brief Set a flag to determine if Components, when exporting their data
  /// to FlatBuffers, should include default values in the serialized data.
  ///
  /// @param[in] b A `bool` indicating if the FlatBuffer serialized data should
  /// include default values.
  void set_export_force_defaults(bool b) { export_force_defaults_ = b; }

 private:
  fplbase::AssetManager* asset_manager_;
  EntityFactory* entity_factory_;
  breadboard::GraphFactory* graph_factory_;
  fplbase::InputSystem* input_system_;
  fplbase::Renderer* renderer_;
  bool export_force_defaults_;
};
/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::CommonServicesComponent,
                         corgi::component_library::CommonServicesData)

#endif  // CORGI_COMPONENT_LIBRARY_COMMON_SERVICES_H_
