# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH:=$(call my-dir)

# Project directory relative to this file.
CORGI_COMPONENT_LIBRARY_DIR:=$(LOCAL_PATH)/../..
include $(CORGI_COMPONENT_LIBRARY_DIR)/jni/android_config.mk

include $(CLEAR_VARS)
LOCAL_PATH:=$(DEPENDENCIES_BULLETPHYSICS_DIR)/src

LOCAL_MODULE := libbullet
LOCAL_MODULE_FILENAME := libbullet

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/ \
    $(LOCAL_PATH)/BulletCollision/BroadphaseCollision \
    $(LOCAL_PATH)/BulletCollision/CollisionDispatch \
    $(LOCAL_PATH)/BulletCollision/CollisionShapes \
    $(LOCAL_PATH)/BulletCollision/NarrowPhaseCollision \
    $(LOCAL_PATH)/BulletDynamics/ConstraintSolver \
    $(LOCAL_PATH)/BulletDynamics/Dynamics \
    $(LOCAL_PATH)/BulletDynamics/Vehicle \
    $(LOCAL_PATH)/LinearMath \

LOCAL_ARM_MODE := arm

bullet_cpp_in_subdirectory = \
  $(call fplutil_all_files_relative_to_path_in_subdirectory,$(LOCAL_PATH),$(1),cpp)

BULLET_COLLISION_SRC_FILES := \
    $(call bullet_cpp_in_subdirectory,BulletCollision)
BULLET_DYNAMICS_SRC_FILES := \
    $(call bullet_cpp_in_subdirectory,BulletDynamics)
BULLET_LINEARMATH_SRC_FILES := \
    $(call bullet_cpp_in_subdirectory,LinearMath)

LOCAL_SRC_FILES := \
    $(BULLET_COLLISION_SRC_FILES) \
    $(BULLET_DYNAMICS_SRC_FILES) \
    $(BULLET_LINEARMATH_SRC_FILES)

include $(BUILD_STATIC_LIBRARY)
