/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "api/scoped_refptr.h"
#include <string>

namespace webrtc {
class MediaStreamTrackInterface;
}

namespace vi {

enum class RoomState { UNKNOWN = 0, CONNECTING, CONNECTED, CLOSED };

class IRoomClientEventHandler {
public:
  virtual ~IRoomClientEventHandler() = default;

  virtual void onRoomStateChanged(RoomState state) = 0;

  virtual void onLocalActiveSpeaker(int32_t volume) = 0;
};

} // namespace vi
