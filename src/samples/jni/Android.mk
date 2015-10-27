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

LOCAL_PATH := $(call my-dir)/..

ENTITY_DIR := $(LOCAL_PATH)/../..

include $(CLEAR_VARS)
LOCAL_MODULE := entity_component_system_sample
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := entity
LOCAL_SRC_FILES := entity_component_system.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := android_native_app_glue libfplutil_main \
        libfplutil_print
include $(BUILD_SHARED_LIBRARY)

$(call import-add-path, $(ENTITY_DIR)/..)
$(call import-module, entity/jni)
$(call import-add-path, $(ENTITY_DIR)/../fplutil/)
$(call import-module, android/native_app_glue)
$(call import-module, libfplutil/jni)
