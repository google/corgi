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

#include <cstdio>

#include "breadboard/graph.h"
#include "breadboard/graph_factory.h"
#include "breadboard/graph_state.h"
#include "corgi_component_library/common_services.h"
#include "corgi_component_library/graph.h"
#include "library_components_generated.h"

CORGI_DEFINE_COMPONENT(corgi::component_library::GraphComponent,
                       corgi::component_library::GraphData)

BREADBOARD_DEFINE_EVENT(corgi::component_library::kAdvanceFrameEventId)

namespace corgi {
namespace component_library {

breadboard::NodeEventBroadcaster* GraphComponent::GetCreateBroadcaster(
    corgi::EntityRef entity) {
  auto graph_data = Data<GraphData>(entity);
  if (!graph_data) {
    graph_data = AddEntity(entity);
  }
  return &graph_data->broadcaster;
}

void GraphComponent::Init() {
  auto common_services_component =
      entity_manager_->GetComponent<CommonServicesComponent>();
  graph_factory_ = common_services_component->graph_factory();
}

void GraphComponent::AddFromRawData(corgi::EntityRef& entity,
                                    const void* raw_data) {
  GraphData* graph_data = AddEntity(entity);
  auto graph_def = static_cast<const corgi::GraphDef*>(raw_data);
  auto filename_list = graph_def->filename_list();
  graph_data->graphs.clear();
  if (filename_list) {
    for (size_t i = 0; i < filename_list->size(); ++i) {
      auto filename =
          filename_list->Get(static_cast<flatbuffers::uoffset_t>(i));
      graph_data->graphs.push_back(SerializableGraphState());
      graph_data->graphs.back().graph_state.reset(new breadboard::GraphState);
      graph_data->graphs.back().filename = filename->c_str();
    }
  }
}

void GraphComponent::EntityPostLoadFixup(corgi::EntityRef& entity) {
  graph_entity_ = entity;
  GraphData* graph_data = Data<GraphData>(entity);
  if (graph_data) {
    for (auto graph_iter = graph_data->graphs.begin();
         graph_iter != graph_data->graphs.end(); ++graph_iter) {
      breadboard::Graph* graph =
          graph_factory_->LoadGraph(graph_iter->filename.c_str());
      if (graph) {
        graph_iter->graph_state->Initialize(graph);
      }
    }
  }
}

void GraphComponent::PostLoadFixup() {
  for (auto iter = component_data_.begin(); iter != component_data_.end();
       ++iter) {
    EntityPostLoadFixup(iter->entity);
  }
}

corgi::ComponentInterface::RawDataUniquePtr GraphComponent::ExportRawData(
    const corgi::EntityRef& entity) const {
  const GraphData* data = GetComponentData(entity);
  if (data == nullptr) return nullptr;

  flatbuffers::FlatBufferBuilder fbb;
  std::vector<flatbuffers::Offset<flatbuffers::String>> filenames_vector;
  for (auto iter = data->graphs.begin(); iter != data->graphs.end(); ++iter) {
    filenames_vector.push_back(fbb.CreateString(iter->filename));
  }
  auto filenames_vector_fb = fbb.CreateVector(filenames_vector);
  corgi::GraphDefBuilder builder(fbb);
  if (filenames_vector.size() > 0) {
    builder.add_filename_list(filenames_vector_fb);
  }
  fbb.Finish(builder.Finish());
  return fbb.ReleaseBufferPointer();
}

void GraphComponent::UpdateAllEntities(corgi::WorldTime /*delta_time*/) {
  advance_frame_broadcaster_.BroadcastEvent(
      corgi::component_library::kAdvanceFrameEventId);
}

}  // fpl_project
}  // corgi
