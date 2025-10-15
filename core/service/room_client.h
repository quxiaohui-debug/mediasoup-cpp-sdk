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
#include "Transport.hpp"
#include "i_media_event_handler.h"
#include "i_room_client.h"
#include "i_signaling_event_handler.h"
#include "utils/container.hpp"
#include "utils/universal_observable.hpp"
#include <memory>
#include <unordered_map>

namespace rtc {
class Thread;
}

namespace webrtc {
class MediaStreamTrackInterface;
}

namespace vi {

class ISignalingClient;
class IMediasoupApi;
class MacTrackSource;
class IParticipant;
class ParticipantController;
class IMediaController;
class RTCContext;

class RoomClient : public IRoomClient,
                   public ISignalingEventHandler,
                   public mediasoupclient::SendTransport::Listener,
                   public mediasoupclient::RecvTransport::Listener,
                   public IMediaEventHandler,
                   public UniversalObservable<IRoomClientEventHandler>,
                   public std::enable_shared_from_this<RoomClient> {
public:
  RoomClient(std::shared_ptr<RTCContext> rtcContext,
             rtc::Thread *mediasoupThread, rtc::Thread *transportThread);

  ~RoomClient();

  void init() override;

  void destroy() override;

  std::string getId() override { return _id; }

  std::string getRoomId() override { return _roomId; }

  void addObserver(std::shared_ptr<IRoomClientEventHandler> observer,
                   rtc::Thread *callbackThread) override;

  void
  removeObserver(std::shared_ptr<IRoomClientEventHandler> observer) override;

  void join(const std::string &host, uint16_t port, const std::string &roomId,
            const std::string &displayName,
            std::shared_ptr<Options> options) override;

  void leave() override;

  RoomState getRoomState() override;

  void enableAudio(bool enabled) override;

  void muteAudio(bool muted) override;

  bool isAudioMuted() override;

  void enableVideo(bool enabled, const std::string &trackName, uint32_t width,
                   uint32_t height) override;

  int32_t speakingVolume() override;

  std::shared_ptr<IParticipantController> getParticipantController() override;

  std::shared_ptr<IMediaController> getMediaController() override;

protected:
  // ISignalingEventHandler
  void onOpened() override;

  void onClosed() override;

  // Request from SFU
  void onNewConsumer(
      std::shared_ptr<signaling::NewConsumerRequest> request) override {}

  void onNewDataConsumer(
      std::shared_ptr<signaling::NewDataConsumerRequest> request) override {}

  // Notification from SFU
  void onProducerScore(std::shared_ptr<signaling::ProducerScoreNotification>
                           notification) override {}

  void onConsumerScore(std::shared_ptr<signaling::ConsumerScoreNotification>
                           notification) override {}

  void onNewPeer(
      std::shared_ptr<signaling::NewPeerNotification> notification) override {}

  void onPeerClosed(std::shared_ptr<signaling::PeerClosedNotification>
                        notification) override {}

  void onPeerDisplayNameChanged(
      std::shared_ptr<signaling::PeerDisplayNameChangedNotification>
          notification) override {}

  void onConsumerPaused(std::shared_ptr<signaling::ConsumerPausedNotification>
                            notification) override {}

  void onConsumerResumed(std::shared_ptr<signaling::ConsumerResumedNotification>
                             notification) override {}

  void onConsumerClosed(std::shared_ptr<signaling::ConsumerClosedNotification>
                            notification) override {}

  void onConsumerLayersChanged(
      std::shared_ptr<signaling::ConsumerLayersChangedNotification>
          notification) override {}

  void onDataConsumerClosed(
      std::shared_ptr<signaling::DataConsumerClosedNotification> notification)
      override {}

  void onDownlinkBwe(std::shared_ptr<signaling::DownlinkBweNotification>
                         notification) override {}

  void onActiveSpeaker(std::shared_ptr<signaling::ActiveSpeakerNotification>
                           notification) override;

protected:
  // Transport::Listener
  std::future<void> OnConnect(mediasoupclient::Transport *transport,
                              const nlohmann::json &dtlsParameters) override;

  void OnConnectionStateChange(mediasoupclient::Transport *transport,
                               const std::string &connectionState) override;

  std::future<std::string> OnProduce(mediasoupclient::SendTransport *transport,
                                     const std::string &kind,
                                     nlohmann::json rtpParameters,
                                     const nlohmann::json &appData) override;

  std::future<std::string>
  OnProduceData(mediasoupclient::SendTransport *transport,
                const nlohmann::json &sctpStreamParameters,
                const std::string &label, const std::string &protocol,
                const nlohmann::json &appData) override;

protected:
  // IMediaEventHandler
  void onRemoteAudioStateChanged(const std::string &pid, bool muted) override {}

  void onRemoteVideoStateChanged(const std::string &pid, bool muted) override {}

  void onCreateRemoteAudioTrack(
      const std::string &pid, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>) override {}

  void onRemoveRemoteAudioTrack(
      const std::string &pid, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>) override {}

  void onCreateRemoteVideoTrack(
      const std::string &pid, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
      const nlohmann::json &appData) override {}

  void onRemoveRemoteVideoTrack(
      const std::string &pid, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>) override {}

private:
  void getRouterRtpCapabilities();

  void joinImpl();

  void onLoadMediasoupDevice(
      std::shared_ptr<signaling::GetRouterRtpCapabilitiesResponse> response);

  void createSendTransport();

  void createRecvTransport();

  void requestCreateTransport(bool forceTcp, bool producing, bool consuming);

  void onCreateSendTransport(
      std::shared_ptr<signaling::CreateWebRtcTransportResponse> transportInfo);

  void onCreateRecvTransport(
      std::shared_ptr<signaling::CreateWebRtcTransportResponse> transportInfo);

  void createTransportImpl(
      bool producing, bool consuming,
      std::shared_ptr<signaling::CreateWebRtcTransportResponse> transportInfo);

  void configure();

  void _onConnect(mediasoupclient::Transport *transport,
                  const nlohmann::json &dtlsParameters);

  std::string _onProduce(mediasoupclient::SendTransport *transport,
                         const std::string &kind, nlohmann::json rtpParameters,
                         const nlohmann::json &appData);

  std::string _onProduceData(mediasoupclient::SendTransport *transport,
                             const nlohmann::json &sctpStreamParameters,
                             const std::string &label,
                             const std::string &protocol,
                             const nlohmann::json &appData);

  void onRoomStateChanged(vi::RoomState state);

  void createComponents();

  void destroyComponents();

private:
  std::string _id = std::to_string(rtc::CreateRandomId());

  std::shared_ptr<RTCContext> _rtcContext;

  rtc::Thread *_mediasoupThread;
  rtc::Thread *_transportThread;

  std::string _hostname;
  uint16_t _port;
  std::string _roomId;

  std::string _peerId;
  std::string _displayName;

  std::shared_ptr<Options> _options;

  std::shared_ptr<ISignalingClient> _signalingClient;
  std::shared_ptr<IMediasoupApi> _mediasoupApi;

  std::shared_ptr<mediasoupclient::Device> _mediasoupDevice;

  std::shared_ptr<mediasoupclient::SendTransport> _sendTransport;
  std::shared_ptr<mediasoupclient::RecvTransport> _recvTransport;

  std::shared_ptr<mediasoupclient::PeerConnection::Options>
      _peerConnectionOptions;

  std::shared_ptr<IMediaController> _mediaController;

  std::shared_ptr<ProxyImpl<IParticipantController, ParticipantController>>
      _participantController;

  RoomState _state = RoomState::CLOSED;

  int32_t _volume = 0;
};

} // namespace vi
