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

#ifndef COMPONENT_LIBRARY_RENDERMESH_H_
#define COMPONENT_LIBRARY_RENDERMESH_H_

#include "component_library/camera_interface.h"
#include "component_library/transform.h"
#include "entity/component.h"
#include "fplbase/asset_manager.h"
#include "fplbase/mesh.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"
#include "library_components_generated.h"
#include "mathfu/constants.h"
#include "mathfu/glsl_mappings.h"
#include "mathfu/matrix_4x4.h"

namespace fpl {
namespace component_library {

// Data for scene object components.
struct RenderMeshData {
 public:
  RenderMeshData()
      : mesh(nullptr),
        shader(nullptr),
        tint(mathfu::kOnes4f),
        mesh_filename(""),
        shader_filename(""),
        z_depth(0),
        culling_mask(0),
        pass_mask(0),
        default_hidden(false),
        currently_hidden(false),
        default_pose(false) {}
  Mesh* mesh;
  Shader* shader;
  mathfu::vec4 tint;
  std::string mesh_filename;
  std::string shader_filename;
  float z_depth;
  unsigned char culling_mask;
  unsigned char pass_mask;
  bool default_hidden;
  bool currently_hidden;
  bool default_pose;
  std::vector<mathfu::mat4> shader_transforms;
};

// Struct used for keeping track of and sorting our render lists:
struct RenderlistEntry {
  RenderlistEntry(entity::EntityRef entity_, RenderMeshData* data_)
      : entity(entity_), data(data_) {}
  entity::EntityRef entity;
  RenderMeshData* data;

  bool operator<(const RenderlistEntry& other) const {
    return (data->z_depth < other.data->z_depth);
  }

  bool operator>(const RenderlistEntry& other) const {
    return (data->z_depth > other.data->z_depth);
  }
};

class RenderMeshComponent : public entity::Component<RenderMeshData> {
 public:
  static const int kDefaultCullDist = 80;

  RenderMeshComponent() : asset_manager_(nullptr),
    culling_distance_squared(kDefaultCullDist * kDefaultCullDist) {}

  virtual void Init();
  virtual void AddFromRawData(entity::EntityRef& entity, const void* raw_data);
  virtual RawDataUniquePtr ExportRawData(const entity::EntityRef& entity) const;

  virtual void InitEntity(entity::EntityRef& /*entity*/);

  // Nothing really happens per-update to these things.
  virtual void UpdateAllEntities(entity::WorldTime /*delta_time*/) {}

  // Prepares to do rendering.  (Must be called before any RenderPass calls.)
  // Pregenerates the draw lists, sorted by z-depth, and applies frustrum
  // culling.  (So that renderpass can just iterate through the list and draw
  // everything with minimum of fuss.)
  void RenderPrep(const CameraInterface& camera);

  // Renders all entities marked as being part of the specified renderpass to
  // the current output buffer.  Shader_override is an optional parameter, which
  // (if provided) overrides the mesh-specified shader, and instead renders
  // the entire pass with override_shader.
  void RenderPass(int pass_id, const CameraInterface& camera,
                  Renderer& renderer);
  void RenderPass(int pass_id, const CameraInterface& camera,
                  Renderer& renderer, const Shader* shader_override);

  // Goes through and renders every entity that is visible from the camera,
  // in pass order.  Is equivalent to iterating through all of the passes
  // and calling RenderPass on each one.
  // Important!  You must have called RenderPrep first, in order to
  // pre-populate the lists of things visible from the camera!
  void RenderAllEntities(Renderer& renderer, const CameraInterface& camera);

  // Recursively sets the hidden-ness of the entity and all children.
  void SetHiddenRecursively(const entity::EntityRef& entity, bool hidden);

  // Get and set the light position.  This is a special uniform that is sent
  // to all shaders without having to declare it explicitly in the
  // shader_instance variable.
  mathfu::vec3 light_position() { return light_position_; }
  void set_light_position(const mathfu::vec3& light_position) {
    light_position_ = light_position;
  }

  void SetCullDistance(float distance) {
    culling_distance_squared = distance * distance;
  }

 private:
  // todo(ccornell) expand this if needed - make an array for multiple lights.
  // Also maybe make this into a full fledged struct to store things like
  // intensity, color, etc.  (Low priority - none of our shaders support
  // these.)
  mathfu::vec3 light_position_;
  AssetManager* asset_manager_;
  float culling_distance_squared;
  // An array of vectors we use for keeping track of things we're going
  // to render.
  std::vector<RenderlistEntry> pass_render_list[RenderPass_Count];
};

}  // component_library
}  // fpl

FPL_ENTITY_REGISTER_COMPONENT(fpl::component_library::RenderMeshComponent,
                              fpl::component_library::RenderMeshData)

#endif  // COMPONENT_LIBRARY_RENDERMESH_H_
