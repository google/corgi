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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := component_library
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := libfplbase libbullet
LOCAL_SHARED_LIBRARIES :=

COMPONENTS_RELATIVE_DIR := ..
COMPONENTS_DIR := $(LOCAL_PATH)/$(COMPONENTS_RELATIVE_DIR)

include $(COMPONENTS_DIR)/jni/android_config.mk
include $(DEPENDENCIES_FLATBUFFERS_DIR)/android/jni/include.mk

LOCAL_EXPORT_C_INCLUDES := \
  $(COMPONENTS_DIR)/include \
  $(COMPONENTS_GENERATED_OUTPUT_DIR)

LOCAL_C_INCLUDES := \
  $(LOCAL_EXPORT_C_INCLUDES) \
  $(DEPENDENCIES_FLATBUFFERS_DIR)/include \
  $(DEPENDENCIES_ENTITY_DIR)/include \
  $(DEPENDENCIES_EVENT_DIR)/include \
  $(DEPENDENCIES_FPLBASE_DIR)/include \
  $(DEPENDENCIES_FPLUTIL_DIR)/libfplutil/include \
  $(DEPENDENCIES_MATHFU_DIR)/include \
  $(DEPENDENCIES_BULLETPHYSICS_DIR)/src \
  $(COMPONENTS_DIR)/src

LOCAL_SRC_FILES := \
	$(COMPONENTS_RELATIVE_DIR)/src/common_services.cpp \
	$(COMPONENTS_RELATIVE_DIR)/src/meta.cpp \
	$(COMPONENTS_RELATIVE_DIR)/src/entity_factory.cpp \
	$(COMPONENTS_RELATIVE_DIR)/src/physics.cpp \
	$(COMPONENTS_RELATIVE_DIR)/src/rendermesh.cpp \
	$(COMPONENTS_RELATIVE_DIR)/src/transform.cpp

COMPONENTS_SCHEMA_DIR := $(COMPONENTS_DIR)/schemas
COMPONENTS_SCHEMA_INCLUDE_DIRS := $(DEPENDENCIES_FPLBASE_DIR)/schemas

COMPONENTS_SCHEMA_FILES := \
  $(COMPONENTS_SCHEMA_DIR)/component_library_events.fbs \
  $(COMPONENTS_SCHEMA_DIR)/library_components.fbs \
  $(COMPONENTS_SCHEMA_DIR)/bullet_def.fbs

ifeq (,$(COMPONENTS_RUN_ONCE))
COMPONENTS_RUN_ONCE := 1
$(call flatbuffers_header_build_rules, \
  $(COMPONENTS_SCHEMA_FILES), \
  $(COMPONENTS_SCHEMA_DIR), \
  $(COMPONENTS_GENERATED_OUTPUT_DIR), \
  $(COMPONENTS_SCHEMA_INCLUDE_DIRS), \
  $(LOCAL_SRC_FILES))
endif

include $(BUILD_STATIC_LIBRARY)

$(call import-add-path,$(DEPENDENCIES_FLATBUFFERS_DIR)/..)
$(call import-add-path,$(DEPENDENCIES_FPLBASE_DIR)/..)
$(call import-add-path,$(DEPENDENCIES_MATHFU_DIR)/..)

$(call import-module,flatbuffers/android/jni)
$(call import-module,fplbase/jni)
$(call import-module,mathfu/jni)
$(call import-module,android/cpufeatures)
