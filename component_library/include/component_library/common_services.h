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

#ifndef COMPONENT_LIBRARY_COMMON_SERVICES_H_
#define COMPONENT_LIBRARY_COMMON_SERVICES_H_

#include "component_library/entity_factory.h"
#include "entity/component.h"
#include "event/event_manager.h"
#include "fplbase/asset_manager.h"
#include "fplbase/input.h"
#include "fplbase/renderer.h"
#include "fplbase/utilities.h"

namespace fpl {
namespace component_library {

// Data for scene object components.
struct CommonServicesData {};

// This is a somewhat unique component - No entities will directly subscribe
// to it, and it has no per-entity data.  However, it provides an easy place
// for other components to access game services and managers.  (Since components
// don't have direct access to the gamestate, but they do have access to other
// components.)
class CommonServicesComponent : public entity::Component<CommonServicesData> {
 public:
  CommonServicesComponent() {}

  void Initialize(AssetManager* asset_manager, EntityFactory* entity_factory,
                  event::EventManager* event_manager, InputSystem* input_system,
                  Renderer* renderer) {
    asset_manager_ = asset_manager;
    entity_factory_ = entity_factory;
    event_manager_ = event_manager;
    input_system_ = input_system;
    renderer_ = renderer;
  }

  AssetManager* asset_manager() { return asset_manager_; }
  event::EventManager* event_manager() { return event_manager_; }
  InputSystem* input_system() { return input_system_; }
  EntityFactory* entity_factory() { return entity_factory_; }
  Renderer* renderer() { return renderer_; }

  // This component should never be added to an entity.  It is only provided
  // as an interface for other components to access common resources.
  void AddFromRawData(entity::EntityRef& /*entity*/, const void* /*raw_data*/) {
    assert(false);
  }

 private:
  AssetManager* asset_manager_;
  EntityFactory* entity_factory_;
  event::EventManager* event_manager_;
  InputSystem* input_system_;
  Renderer* renderer_;
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::CommonServicesComponent,
                              fpl::component_library::CommonServicesData)

#endif  // COMPONENT_LIBRARY_SERVICES_H_
