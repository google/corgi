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

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @struct SerializableGraphState
///
/// @brief Contains the graph state information, as well as the
/// filename for serialization.
struct SerializableGraphState {
  /// @brief The filename for serialization.
  std::string filename;

  /// @brief The Breadboard graph state data.
  std::unique_ptr<breadboard::GraphState> graph_state;

  /// @brief Default constructor for a SerializableGraphState.
  SerializableGraphState() {}

  /// @brief Move constructor for a SerializableGraphState.
  ///
  /// @param[in] other A SerializableGraphState whose data will be
  /// moved into this SerializableGraphState.
  SerializableGraphState(SerializableGraphState&& other) {
    *this = std::move(other);
  }

  /// @brief Move assignment operator for SerializableGraphState.
  ///
  /// @param[in] other A SerializableGraphState whose data will be
  /// moved into this SerializableGraphState.
  SerializableGraphState& operator=(SerializableGraphState&& other) {
    filename = std::move(other.filename);
    graph_state = std::move(other.graph_state);
    return *this;
  }

/// @cond COMPONENT_LIBRARY_INTERNAL
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
/// @endcond
};

/// @struct GraphData
///
/// @brief Contains the data for each Entity registered with the GraphComponent.
struct GraphData {
  /// @var graphs
  ///
  /// @brief A vector of all the graph states.
  std::vector<SerializableGraphState> graphs;

  /// @var broadcaster
  ///
  /// @brief A NodeEventBroadcaster that broadcasts events to any registered
  /// NodeEventListeners.
  breadboard::NodeEventBroadcaster broadcaster;

  /// @brief The default constructor for an empty GraphData.
  GraphData() {}

  /// @brief The move constructor for a GraphData.
  ///
  /// @param[in] other A GraphData whose data will be moved into this
  /// GraphData.
  GraphData(GraphData&& other) { *this = std::move(other); }

  /// @brief The move assignment operator for GraphData.
  ///
  /// @param[in] other The GraphData whose data will be move into this
  /// GraphData.
  ///
  /// @return Returns a reference to the
  GraphData& operator=(GraphData&& other) {
    graphs = std::move(other.graphs);
    broadcaster = std::move(other.broadcaster);
    return *this;
  }

 private:
  GraphData(GraphData&);
  GraphData& operator=(GraphData&);
};

/// @class GraphComponent
///
/// @brief Manages the event graphs for any Entity that wishes to utilize the
/// event system.
///
/// @note Once the Entities themselves have been initialized, initialize the
/// graphs. The graphs must be initialized after the Entities because the graphs
/// may reference the Entities.
class GraphComponent : public corgi::Component<GraphData> {
 public:
  /// @brief Fixes up all loaded Entities.
  ///
  /// @note If you create single Entities later, individual calls to
  /// `EntityPostLoadFixup` are required on a per-Entity basis.
  void PostLoadFixup();

  /// @brief Fixes up a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to the Entity that will
  /// be fixed up.
  void EntityPostLoadFixup(corgi::EntityRef& entity);

  /// @brief Get the broadcaster for a given Entity.
  ///
  /// @param[in] entity An EntityRef to the Entity whose broadcaster should be
  /// returned.
  ///
  /// @return Returns a NodeEventBroadcaster for the Entity, even if it does not
  /// yet have one.
  breadboard::NodeEventBroadcaster* GetCreateBroadcaster(
      corgi::EntityRef entity);

  /// @brief Destructor for GraphComponent.
  virtual ~GraphComponent() {}

  /// @brief Initialize the GraphComponent.
  virtual void Init();

  /// @brief Deserialize a flat binary buffer to create and populate an Entity
  /// from raw data.
  ///
  /// @param[in,out] entity An EntityRef reference that points to an Entity that
  /// is being added from the raw data.
  /// @param[in] raw_data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(corgi::EntityRef& entity, const void* raw_data);

  /// @brief Serializes a GraphComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference to an Entity whose corresponding
  /// GraphData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a flat
  /// binary buffer.
  virtual RawDataUniquePtr ExportRawData(const corgi::EntityRef& entity) const;

  /// @cond COMPONENT_LIBRARY_INTERNAL
  virtual void InitEntity(corgi::EntityRef& /*entity*/) {}
  /// @endcond

  /// @brief Broadcasts a `kAdvanceFrameEventId` event to update all Entities'
  /// graphs.
  ///
  /// @param[in] delta_time A WorldTime corresponding to the delta time
  /// since the last call to this function.
  virtual void UpdateAllEntities(corgi::WorldTime delta_time);

  /// @return Returns a const EntityRef reference to the Entity that is
  /// currently being initialized.
  ///
  /// During initialization, this value iterates over each Entity, one at a
  /// time.
  ///
  /// @note After initialization, the return value from this function is not
  /// meaningful. It would simply return the last Entity that was initialized.
  const corgi::EntityRef& graph_entity() const { return graph_entity_; }

  /// @return Returns a pointer to the NodeEventBroadcaster that is called
  /// once per frame.
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
/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::GraphComponent,
                         corgi::component_library::GraphData)

#endif  // CORGI_COMPONENTS_GRAPH_H_
