/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "i_observer.hpp"
#include "rtc_base/deprecated/recursive_critical_section.h"
#include <functional>
#include <memory>

namespace vi {

template <class T, class N> class Observer : public IObserver {
public:
  typedef void (T::*Method)(const std::shared_ptr<N> &);

  Observer(const std::shared_ptr<T> &object, Method method,
           rtc::Thread *scheduleThread)
      : _object(object), _method(method), _scheduleThread(scheduleThread) {}

  Observer(const Observer &observer)
      : _object(observer._object), _method(observer._method),
        _scheduleThread(observer._scheduleThread) {}

  Observer &operator=(const Observer &observer) {
    if (this != &observer) {
      _object = observer._object;
      _method = observer._method;
      _scheduleThread = observer._scheduleThread;
    }
    return *this;
  }

  ~Observer() {}

  rtc::Thread *scheduleThread() override { return _scheduleThread; }

  void notify(const std::shared_ptr<INotification> &notification) override {
    rtc::CritScope scope(&_criticalSection);

    auto nf = std::dynamic_pointer_cast<N>(notification);
    if (!nf) {
      return;
    }

    if (auto object = _object.lock()) {
      if (_method) {
        (object.get()->*_method)(nf);
      }
    }
  }

  bool shouldAccept(const std::shared_ptr<INotification> &nf) override {
    return std::dynamic_pointer_cast<N>(nf) != nullptr;
  }

  bool equals(const IObserver &observer) const override {
    auto obs = dynamic_cast<const Observer *>(&observer);
    if (!obs) {
      return false;
    }
    if (_object.lock() && obs->_object.lock() &&
        obs->_object.lock() == _object.lock() && obs->_method == _method) {
      return true;
    }
    return false;
  }

  bool isValid() override {
    return _object.lock() != nullptr && _method != nullptr;
  }

  IObserver *clone() const override { return new Observer(*this); }

private:
  Observer();

private:
  rtc::RecursiveCriticalSection _criticalSection;
  std::weak_ptr<T> _object;
  Method _method;
  rtc::Thread *_scheduleThread;
};

} // namespace vi
