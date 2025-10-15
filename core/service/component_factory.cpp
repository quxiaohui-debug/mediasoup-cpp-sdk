/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#include "component_factory.h"
#include "utils/thread_provider.h"

namespace vi {

ComponentFactory::ComponentFactory() {}

void ComponentFactory::init() {
  if (!_threadProvider) {
    _threadProvider = std::make_unique<ThreadProvider>();
    _threadProvider->init();
#ifdef WIN32
    _threadProvider->create({"transport", "mediasoup", "communication"});
#else
    _threadProvider->create(
        {"transport", "mediasoup", "communication", "main"});
#endif
  }
}

void ComponentFactory::destroy() {
  if (_threadProvider) {
    _threadProvider->destroy();
    _threadProvider = nullptr;
  }
}

const std::unique_ptr<ThreadProvider> &ComponentFactory::getThreadProvider() {
  return _threadProvider;
}
} // namespace vi
