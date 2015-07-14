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

namespace fpl {

class CameraInterface {
  // An interface for 3D camera, allowing them to have position, facing, field
  // of view, etc. Libraries can use this to pass around generic cameras that
  // the game itself can implement the logic for.

 public:
  virtual mathfu::mat4 GetTransformMatrix() const = 0;

  virtual mathfu::vec3 position() const = 0;
  virtual void set_position(mathfu::vec3 position) = 0;

  virtual const mathfu::vec3& facing() const = 0;
  virtual void set_facing(const mathfu::vec3& facing) = 0;

  virtual const mathfu::vec3& up() const = 0;
  virtual void set_up(const mathfu::vec3& up) = 0;

  virtual float viewport_angle() const = 0;

  virtual mathfu::vec2 viewport_resolution() const = 0;

  virtual float viewport_near_plane() const = 0;

  virtual float viewport_far_plane() const = 0;
};

}  // fpl

#endif  // FPL_CAMERA_INTERFACE_H
