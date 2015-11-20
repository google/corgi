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

#include "entity_component_system.h"

// `CORGI_DEFINE_COMPONENT` should be called in a `.cpp` file
// that defines the Component class and corresponding Component data stucture.
// It handles defining the storage location for the Component for a given
// Component type and data type.
//
// NOTE: Calls to this function should happen outside of any namespaces!
CORGI_DEFINE_COMPONENT(corgi::sample::CounterComponent,
                       corgi::sample::CounterComponentData)

CORGI_DEFINE_COMPONENT(corgi::sample::ScreamingComponent,
                       corgi::sample::ScreamingComponentData)

namespace corgi {
namespace sample {

void CounterComponent::UpdateAllEntities(WorldTime /*delta_time*/) {
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    CounterComponentData* entity_data =
        Data<CounterComponentData>(iter->entity);
    entity_data->counter++;
  }
}

void ScreamingComponent::UpdateAllEntities(WorldTime delta_time) {
  if (delta_time > 10) {
    for (auto iter = component_data_.begin(); iter != component_data_.end();
         ++iter) {
      ScreamingComponentData* entity_data =
          Data<ScreamingComponentData>(iter->entity);
      printf("%s\n", entity_data->battle_cry.c_str());
    }
  }
}

}  // namespace sample
}  // namespace corgi

int main() {
  using corgi::EntityManager;
  using corgi::EntityRef;
  using corgi::sample::CounterComponent;
  using corgi::sample::CounterComponentData;
  using corgi::sample::ScreamingComponent;
  using corgi::sample::ScreamingComponentData;

  // Create the EntityManager and all of the Components.
  EntityManager entity_manager;
  CounterComponent counter_component;
  ScreamingComponent screaming_component;

  // Register the Components with the Entity Manager.
  // NOTE: The order that you register Components with the EntityManager is the
  //       order that they will be executed in.
  entity_manager.RegisterComponent<CounterComponent>(&counter_component);
  entity_manager.RegisterComponent<ScreamingComponent>(&screaming_component);

  // Create all of the Entities.
  EntityRef new_entity = entity_manager.AllocateNewEntity();

  // Register an Entity with all the Components it should be associated with.
  entity_manager.AddEntityToComponent<CounterComponent>(new_entity);
  entity_manager.AddEntityToComponent<ScreamingComponent>(new_entity);

  // Simulate a game-loop that executes 10 times with a random delta_time.
  std::srand(static_cast<unsigned>(std::time(0)));

  for (int x = 0; x < 10; x++) {
    int mock_delta_time = std::rand() % 20 + 1;  // Random delta_time from 1-20.

    // You typically call EntityManager's `UpdateComponents` once per frame.
    //
    // In this sample, it will first execute CounterComponent's
    // `UpdateAllEntities` to increment the `counter` for each Entity registered
    // with CounterComponent.
    //
    // Next, it will execute ScreamingComponent's `UpdateAllEntities` to check
    // if the `mock_delta_time` is greater than 10. If so, it will print out
    // each Entity's `battle_cry` string for each Entity that is registered with
    // ScreamingComponent.
    entity_manager.UpdateComponents(mock_delta_time);
  }

  // Output the Entity's `counter` data to show that it incremented correctly.
  CounterComponentData* entity_data =
      entity_manager.GetComponentData<CounterComponentData>(new_entity);
  printf("The current counter is = %d.\n", entity_data->counter);
}
