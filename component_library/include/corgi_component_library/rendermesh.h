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

#ifndef CORGI_COMPONENT_LIBRARY_RENDERMESH_H_
#define CORGI_COMPONENT_LIBRARY_RENDERMESH_H_

#include "corgi/component.h"
#include "corgi_component_library/camera_interface.h"
#include "corgi_component_library/transform.h"
#include "fplbase/asset_manager.h"
#include "fplbase/mesh.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"
#include "library_components_generated.h"
#include "mathfu/constants.h"
#include "mathfu/glsl_mappings.h"
#include "mathfu/matrix_4x4.h"

namespace corgi {
namespace component_library {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @struct RenderMeshData
///
/// @brief The per-Entity mesh and shader data.
struct RenderMeshData {
 public:
  /// @brief Default constructor for RenderMeshData.
  RenderMeshData()
      : mesh(nullptr),
        tint(mathfu::kOnes4f),
        mesh_filename(""),
        z_depth(0),
        culling_mask(0),
        pass_mask(0),
        visible(true),
        initialized(false),
        default_pose(false),
        num_shader_transforms(0),
        shader_transforms(nullptr),
        debug_name("") {}

  /// @brief Destructor for RenderMeshData.
  ~RenderMeshData() {
    delete[] shader_transforms;
    shader_transforms = nullptr;
  }

  /// @brief The move constructor for RenderMeshData to avoid reallocating
  /// shader transforms.
  ///
  /// @param[in] other The other RenderMeshData whose data should be moved
  /// into this RenderMeshData.
  RenderMeshData(RenderMeshData&& other) { *this = std::move(other); }

  /// @brief The move assignment operator for RenderMeshData to avoid
  /// reallocating shader transforms.
  ///
  /// @param[in] other The other RenderMeshData whose data should be moved
  /// into this RenderMeshData.
  ///
  /// @return Returns a reference to the RenderMeshData that received the
  /// data from `other`.
  RenderMeshData& operator=(RenderMeshData&& other) {
    mesh = std::move(other.mesh);
    shaders = std::move(other.shaders);
    tint = std::move(other.tint);
    mesh_filename = std::move(other.mesh_filename);
    shader_filenames = std::move(other.shader_filenames);
    z_depth = std::move(other.z_depth);
    culling_mask = std::move(other.culling_mask);
    pass_mask = std::move(other.pass_mask);
    visible = std::move(other.visible);
    initialized = std::move(other.initialized);
    default_pose = std::move(other.default_pose);
    num_shader_transforms = other.num_shader_transforms;
    shader_transforms = other.shader_transforms;
    other.shader_transforms = nullptr;
    other.num_shader_transforms = 0;
    debug_name = std::move(other.debug_name);
    return *this;
  }

  /// @brief The fplbase::Mesh for this Entity.
  fplbase::Mesh* mesh;

  /// @brief A vector of fplbase::Shader for this Entity.
  std::vector<fplbase::Shader*> shaders;

  /// @brief A mathfu::vec4 specifying the tinting for the Entity in RGBA.
  mathfu::vec4 tint;

  /// @brief A std::string of the filename for the mesh, used for exporting.
  std::string mesh_filename;

  /// @brief A std::string of the filenames for the shaders, used for exporting.
  std::vector<std::string> shader_filenames;

  /// @brief The z distance corresponding to the depth of where the
  /// Entity should be rendered.
  float z_depth;

  /// @brief A bit field determining which types of culling are applied to the
  /// Entity.
  unsigned char culling_mask;

  /// @brief A bit field determining during which render passes to render this
  /// Entity.
  unsigned char pass_mask;

  /// @brief A bool determining if this Entity should be rendered.
  bool visible;

  /// @brief A bool indicating if this RenderMesh is initialized.
  bool initialized;

  /// @brief A bool determining if the Entity should be rendered in the
  /// default pose.
  bool default_pose;

  /// @brief The number of shader transforms in the `shader_transforms`
  /// array.
  uint8_t num_shader_transforms;

  /// @brief A mathfu::AffineTransform array that contains the shader
  /// transforms.
  mathfu::AffineTransform* shader_transforms;

  /// @brief The debug name of this mesh
  std::string debug_name;

 private:
  // Disallow copies. They're inefficient with the shader_transforms array.
  RenderMeshData(const RenderMeshData&);
  RenderMeshData& operator=(const RenderMeshData&);
};

/// @brief Struct used for keeping track of and sorting our render lists.
struct RenderlistEntry {
  /// @brief Constructor for RenderlistEntry.
  RenderlistEntry(EntityRef entity_, RenderMeshData* data_)
      : entity(entity_), data(data_) {}

  /// @brief The Entity associated with this RenderlistEntry.
  EntityRef entity;

  /// @brief The RenderMeshData associated with the `entity` in this
  /// RenderlistEntry.
  RenderMeshData* data;

  /// @brief The greater than operator for RenderlistEntry.
  ///
  /// @param[in] other The other RenderlistEntry to compare if its
  /// `RenderMeshData->z_depth` is greater than this RenderlistEntry's.
  ///
  /// @return Returns `true` if `other`'s `RenderMeshData->z_depth` is greater
  /// than this RenderlistEntry's. Otherwise, it returns `false`.
  bool operator<(const RenderlistEntry& other) const {
    return (data->z_depth < other.data->z_depth);
  }

  /// @brief The less than operator for RenderlistEntry.
  ///
  /// @param[in] other The other RenderlistEntry to compare if its
  /// `RenderMeshData->z_depth` is less than this RenderlistEntry's.
  ///
  /// @return Returns `true` if `other`'s `RenderMeshData->z_depth` is less
  /// than this RenderlistEntry's. Otherwise, it returns `false`.
  bool operator>(const RenderlistEntry& other) const {
    return (data->z_depth > other.data->z_depth);
  }
};

/// @brief A Component that handles the rendering of each Entities
/// mesh that is registered with this Component.
class RenderMeshComponent : public Component<RenderMeshData> {
 public:
  /// @brief The default distance to start culling.
  static const int kDefaultCullDist = 80;

  /// @brief Default constructor for RenderMeshComponent.
  RenderMeshComponent()
      : asset_manager_(nullptr),
        culling_distance_squared_(kDefaultCullDist * kDefaultCullDist) {}

  /// @brief Destructor for RenderMeshComponent.
  virtual ~RenderMeshComponent() {}

  /// @brief Initialize the RenderMeshComponent with the AssetManager from
  /// the EntityManager.
  virtual void Init();

  /// @brief Deserialize a flat binary buffer to create and populate an Entity
  /// from raw data.
  ///
  /// @param[in,out] entity An EntityRef reference that points to the Entity
  /// that is being added from the raw data.
  /// @param[in] raw_data A void pointer to the raw FlatBuffer data.
  virtual void AddFromRawData(EntityRef& entity, const void* raw_data);

  /// @brief Serializes a RenderMeshComponent's data for a given Entity.
  ///
  /// @param[in] entity An EntityRef reference that points to the Entity whose
  /// corresponding RenderMeshData will be serialized.
  ///
  /// @return Returns a RawDataUniquePtr to the start of the raw data in a
  /// flat binary buffer.
  virtual RawDataUniquePtr ExportRawData(const EntityRef& entity) const;

  /// @brief Initialize an Entity by also adding it to the TransformComponent.
  virtual void InitEntity(EntityRef& /*entity*/);

  /// @brief Nothing happens per frame for these Entities.
  virtual void UpdateAllEntities(WorldTime /*delta_time*/) {}

  /// @brief Prepares to do rendering.
  ///
  /// @note Must be called before any `RenderPass()` calls.
  ///
  /// Pre-generates the draw lists, sorted by z-depth, and applies frustrum
  /// culling.  (So that `RenderPass` can just iterate through the list and draw
  /// everything easily.)
  ///
  /// @param[in] camera A CameraInterface reference to the camera used to render
  /// within the field of view.
  void RenderPrep(const CameraInterface& camera);

  /// @brief Renders all Entities that are marked as being part of a given
  /// render pass to the current output buffer.
  ///
  /// @param[in] pass_id An int used to identify and index the desired render
  /// pass.
  /// @param[in] camera A CameraInterface reference to the camera used to render
  /// within the field of view.
  /// @param[out] renderer A reference to the fplbase::Renderer to capture the
  /// output of the render pass.
  void RenderPass(int pass_id, const CameraInterface& camera,
                  fplbase::Renderer& renderer);

  /// @brief Renders all Entities that are marked as being part of a given
  /// render pass to the current output buffer.
  ///
  /// @param[in] pass_id An int used to identify and index the desired render
  /// pass.
  /// @param[in] camera A CameraInterface reference to the camera used to render
  /// within the field of view.
  /// @param[out] renderer A reference to the fplbase::Renderer to capture the
  /// output of the render pass.
  /// @param[in] shader_index The shader index for the fplbase::Shader to use as
  /// the mesh shader for this render pass.
  void RenderPass(int pass_id, const CameraInterface& camera,
                  fplbase::Renderer& renderer,
                  size_t shader_index);

  /// @brief Goes through and renders every Entity that is visible from the
  /// camera, in pass order.
  ///
  /// It Is equivalent to iterating through all of the passes and calling
  /// `RenderPass()` on each one.
  ///
  /// @warning You must have called `RenderPrep()` first, in order to
  /// pre-populate the lists of things visible from the camera!
  ///
  /// @param[out] renderer A reference to the fplbase::Renderer to capture the
  /// output of the render pass.
  /// @param[in] camera A CameraInterface reference to the camera used to
  /// render within the field of view.
  void RenderAllEntities(fplbase::Renderer& renderer,
                         const CameraInterface& camera);

  /// @brief Recursively sets the visibility of the Entity and all of its
  /// children.
  ///
  /// @param[in] entity An EntityRef reference to the Entity that should have
  /// itself and its children's visibility set.
  /// @param[in] visible A bool determining if the Entities should be rendered.
  void SetVisibilityRecursively(const EntityRef& entity, bool visible);

  /// @brief Get the light position uniform.
  ///
  /// @note This is a special uniform that is sent to all shaders without having
  /// to declare it explicitly in the shader.
  ///
  /// @return Returns the light position uniform as a mathfu::vec3.
  mathfu::vec3 light_position() { return light_position_; }

  /// @brief Set the light position uniform.
  ///
  /// @note This is a special uniform that is sent to all shaders without having
  /// to declare it explicitly in the shader.
  ///
  /// @param[in] light_position A const mathfu::vec3 reference to the uniform to
  /// set for the light position.
  void set_light_position(const mathfu::vec3& light_position) {
    light_position_ = light_position;
  }

  /// @brief Set the culling distance.
  ///
  /// @param[in] distance A float representing the new culling distance to set.
  void SetCullDistance(float distance) {
    culling_distance_squared_ = distance * distance;
  }

  /// @brief Get the culling distance squared.
  ///
  /// @return Returns the square of the culling distance.
  float culling_distance_squared() const { return culling_distance_squared_; }

  /// @brief Set the square of the culling distance.
  ///
  /// @param[in] culling_distance_squared A float representing the square of the
  /// culling distance that should be set.
  void set_culling_distance_squared(float culling_distance_squared) {
    culling_distance_squared_ = culling_distance_squared;
  }

 private:
  // Finalize the initialization of RenderMeshData if it's not completed yet.
  // This function should be called right after the corresponding mesh is
  // loaded.
  void FinalizeRenderMeshDataIfRequired(RenderMeshData* rendermesh_data);

  // todo(ccornell) expand this if needed - make an array for multiple lights.
  // Also maybe make this into a full fledged struct to store things like
  // intensity, color, etc.  (Low priority - none of our shaders support
  // these.)
  mathfu::vec3 light_position_;
  fplbase::AssetManager* asset_manager_;
  float culling_distance_squared_;
  // An array of vectors we use for keeping track of things we're going
  // to render.
  std::vector<RenderlistEntry> pass_render_list_[RenderPass_Count];
};

/// @}

}  // component_library
}  // corgi

CORGI_REGISTER_COMPONENT(corgi::component_library::RenderMeshComponent,
                         corgi::component_library::RenderMeshData)

#endif  // CORGI_COMPONENT_LIBRARY_RENDERMESH_H_
