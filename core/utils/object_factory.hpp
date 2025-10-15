/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "rtc_base/deprecated/recursive_critical_section.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace vi {
class IRcvProvider;

template <typename T> class ObjectFactory {
public:
  void _init() {
    rtc::CritScope scope(&_criticalSection);
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
      auto object = iter->second;
      if (object) {
        object->init();
      }
    }
  }

  void _destroy() {
    rtc::CritScope scope(&_criticalSection);
    for (auto iter = _objects.begin(); iter != _objects.end(); ++iter) {
      auto object = iter->second;
      if (object) {
        object->destroy();
      }
    }
    _objects.clear();
  }

  void _register(const std::string &key, const std::shared_ptr<T> &object) {
    rtc::CritScope scope(&_criticalSection);
    _objects[key] = object;
  }

  void _unregister(const std::string &key) {
    rtc::CritScope scope(&_criticalSection);
    auto it = _objects.find(key);
    if (it != _objects.end()) {
      _objects.erase(key);
    }
  }

  std::shared_ptr<T> _get(const std::string &key) {
    decltype(_objects) objects;
    {
      rtc::CritScope scope(&_criticalSection);
      objects = _objects;
    }

    auto it = objects.find(key);
    if (objects.end() != it) {
      return it->second;
    }

    return nullptr;
  }

protected:
  rtc::RecursiveCriticalSection _criticalSection;

  std::unordered_map<std::string, std::shared_ptr<T>> _objects;
};
} // namespace vi
