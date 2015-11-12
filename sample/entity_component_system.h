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

#ifndef ENTITY_SAMPLES_ENTITY_COMPONENT_SYSTEM_H_
#define ENTITY_SAMPLES_ENTITY_COMPONENT_SYSTEM_H_

#include <stdio.h>  // printf()
#include <cstdlib>  // std::srand(), std::rand()
#include <ctime>    // std::time()
#include <string>   // std::string

#include "corgi/component.h"
#include "corgi/entity_manager.h"

namespace corgi {
namespace sample {

// This is the Component data structure for the CounterComponent. It holds an
// `int` counter for each Entity that will be incremented on calls to
// `UpdateAllEntities`.
struct CounterComponentData {
  CounterComponentData() : counter(0) {}
  int counter;
};

// The first Component is called `CounterComponent`. It will
// be a simple Component that stores an `int` counter for each Entity.
//
// Every time the Component calls `UpdateAllEntities`, each Entity's counter
// will be incremented.
class CounterComponent : public corgi::Component<CounterComponentData> {
 public:
  // This function is not used in this sample. It is only implemented
  // as part of the `ComponentInterface`, which requires it as a pure,
  // virtual function.
  virtual void AddFromRawData(EntityRef& /*entity*/, const void* /*data*/) {}

  // This function iterates through every Entity that is registered
  // with this CounterComponent and increments its counter.
  virtual void UpdateAllEntities(WorldTime);
};

// This is the Component data structure for the ScreamingComponent. It holds
// a `std::string` for each Entity that registers with the
// ScreamingComponent. This string will be output via `printf()` during calls
// to `UpdateAllEntities`.
struct ScreamingComponentData {
  ScreamingComponentData() : battle_cry("Prepare to battle!!!!!!!") {}
  std::string battle_cry;
};

// The second Component is called `ScreamingComponent`. It will
// be a simple Component that stores a `std::string` representing a
// string literal of an Entity's `battle_cry`.
//
// Every time the Component calls `UpdateAllEntities`, it will check the
// `WorldTime delta_time` since the last call. If `delta_time` is greater
// than 10 milliseconds, each Entity registered with this component will output
// its `battle_cry` via `printf()`.
class ScreamingComponent : public corgi::Component<ScreamingComponentData> {
 public:
  // This function is not used in this sample. It is only implemented
  // as part of the `ComponentInterface`, which requires it as a pure,
  // virtual function.
  virtual void AddFromRawData(EntityRef& /*entity*/, const void* /*data*/) {}

  // This function checks if the `WorldTime delta_time` is larger than
  // 10 milliseconds. If so, it iterates through every Entity that is registerd
  // with this ScreamingComponent and outputs its `battle_cry` via `printf()`.
  virtual void UpdateAllEntities(WorldTime delta_time);
};

}  // namespace sample
}  // namespace corgi

// Make sure you call `CORGI_REGISTER_COMPONENT` in the header file
// where you declare the Component and associated Component data struct. This
// is required in order to declare the necessary constants for lookups.
//
// NOTE: Remember, this should be called outside of any namespaces!
CORGI_REGISTER_COMPONENT(corgi::sample::CounterComponent,
                         corgi::sample::CounterComponentData)

CORGI_REGISTER_COMPONENT(corgi::sample::ScreamingComponent,
                         corgi::sample::ScreamingComponentData)

#endif  // ENTITY_SAMPLES_ENTITY_COMPONENT_SYSTEM_H_
