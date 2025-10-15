/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "service/options.h"
#include "utils/singleton.h"
#include <memory>
#include <unordered_map>
namespace vi {
class IRoomClient;
class RTCContext;

class Engine : public vi::Singleton<Engine> {
public:
  ~Engine();

  void init();

  void destroy();

  void setRTCLoggingSeverity(const std::string &level = "error");

  std::shared_ptr<RTCContext> getRTCContext();

  std::shared_ptr<IRoomClient> createRoomClient();

  const std::unordered_map<std::string, std::shared_ptr<IRoomClient>> &
  getRoomClients();

private:
  Engine();

  Engine(Engine &&) = delete;

  Engine(const Engine &) = delete;

  Engine &operator=(const Engine &) = delete;

  Engine &operator=(Engine &&) = delete;

private:
  friend class vi::Singleton<Engine>;

  std::shared_ptr<RTCContext> _rtcContext;

  std::unordered_map<std::string, std::shared_ptr<IRoomClient>> _roomClients;
};
} // namespace vi

#define getEngine() vi::Engine::sharedInstance()

#define getRoomClient(ID)                                                      \
  getEngine()->getRoomClients().find(ID) !=                                    \
          getEngine()->getRoomClients().end()                                  \
      ? getEngine()->getRoomClients().at(ID)                                   \
      : nullptr
