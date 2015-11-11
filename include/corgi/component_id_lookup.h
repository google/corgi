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

#ifndef CORGI_COMPONENT_ID_LOOKUP_H_
#define CORGI_COMPONENT_ID_LOOKUP_H_

#include "corgi/entity_common.h"

namespace corgi {

/// @file
/// @addtogroup corgi_component
/// @{
///
/// @struct ComponentIdLookup
///
/// @brief A templated struct for holding type-dependent data.
///
/// @note This is typically declared via a macro
/// (e.g. CORGI_REGISTER_COMPONENT).
///
/// @tparam T The type of the data structure stored by the Component.
/// (i.e. The type that the Component was specialized for.)
template <typename T>
struct ComponentIdLookup {};

/// @def CORGI_REGISTER_COMPONENT(ComponentType, DataType)
/// Each Component needs to use this macro in its header, in order to
/// declare the necessary constants for lookups.
///
/// @note Since the macro includes its own namespace defintions, this should
/// ideally be called outside of any namespaces.
///
/// @param ComponentType The name of the Component class.
/// @param DataType The name of the struct that holds the Component data.
/// (i.e. The type that the Component was specialized for.)
#define CORGI_REGISTER_COMPONENT(ComponentType, DataType) \
  CORGI_REGISTER_COMPONENT_ID_LOOKUP(ComponentType)       \
  CORGI_REGISTER_COMPONENT_ID_LOOKUP(DataType)

/// @def CORGI_DEFINE_COMPONENT(ComponentType, DataType)
/// Handles defining the storage location for the Component for a
/// given Component type and data type. It should be placed at the
/// top of the .cpp file for that Component (after the includes but
/// before the namespaces).
///
/// @param ComponentType The name of the Component class.
/// @param DataType The name of the struct that holds the Component data.
/// (i.e. The type that the Component was specialized for.)
#define CORGI_DEFINE_COMPONENT(ComponentType, DataType) \
  CORGI_DEFINE_COMPONENT_ID_LOOKUP(ComponentType)       \
  CORGI_DEFINE_COMPONENT_ID_LOOKUP(DataType)

/// @def CORGI_REGISTER_COMPONENT_ID_LOOKUP(DataType)
/// This macro handles the lower level job of generating code to associate data
/// with a type. It is usually invoked by CORGI_REGISTER_COMPONENT, rather
/// than invoking it directly. (Since registration of a Component requires
/// multiple datatype/ID registrations.)
///
/// @param DataType The name of the struct that holds the Component data.
/// (i.e. The type that the Component was specialized for.)
#define CORGI_REGISTER_COMPONENT_ID_LOOKUP(DataType) \
  namespace corgi {                                  \
  template <>                                        \
  struct ComponentIdLookup<DataType> {               \
    static ComponentId component_id;                 \
  };                                                 \
  }

/// @def CORGI_DEFINE_COMPONENT_ID_LOOKUP(DataType)
/// This macro handles defining the storage location for the Component ID
/// for a given data type. It is usually invoked by
/// CORGI_DEFINE_COMPONENT, rather than invoking it directly.
///
/// @param DataType The name of the struct that holds the Component data.
/// (i.e. The type that the Component was specialized for.)
#define CORGI_DEFINE_COMPONENT_ID_LOOKUP(DataType)                           \
  namespace corgi {                                                          \
  ComponentId ComponentIdLookup<DataType>::component_id = kInvalidComponent; \
  }

/// @}

}  // corgi

#endif  // CORGI_COMPONENT_ID_LOOKUP_H_
