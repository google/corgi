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

#ifndef CORGI_COMPONENTS_GRAPH_H_
#define CORGI_COMPONENTS_GRAPH_H_

#include <memory>

#include "breadboard/event.h"
#include "breadboard/graph.h"
#include "breadboard/graph_factory.h"
#include "breadboard/graph_state.h"
#include "corgi/component.h"

namespace corgi {
namespace component_library {

BREADBOARD_DECLARE_EVENT(kAdvanceFrameEventId)

struct SerializableGraphState {
  std::string filename;
  std::unique_ptr<breadboard::GraphState> graph_state;

  SerializableGraphState() {}
  SerializableGraphState(SerializableGraphState&& other) {
    *this = std::move(other);
  }

  SerializableGraphState& operator=(SerializableGraphState&& other) {
    filename = std::move(other.filename);
    graph_state = std::move(other.graph_state);
    return *this;
  }

#ifdef _MSC_VER
  // Provide fake versions of copy contructor and operator to let VS2012
  // compile and link.
  //
  // These should never be called, since unique_ptr is not copyable.
  // However, Visual Studio 2012 not only calls them, but links them in,
  // when this class is in an std::pair (as it is when in an std::map).
  //
  // The asserts never get hit here. These functions are not actually called.
  SerializableGraphState(const SerializableGraphState&) { assert(false); }
  SerializableGraphState& operator=(const SerializableGraphState&) {
    assert(false);
    return *this;
  }
#endif  // _MSC_VER
};

struct GraphData {
  std::vector<SerializableGraphState> graphs;
  breadboard::NodeEventBroadcaster broadcaster;

  GraphData() {}
  GraphData(GraphData&& other) { *this = std::move(other); }

  GraphData& operator=(GraphData&& other) {
    graphs = std::move(other.graphs);
    broadcaster = std::move(other.broadcaster);
    return *this;
  }

 private:
  GraphData(GraphData&);
  GraphData& operator=(GraphData&);
};

class GraphComponent : public corgi::Component<GraphData> {
 public:
  // Once entities themselves have been initialized, initialize the graphs. This
  // must be done after because graphs may reference entities.
  //
  // PostLoadFixup will fix up all loaded entities. If you create single
  // entities later, individual calls to EntityPostLoadFixup are required on a
  // per-entity basis.
  void PostLoadFixup();
  void EntityPostLoadFixup(corgi::EntityRef& entity);

  // Gets the broadcaster for an entity, even if that entity does not yet have
  // one.
  breadboard::NodeEventBroadcaster* GetCreateBroadcaster(
      corgi::EntityRef entity);

  virtual void Init();
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;
  virtual void InitEntity(corgi::EntityRef& /*entity*/) {}
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  const corgi::EntityRef& graph_entity() const { return graph_entity_; }

  breadboard::NodeEventBroadcaster* advance_frame_broadcaster() {
    return &advance_frame_broadcaster_;
  }

 private:
  breadboard::GraphFactory* graph_factory_;
  breadboard::NodeEventBroadcaster advance_frame_broadcaster_;

  // TODO: Remove this in favor of a more generic per-graph state interface.
  // b/24510652
  corgi::EntityRef graph_entity_;
};

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::GraphComponent,
                         corgi::component_library::GraphData)

#endif  // CORGI_COMPONENTS_GRAPH_H_
