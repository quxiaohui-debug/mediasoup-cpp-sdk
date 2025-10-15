/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#pragma once

#include "Consumer.hpp"
#include "DataConsumer.hpp"
#include "DataProducer.hpp"
#include "Device.hpp"
#include "Producer.hpp"
#include "dummy_video_source.h"
#include "i_media_controller.h"
#include "i_media_event_handler.h"
#include "i_signaling_event_handler.h"
#include "options.h"
#include "signaling_models.h"
#include "utils/universal_observable.hpp"
#include <memory>

namespace rtc {
class Thread;
}

namespace webrtc {
class MediaStreamTrackInterface;
}

namespace mediasoupclient {
class SendTransport;
class RecvTransport;
} // namespace mediasoupclient

namespace vi {

class IMediasoupApi;
class WindowsCapturerTrackSource;
class MacTrackSource;

class MediaController : public IMediaController,
                        public ISignalingEventHandler,
                        public mediasoupclient::Producer::Listener,
                        public mediasoupclient::Consumer::Listener,
                        public mediasoupclient::DataProducer::Listener,
                        public mediasoupclient::DataConsumer::Listener,
                        public UniversalObservable<IMediaEventHandler>,
                        public std::enable_shared_from_this<MediaController> {
public:
  MediaController(
      std::shared_ptr<Options> options,
      rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf,
      std::shared_ptr<IMediasoupApi> mediasoupApi, rtc::Thread *mediasoupThread,
      rtc::Thread *signalingThread);

  ~MediaController();

  void init() override;

  void destroy() override;

  void setMediasoupDevice(
      const std::shared_ptr<mediasoupclient::Device> &device) override;

  void setSendTransport(const std::shared_ptr<mediasoupclient::SendTransport>
                            &sendTransport) override;

  void setRecvTransport(const std::shared_ptr<mediasoupclient::RecvTransport>
                            &recvTransport) override;

  void addObserver(std::shared_ptr<IMediaEventHandler> observer,
                   rtc::Thread *callbackThread) override;

  void removeObserver(std::shared_ptr<IMediaEventHandler> observer) override;

  void enableAudio(bool enabled) override;

  void muteAudio(bool muted) override;

  bool isAudioMuted() override;

  void enableVideo(bool enabled, const std::string &trackName, uint32_t width,
                   uint32_t height) override;

  void muteAudio(const std::string &pid, bool muted) override;

  bool isAudioMuted(const std::string &pid) override;

  void muteVideo(const std::string &pid, bool muted) override;

  bool isVideoMuted(const std::string &pid) override;

  std::unordered_map<std::string,
                     rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
  getRemoteAudioTracks(const std::string &pid) override;

  std::unordered_map<std::string,
                     rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
  getRemoteVideoTracks(const std::string &pid) override;

  void inputFrame(const std::string &trackName, const uint8_t *data,
                  uint32_t len) override;

protected:
  // Producer::Listener
  void OnTransportClose(mediasoupclient::Producer *producer) override;

  // Consumer::Listener
  void OnTransportClose(mediasoupclient::Consumer *consumer) override;

  // DataProducer::Listener
  void OnOpen(mediasoupclient::DataProducer *dataProducer) override;

  void OnClose(mediasoupclient::DataProducer *dataProducer) override;

  void OnBufferedAmountChange(mediasoupclient::DataProducer *dataProducer,
                              uint64_t sentDataSize) override;

  void OnTransportClose(mediasoupclient::DataProducer *dataProducer) override;

  // DataConsumer::Listener
  void OnConnecting(mediasoupclient::DataConsumer *dataConsumer) override;

  void OnOpen(mediasoupclient::DataConsumer *dataConsumer) override;

  void OnClosing(mediasoupclient::DataConsumer *dataConsumer) override;

  void OnClose(mediasoupclient::DataConsumer *dataConsumer) override;

  void OnMessage(mediasoupclient::DataConsumer *dataConsumer,
                 const webrtc::DataBuffer &buffer) override;

  void OnTransportClose(mediasoupclient::DataConsumer *dataConsumer) override;

protected:
  // ISignalingEventHandler
  void onOpened() override {}

  void onClosed() override {}

  // Request from SFU
  void onNewConsumer(
      std::shared_ptr<signaling::NewConsumerRequest> request) override;

  void onNewDataConsumer(
      std::shared_ptr<signaling::NewDataConsumerRequest> request) override;

  // Notification from SFU
  void onProducerScore(std::shared_ptr<signaling::ProducerScoreNotification>
                           notification) override;

  void onConsumerScore(std::shared_ptr<signaling::ConsumerScoreNotification>
                           notification) override;

  void onNewPeer(
      std::shared_ptr<signaling::NewPeerNotification> notification) override {}

  void onPeerClosed(std::shared_ptr<signaling::PeerClosedNotification>
                        notification) override {}

  void onPeerDisplayNameChanged(
      std::shared_ptr<signaling::PeerDisplayNameChangedNotification>
          notification) override {}

  void onConsumerPaused(std::shared_ptr<signaling::ConsumerPausedNotification>
                            notification) override;

  void onConsumerResumed(std::shared_ptr<signaling::ConsumerResumedNotification>
                             notification) override;

  void onConsumerClosed(std::shared_ptr<signaling::ConsumerClosedNotification>
                            notification) override;

  void onConsumerLayersChanged(
      std::shared_ptr<signaling::ConsumerLayersChangedNotification>
          notification) override;

  void onDataConsumerClosed(
      std::shared_ptr<signaling::DataConsumerClosedNotification> notification)
      override;

  void onDownlinkBwe(std::shared_ptr<signaling::DownlinkBweNotification>
                         notification) override;

  void onActiveSpeaker(std::shared_ptr<signaling::ActiveSpeakerNotification>
                           notification) override {}

private:
  void configVideoEncodings();

  void
  createNewConsumer(std::shared_ptr<signaling::NewConsumerRequest> request);

  void createNewDataConsumer(
      std::shared_ptr<signaling::NewDataConsumerRequest> request);

  void updateConsumer(const std::string &tid, bool paused);

private:
  std::shared_ptr<Options> _options;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      _peerConnectionFactory;
  std::shared_ptr<IMediasoupApi> _mediasoupApi;
  rtc::Thread *_mediasoupThread;
  rtc::Thread *_signalingThread;
  std::shared_ptr<mediasoupclient::Device> _mediasoupDevice;
  std::shared_ptr<mediasoupclient::SendTransport> _sendTransport;
  std::shared_ptr<mediasoupclient::RecvTransport> _recvTransport;

  std::vector<webrtc::RtpEncodingParameters> _encodings;

  std::shared_ptr<mediasoupclient::Producer> _micProducer;

  // key: trackName
  std::unordered_map<std::string, std::shared_ptr<mediasoupclient::Producer>>
      _producerMap;

  // key: trackName
  std::unordered_map<std::string, rtc::scoped_refptr<DummyVideoSource>>
      _sourceMap;

  // key: consumerId
  std::unordered_map<std::string, std::shared_ptr<mediasoupclient::Consumer>>
      _consumerMap;

  // key: dataConsumerId
  std::unordered_map<std::string,
                     std::shared_ptr<mediasoupclient::DataConsumer>>
      _dataConsumerMap;

  // key: consumerId, value: peerId
  std::unordered_map<std::string, std::string> _consumerIdToPeerId;
};

} // namespace vi
