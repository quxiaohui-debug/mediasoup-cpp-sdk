/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#define MSC_CLASS "MediaStreamTrackFactory"

#include "rtc_context.hpp"
#include "MediaSoupClientErrors.hpp"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/media_stream_interface.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "dummy_video_encoder.h"
#include "engine.h"
#include "logger/spd_logger.h"
#include "modules/audio_device/include/audio_device.h"
#include "rtc_base/thread.h"
#include "system_wrappers/include/clock.h"
#include <iostream>

using namespace mediasoupclient;

namespace vi {

RTCContext::RTCContext() {}

RTCContext::~RTCContext() { DLOG("~RTCContext()"); }

void RTCContext::init() {
  if (_factory) {
    DLOG("already configured");
    return;
  }

  _networkThread = rtc::Thread::CreateWithSocketServer().release();
  _signalingThread = rtc::Thread::Create().release();
  _workerThread = rtc::Thread::Create().release();

  _networkThread->SetName("network_thread", nullptr);
  _signalingThread->SetName("signaling_thread", nullptr);
  _workerThread->SetName("worker_thread", nullptr);

  if (!_networkThread->Start() || !_signalingThread->Start() ||
      !_workerThread->Start()) {
    ELOG("thread start errored");
  }

  this->_factory = webrtc::CreatePeerConnectionFactory(
      this->_networkThread, this->_workerThread, this->_signalingThread,
      nullptr, webrtc::CreateBuiltinAudioEncoderFactory(),
      webrtc::CreateBuiltinAudioDecoderFactory(),
      std::make_unique<DummyVideoEncoderFactory>(),
      webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /*audio_mixer*/,
      nullptr /*audio_processing*/);
}

void RTCContext::destroy() {
  if (_factory) {
    _factory = nullptr;
  }
}

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
RTCContext::factory() {
  return _factory;
}

rtc::Thread *RTCContext::networkThread() { return _networkThread; }

rtc::Thread *RTCContext::signalingThread() { return _signalingThread; }

rtc::Thread *RTCContext::workerThread() { return _workerThread; }
} // namespace vi