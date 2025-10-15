/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#include "core.h"

#include "logger/spd_logger.h"
#include "mediasoupclient.hpp"
#include "service/component_factory.h"
#include "service/engine.h"

namespace vi {

Core::Core() {}

void Core::init(const std::string &log_path) {
  vi::Logger::init(log_path);

  mediasoupclient::Initialize();

  getComponentFactory()->init();

  getEngine()->init();

  DLOG("mediasoupclient version: {}", mediasoupclient::Version().c_str());
}

void Core::destroy() {
  mediasoupclient::Cleanup();

  getEngine()->destroy();

  getComponentFactory()->destroy();

  vi::Logger::destroy();
}
} // namespace vi
