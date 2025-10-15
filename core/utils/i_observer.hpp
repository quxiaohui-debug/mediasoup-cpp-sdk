/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "absl/types/optional.h"
#include "rtc_base/thread.h"
#include <memory>
#include <string>

namespace vi {

class INotification;

class IObserver {
public:
  IObserver() {}

  IObserver(const IObserver &observer) {}

  virtual ~IObserver() = default;

  virtual rtc::Thread *scheduleThread() = 0;

  virtual void notify(const std::shared_ptr<INotification> &nf) = 0;

  virtual bool shouldAccept(const std::shared_ptr<INotification> &nf) = 0;

  virtual bool equals(const IObserver &observer) const = 0;

  virtual bool isValid() = 0;

  virtual IObserver *clone() const = 0;
};

} // namespace vi
