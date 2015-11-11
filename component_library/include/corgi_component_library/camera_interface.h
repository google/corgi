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

#ifndef FPL_CAMERA_INTERFACE_H
#define FPL_CAMERA_INTERFACE_H

#include "mathfu/glsl_mappings.h"

namespace corgi {

/// @file
/// @addtogroup corgi_component_library
/// @{
///
/// @class CameraInterface
///
/// @brief An interface for 3D cameras, allowing them to have position, facing
/// field of view, etc. Libraries can use this to pass around generic cameras
/// that the game itself can implement the logic for.
class CameraInterface {
 public:
  /// @brief The destructor for the CameraInterface.
  virtual ~CameraInterface() {}

  /// @brief Get the view/projection matrix.
  ///
  /// @return Returns the view/projection matrix as a mathfu::mat4.
  virtual mathfu::mat4 GetTransformMatrix() const = 0;

  /// @brief Get the view matrix.
  ///
  /// @return Returns the view matrix as a mathfu::mat4.
  virtual mathfu::mat4 GetViewMatrix() const = 0;

  /// @brief Get the view/projection matrix at a given index.
  ///
  /// @param[in] index The index of the desired matrix.
  ///
  /// @return Returns the view/projection matrix as a mathfu::mat4.
  virtual mathfu::mat4 GetTransformMatrix(int32_t index) const = 0;

  /// @brief Get the view matrix at a given index.
  ///
  /// @param[in] index The index of the desired matrix.
  ///
  /// @return Returns the view matrix as a mathfu::mat4.
  virtual mathfu::mat4 GetViewMatrix(int32_t index) const = 0;

  /// @brief Get the camera's world position,.
  ///
  /// @return Returns the camera's position as a mathfu::vec3.
  virtual mathfu::vec3 position() const = 0;

  /// @brief Get the camera's world position at a given index.
  ///
  /// @param[in] index The index of the camera whose position should be
  /// returned.
  ///
  /// @return Returns the camera's position as a mathfu::vec3.
  virtual mathfu::vec3 position(int32_t index) const = 0;

  /// @brief Set the camera's world position.
  ///
  /// @param[in] position A const mathfu::vec3 reference to the world position
  /// to set.
  virtual void set_position(const mathfu::vec3& position) = 0;

  /// @brief Set the camera's world position.
  ///
  /// @param[in] index The index of the camera whose position should be set.
  /// @param[in] position A const mathfu::vec3 reference to the world position
  /// to set.
  virtual void set_position(int32_t index, const mathfu::vec3& position) = 0;

  /// @brief Get the camera's forward direction.
  ///
  /// @return Returns the camera's forward direction as a const mathfu::vec3
  /// reference.
  virtual const mathfu::vec3& facing() const = 0;

  /// @brief Set the camera's forward direction.
  ///
  /// @param[in] facing A const mathfu::vec3 reference to the direction the
  /// camera should face.
  virtual void set_facing(const mathfu::vec3& facing) = 0;

  /// @brief Get the camera's up direction.
  ///
  /// @return Returns the camera's up direction as a const mathfu::vec3
  /// reference.
  virtual const mathfu::vec3& up() const = 0;

  /// @brief Set the camera's up direction.
  ///
  /// @param[in] up A const mathfu::vec3 reference to the up direction to
  /// set for the camera.
  virtual void set_up(const mathfu::vec3& up) = 0;

  /// @brief Set the viewport angle.
  ///
  /// @param[in] viewport_angle A float representing the viewport angle, in
  /// radians.
  virtual void set_viewport_angle(float viewport_angle) = 0;

  /// @brief Get the viewport angle.
  ///
  /// @return Returns the viewport angle, in radians.
  virtual float viewport_angle() const = 0;

  /// @brief Set the camera's viewport resolution.
  ///
  /// @param[in] viewport_resolution A mathfu::vec2 representing the viewport
  /// resolution.
  virtual void set_viewport_resolution(mathfu::vec2 viewport_resolution) = 0;

  /// @brief Get the viewport resolution.
  ///
  /// @return Returns the viewport resolution as a mathfu::vec2.
  virtual mathfu::vec2 viewport_resolution() const = 0;

  /// @brief Set the distance to the near clipping plane.
  ///
  /// @param[in] viewport_near_plane A float distance to the near clipping
  /// plane.
  virtual void set_viewport_near_plane(float viewport_near_plane) = 0;

  /// @brief Get the distance to the near clipping plane.
  ///
  /// @return Returns the float distance to the near clipping plane.
  virtual float viewport_near_plane() const = 0;

  /// @brief Set the distance to the far clipping plane.
  ///
  /// @param[in] viewport_far_plane A float distance to the far clipping
  /// plane.
  virtual void set_viewport_far_plane(float viewport_far_plane) = 0;

  /// @brief Get the distance to the far clipping plane.
  ///
  /// @return Returns the float distance to the far clipping plane.
  virtual float viewport_far_plane() const = 0;

  /// @brief Sets the camera's viewport.
  ///
  /// @param[in] viewport A const mathfu::vec4i reference to the viewport that
  /// should be set.
  virtual void set_viewport(const mathfu::vec4i& viewport) = 0;

  /// @brief Sets the camera's viewport at a given index.
  ///
  /// @param[in] index The index of the viewport to set.
  /// @param[in] viewport A const mathfu::vec4i reference to the viewport that
  /// should be set.
  virtual void set_viewport(int32_t index, const mathfu::vec4i& viewport) = 0;

  /// @brief Get the camera's viewport at a given index.
  ///
  /// @param[in] index The index of the desired viewport.
  ///
  /// @return Returns the camera viewport as a const mathfu::vec4i reference.
  virtual const mathfu::vec4i& viewport(int32_t index) const = 0;

  /// @brief Check if the camera is stereoscopic.
  ///
  /// @return Returns `true` if this camera is stereoscopic. Otherwise it
  /// returns `false`.
  virtual bool IsStereo() const = 0;

  /// @brief Set a camera as stereoscopic.
  ///
  /// @param[in] b A bool determining if the camera is stereoscopic or not.
  virtual void set_stereo(bool b) = 0;
};
/// @}

}  // corgi

#endif  // FPL_CAMERA_INTERFACE_H
