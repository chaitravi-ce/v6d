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

#include "basic/ds/types.h"

namespace vineyard {

AnyType ParseAnyType(const std::string& type_name) {
  if (type_name == "int32") {
    return AnyType::Int32;
  } else if (type_name == "uint32") {
    return AnyType::UInt32;
  } else if (type_name == "int64") {
    return AnyType::Int64;
  } else if (type_name == "uint64") {
    return AnyType::UInt64;
  } else if (type_name == "float") {
    return AnyType::Float;
  } else if (type_name == "double") {
    return AnyType::Double;
  } else if (type_name == "string") {
    return AnyType::String;
  } else {
    return AnyType::Undefined;
  }
}

std::string GetAnyTypeName(AnyType type) {
  switch (type) {
  case AnyType::Int32:
    return "int32";
  case AnyType::UInt32:
    return "uint32";
  case AnyType::Int64:
    return "int64";
  case AnyType::UInt64:
    return "uint64";
  case AnyType::Float:
    return "float";
  case AnyType::Double:
    return "double";
  case AnyType::String:
    return "string";
  default:
    return "undefined";
  }
}

IdType ParseIdType(const std::string& type_name) {
  if (type_name == "int" || type_name == "int32" || type_name == "int32_t") {
    return IdType::Int32;
  } else if (type_name == "uint32" || type_name == "uint32_t") {
    return IdType::UInt32;
  } else if (type_name == "int64" || type_name == "int64_t") {
    return IdType::Int64;
  } else if (type_name == "uint64" || type_name == "uint64_t") {
    return IdType::UInt64;
  } else if (type_name == "string") {
    return IdType::String;
  } else {
    return IdType::Undefined;
  }
}

std::string GetIdTypeName(IdType type) {
  switch (type) {
  case IdType::Int32:
    return "int32";
  case IdType::UInt32:
    return "uint32";
  case IdType::Int64:
    return "int64";
  case IdType::UInt64:
    return "uint64";
  case IdType::String:
    return "string";
  default:
    return "undefined";
  }
}

std::ostream& operator<<(std::ostream& os, const AnyType& st) {
  os << GetAnyTypeName(st);
  return os;
}

std::istream& operator>>(std::istream& is, AnyType& st) {
  std::string name;
  is >> name;
  st = ParseAnyType(name);
  return is;
}

template <>
void print_json_value(std::stringstream& ss, AnyType const& value) {
  ss << std::quoted(GetAnyTypeName(value));
}

std::ostream& operator<<(std::ostream& os, const IdType& st) {
  os << GetIdTypeName(st);
  return os;
}

std::istream& operator>>(std::istream& is, IdType& st) {
  std::string name;
  is >> name;
  st = ParseIdType(name);
  return is;
}

template <>
void print_json_value(std::stringstream& ss, IdType const& value) {
  ss << "\"" << GetIdTypeName(value) << "\"";
}

}  // namespace vineyard
