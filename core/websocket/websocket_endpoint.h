/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "connection_metadata.h"
#include <string>
#include <vector>
#include <websocketpp/client.hpp>
#include <websocketpp/common/memory.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>

namespace vi {

using Client = websocketpp::client<websocketpp::config::asio_client>;

class WebsocketEndpoint {
public:
  WebsocketEndpoint();

  ~WebsocketEndpoint();

  int connect(std::string const &uri,
              std::shared_ptr<IConnectionObserver> observer,
              const std::string &subprotocol = "");

  void close(int id, websocketpp::close::status::value code,
             const std::string &reason);

  void sendText(int id, const std::string &data);

  void sendBinary(int id, const std::vector<uint8_t> &data);

  void sendPing(int id, const std::string &data);

  void sendPong(int id, const std::string &data);

  ConnectionMetadata<Client>::ptr getMetadata(int id) const;

private:
  typedef std::map<int, ConnectionMetadata<Client>::ptr> ConnectionList;

  Client _endpoint;
  websocketpp::lib::shared_ptr<websocketpp::lib::thread> _thread;

  ConnectionList _connectionList;
  int _nextId;
};
} // namespace vi
