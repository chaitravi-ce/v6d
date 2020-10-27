/** Copyright 2020 Alibaba Group Holding Limited.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#include <cinttypes>
#endif

#include "common/util/uuid.h"

#include <cstdio>
#include <cstdlib>

namespace vineyard {

const std::string VYObjectIDToString(const ObjectID id) {
  char buffer[17] = {'\0'};
  std::snprintf(buffer, sizeof(buffer), "%016" PRIx64, id);
  return std::string(buffer);
}

ObjectID VYObjectIDFromString(const std::string& s) {
  return strtoull(s.c_str(), nullptr, 16);
}

ObjectID VYObjectIDFromString(const char* s) {
  return strtoull(s, nullptr, 16);
}

}  // namespace vineyard
