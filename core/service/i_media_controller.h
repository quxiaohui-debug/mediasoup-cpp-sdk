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
#include <memory>
#include <unordered_map>

namespace rtc {
class Thread;
}

namespace webrtc {
class MediaStreamTrackInterface;
}

namespace mediasoupclient {
class Device;
class SendTransport;
class RecvTransport;
} // namespace mediasoupclient

namespace vi {

class IMediaEventHandler;

class IMediaController {
public:
  virtual ~IMediaController() = default;

  virtual void init() = 0;

  virtual void destroy() = 0;

  virtual void setMediasoupDevice(
      const std::shared_ptr<mediasoupclient::Device> &device) = 0;

  virtual void setSendTransport(
      const std::shared_ptr<mediasoupclient::SendTransport> &sendTransport) = 0;

  virtual void setRecvTransport(
      const std::shared_ptr<mediasoupclient::RecvTransport> &recvTransport) = 0;

  virtual void addObserver(std::shared_ptr<IMediaEventHandler> observer,
                           rtc::Thread *callbackThread) = 0;

  virtual void removeObserver(std::shared_ptr<IMediaEventHandler> observer) = 0;

  virtual void enableAudio(bool enabled) = 0;

  virtual void muteAudio(bool muted) = 0;

  virtual bool isAudioMuted() = 0;

  virtual void enableVideo(bool enabled, const std::string &trackName,
                           uint32_t width, uint32_t height) = 0;

  virtual void muteAudio(const std::string &pid, bool muted) = 0;

  virtual bool isAudioMuted(const std::string &pid) = 0;

  virtual void muteVideo(const std::string &pid, bool muted) = 0;

  virtual bool isVideoMuted(const std::string &pid) = 0;

  virtual std::unordered_map<
      std::string, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
  getRemoteAudioTracks(const std::string &pid) = 0;

  virtual std::unordered_map<
      std::string, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
  getRemoteVideoTracks(const std::string &pid) = 0;

  //帧输入接口
  virtual void inputFrame(const std::string &trackName, const uint8_t *data,
                          uint32_t len) = 0;
};

} // namespace vi
