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

// Data for scene object components.
struct CommonServicesData {};

// This is a somewhat unique component - No entities will directly subscribe
// to it, and it has no per-entity data.  However, it provides an easy place
// for other components to access game services and managers.  (Since components
// don't have direct access to the gamestate, but they do have access to other
// components.)
class CommonServicesComponent : public corgi::Component<CommonServicesData> {
 public:
  CommonServicesComponent() : export_force_defaults_(false) {}

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

  fplbase::AssetManager* asset_manager() { return asset_manager_; }
  breadboard::GraphFactory* graph_factory() { return graph_factory_; }
  fplbase::InputSystem* input_system() { return input_system_; }
  EntityFactory* entity_factory() { return entity_factory_; }
  fplbase::Renderer* renderer() { return renderer_; }

  // This component should never be added to an entity.  It is only provided
  // as an interface for other components to access common resources.
  void AddFromRawData(corgi::EntityRef& /*entity*/, const void* /*raw_data*/) {
    assert(false);
  }

  // When components are exporting their data, should they use ForceDefaults?
  bool export_force_defaults() const { return export_force_defaults_; }
  void set_export_force_defaults(bool b) { export_force_defaults_ = b; }

 private:
  fplbase::AssetManager* asset_manager_;
  EntityFactory* entity_factory_;
  breadboard::GraphFactory* graph_factory_;
  fplbase::InputSystem* input_system_;
  fplbase::Renderer* renderer_;
  bool export_force_defaults_;
};

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::CommonServicesComponent,
                         corgi::component_library::CommonServicesData)

#endif  // CORGI_COMPONENT_LIBRARY_COMMON_SERVICES_H_
