#pragma once

#include "absl/types/optional.h"
#include "serialization.hpp"
#include <memory>
#include <string>

#define FIELDS_MAP(...)                                                        \
  JSON_SERIALIZE(__VA_ARGS__)                                                  \
  MODEL_2_STRING()                                                             \
  STRING_2_MODEL()

#define FIELDS_MAP_NO_DSERIALIZE(...)                                          \
  JSON_NO_DSERIALIZE(__VA_ARGS__)                                              \
  MODEL_2_STRING()                                                             \
  STRING_2_MODEL()

#define MODEL_2_STRING()                                                       \
  virtual std::string toJsonStr() { return toJsonString(*this); }
#define STRING_2_MODEL()                                                       \
  virtual rapidjson::Document toJsonOject() { return toJson(*this); }

namespace vi {
class Jsonable {
public:
  virtual ~Jsonable(){};
};
} // namespace vi
/* jsonable_hpp */
