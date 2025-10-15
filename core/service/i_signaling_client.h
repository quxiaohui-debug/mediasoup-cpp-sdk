/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "i_signaling_event_handler.h"
#include <string>
#include <vector>

namespace vi {

using SuccessCallback = std::function<void(const std::string &json)>;
using FailureCallback =
    std::function<void(int32_t errorCode, const std::string &errorInfo)>;

class ISignalingClient {
public:
  virtual ~ISignalingClient() = default;

  virtual void init() = 0;

  virtual void destroy() = 0;

  virtual void
  addObserver(std::shared_ptr<ISignalingEventHandler> observer) = 0;

  virtual void
  removeObserver(std::shared_ptr<ISignalingEventHandler> observer) = 0;

  virtual void connect(const std::string &url,
                       const std::string &subprotocol) = 0;

  virtual void disconnect() = 0;

  virtual void send(const std::string &text, int64_t transcation,
                    SuccessCallback scb, FailureCallback fcb) = 0;

  virtual void send(const std::vector<uint8_t> &data, int64_t transcation,
                    SuccessCallback scb, FailureCallback fcb) = 0;
};

} // namespace vi
