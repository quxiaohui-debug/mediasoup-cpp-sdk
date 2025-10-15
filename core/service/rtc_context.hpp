#pragma once

#include "api/scoped_refptr.h"

namespace rtc {
class Thread;
}

namespace webrtc {
class PeerConnectionFactoryInterface;
} // namespace webrtc

namespace vi {

class RTCContext {

public:
  RTCContext();

  ~RTCContext();

  void init();

  void destroy();

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory();

  rtc::Thread *networkThread();

  rtc::Thread *signalingThread();

  rtc::Thread *workerThread();

private:
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;

  /* MediaStreamTrack holds reference to the threads of the
   * PeerConnectionFactory. Use plain pointers in order to avoid threads being
   * destructed before tracks.
   */
  rtc::Thread *_networkThread;

  rtc::Thread *_signalingThread;

  rtc::Thread *_workerThread;
};

} // namespace vi