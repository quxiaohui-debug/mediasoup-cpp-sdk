/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "utils/singleton.h"
#include "utils/thread_provider.h"
#include <map>
#include <memory>

namespace vi {
class ComponentFactory : public vi::Singleton<ComponentFactory> {
public:
  void init();

  void destroy();

  const std::unique_ptr<ThreadProvider> &getThreadProvider();

private:
  ComponentFactory();

  ComponentFactory(ComponentFactory &&) = delete;

  ComponentFactory(const ComponentFactory &) = delete;

  ComponentFactory &operator=(const ComponentFactory &) = delete;

  ComponentFactory &operator=(ComponentFactory &&) = delete;

private:
  friend class vi::Singleton<ComponentFactory>;

  std::unique_ptr<ThreadProvider> _threadProvider;
};
} // namespace vi

#define getComponentFactory() vi::ComponentFactory::sharedInstance()

#define getThread(T) getComponentFactory()->getThreadProvider()->thread(T)
