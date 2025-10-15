/************************************************************************
 * @Copyright: 2021-2024
 * @FileName:
 * @Description: Open source mediasoup room client library
 * @Version: 1.0.0
 * @Author: Jackie Ou
 * @CreateTime: 2021-10-1
 *************************************************************************/

#include "media_controller.h"
#include "Transport.hpp"
#include "api/peer_connection_interface.h"
#include "api/rtp_parameters.h"
#include "logger/spd_logger.h"
#include "mediasoup_api.h"
#include "modules/audio_device/include/audio_device.h"
#include "rtc_base/thread.h"
#include "rtc_context.hpp"
#include "service/engine.h"
#include <future>

namespace vi {

MediaController::MediaController(
    std::shared_ptr<Options> options,
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf,
    std::shared_ptr<IMediasoupApi> mediasoupApi, rtc::Thread *mediasoupThread,
    rtc::Thread *signalingThread)
    : _options(options), _peerConnectionFactory(pcf),
      _mediasoupApi(mediasoupApi), _mediasoupThread(mediasoupThread),
      _signalingThread(signalingThread) {}

MediaController::~MediaController() { DLOG("~MediaController()"); }

void MediaController::init() { configVideoEncodings(); }

void MediaController::destroy() {
  if (_micProducer) {
    _micProducer->Close();
    _micProducer = nullptr;
  }
  for (auto &ref : _producerMap) {
    ref.second->Close();
    ref.second = nullptr;
  }

  if (!_dataConsumerMap.empty()) {
    for (auto &dataConsumer : _dataConsumerMap) {
      dataConsumer.second->Close();
    }
    _dataConsumerMap.clear();
  }

  if (!_consumerMap.empty()) {
    for (auto &consumer : _consumerMap) {
      consumer.second->Close();
    }
    _consumerMap.clear();
  }

  for (auto &ref : _sourceMap) {
    ref.second = nullptr;
  }
}

void MediaController::setMediasoupDevice(
    const std::shared_ptr<mediasoupclient::Device> &device) {
  _mediasoupDevice = device;
}

void MediaController::setSendTransport(
    const std::shared_ptr<mediasoupclient::SendTransport> &sendTransport) {
  _sendTransport = sendTransport;
}

void MediaController::setRecvTransport(
    const std::shared_ptr<mediasoupclient::RecvTransport> &recvTransport) {
  _recvTransport = recvTransport;
}

void MediaController::addObserver(std::shared_ptr<IMediaEventHandler> observer,
                                  rtc::Thread *callbackThread) {
  UniversalObservable<IMediaEventHandler>::addWeakObserver(observer,
                                                           callbackThread);
}

void MediaController::removeObserver(
    std::shared_ptr<IMediaEventHandler> observer) {
  UniversalObservable<IMediaEventHandler>::removeObserver(observer);
}

void MediaController::configVideoEncodings() {
  _encodings.clear();

  webrtc::RtpEncodingParameters ph;
  ph.rid = "h";
  ph.active = true;
  ph.max_bitrate_bps = 5000000;
  ph.scale_resolution_down_by = 1;

  webrtc::RtpEncodingParameters pm;
  pm.rid = "m";
  pm.active = true;
  pm.max_bitrate_bps = 1000000;
  pm.scale_resolution_down_by = 2;

  webrtc::RtpEncodingParameters pl;
  pl.rid = "m";
  pl.active = true;
  pl.max_bitrate_bps = 500000;
  pl.scale_resolution_down_by = 4;

  _encodings.emplace_back(ph);
  _encodings.emplace_back(pm);
  _encodings.emplace_back(pl);
}

void MediaController::enableAudio(bool enabled) {
  if (!_mediasoupDevice) {
    DLOG("_mediasoupDevice is null");
    return;
  }

  if (!_mediasoupDevice->CanProduce("audio")) {
    DLOG("cannot produce audio");
    return;
  }

  if (!_sendTransport) {
    DLOG("_sendTransport is null");
    return;
  }

  if (enabled) {
    if (_micProducer) {
      DLOG("already has a mic producer");
      return;
    }

    cricket::AudioOptions options;
    options.echo_cancellation = false;
    DLOG("audio options: {}", options.ToString());
    rtc::scoped_refptr<webrtc::AudioTrackInterface> track =
        _peerConnectionFactory->CreateAudioTrack(
            "mic-track",
            _peerConnectionFactory->CreateAudioSource(options).get());

    nlohmann::json codecOptions = nlohmann::json::object();
    codecOptions["opusStereo"] = true;
    codecOptions["opusDtx"] = true;

    mediasoupclient::Producer *producer =
        _sendTransport->Produce(this, track, nullptr, &codecOptions, nullptr);
    _micProducer.reset(producer);
  } else {
    if (!_mediasoupApi) {
      DLOG("_mediasoupApi is null");
      return;
    }

    if (!_micProducer) {
      DLOG("_micProducer is null");
      return;
    }

    _mediasoupApi->closeProducer(
        _micProducer->GetId(),
        [wself = weak_from_this()](
            int32_t errorCode, const std::string &errorInfo,
            std::shared_ptr<signaling::BasicResponse> response) {
          auto self = wself.lock();
          if (!self) {
            DLOG("RoomClient is null");
            return;
          }
          if (errorCode != 0) {
            DLOG("closeProducer failed, error code: {}, error info: {}",
                 errorCode, errorInfo);
            return;
          }
          if (!response || !response->ok) {
            DLOG("response is null or response->ok == false");
            return;
          }
        });
    _micProducer->Close();
    _micProducer = nullptr;
  }
}

void MediaController::muteAudio(bool muted) {
  if (!_mediasoupApi) {
    DLOG("_mediasoupApi is null");
    return;
  }

  if (!_micProducer) {
    DLOG("_micProducer is null");
    return;
  }

  if (muted) {
    _micProducer->Pause();

    _mediasoupApi->pauseProducer(
        _micProducer->GetId(),
        [wself = weak_from_this()](
            int32_t errorCode, const std::string &errorInfo,
            std::shared_ptr<signaling::BasicResponse> response) {
          auto self = wself.lock();
          if (!self) {
            DLOG("RoomClient is null");
            return;
          }
          if (errorCode != 0) {
            DLOG("pauseProducer failed, error code: {}, error info: {}",
                 errorCode, errorInfo);
            return;
          }
          if (!response || !response->ok) {
            DLOG("response is null or response->ok == false");
            return;
          }
        });

  } else {
    _micProducer->Resume();

    _mediasoupApi->resumeProducer(
        _micProducer->GetId(),
        [wself = weak_from_this()](
            int32_t errorCode, const std::string &errorInfo,
            std::shared_ptr<signaling::BasicResponse> response) {
          auto self = wself.lock();
          if (!self) {
            DLOG("RoomClient is null");
            return;
          }
          if (errorCode != 0) {
            DLOG("resumeProducer failed, error code: {}, error info: {}",
                 errorCode, errorInfo);
            return;
          }
          if (!response || !response->ok) {
            DLOG("response is null or response->ok == false");
            return;
          }
        });
  }
}

bool MediaController::isAudioMuted() {
  if (!_micProducer) {
    DLOG("_micProducer is null");
    return true;
  }
  return _micProducer->IsPaused();
}

void MediaController::enableVideo(bool enabled, const std::string &trackName,
                                  uint32_t width, uint32_t height) {
  if (!_mediasoupDevice) {
    DLOG("_mediasoupDevice is null");
    return;
  }

  if (!_mediasoupDevice->CanProduce("video")) {
    DLOG("cannot produce video");
    return;
  }

  if (!_sendTransport) {
    DLOG("_sendTransport is null");
    return;
  }

  auto &videoProducer = _producerMap[trackName];
  auto &videoSource = _sourceMap[trackName];

  if (enabled) {
    if (videoProducer) {
      DLOG("already has a video producer, track name: {}", trackName);
      return;
    }

    if (videoSource) {
      DLOG("already has a video source, track name: {}", trackName);
      return;
    }

    // 自定义视频源
    videoSource = rtc::scoped_refptr<DummyVideoSource>(
        new rtc::RefCountedObject<DummyVideoSource>(width, height));

    if (videoSource) {
      DLOG("create video source, track name: {}", trackName);
      rtc::scoped_refptr<webrtc::VideoTrackInterface> track =
          _peerConnectionFactory->CreateVideoTrack(trackName,
                                                   videoSource.get());
      track->set_enabled(true);
      nlohmann::json codecOptions = nlohmann::json::object();
      codecOptions["videoGoogleStartBitrate"] = 20 * 1024; // 20Mbps
      codecOptions["videoGoogleMaxBitrate"] = 20 * 1024;   // 20Mbps
      codecOptions["videoGoogleMinBitrate"] = 0;           // 0Mbps

      nlohmann::json appData;
      signaling::SharingAppData sharingAppData;
      sharingAppData.sharing.trackName = trackName;
      appData = sharingAppData;

      mediasoupclient::Producer *producer = _sendTransport->Produce(
          this, track,
          _options->useSimulcast.value_or(false) ? &_encodings : nullptr,
          &codecOptions, nullptr, appData);
      videoProducer.reset(producer);
    }
  } else {
    if (videoSource) {
      videoSource = nullptr;
    }

    if (!_mediasoupApi) {
      DLOG("_mediasoupApi is null");
      return;
    }

    if (!videoProducer) {
      DLOG("video procuder is null");
      return;
    }

    _mediasoupApi->closeProducer(
        videoProducer->GetId(),
        [wself = weak_from_this()](
            int32_t errorCode, const std::string &errorInfo,
            std::shared_ptr<signaling::BasicResponse> response) {
          auto self = wself.lock();
          if (!self) {
            DLOG("RoomClient is null");
            return;
          }
          if (errorCode != 0) {
            DLOG("closeProducer failed, error code: {}, error info: {}",
                 errorCode, errorInfo);
            return;
          }
          if (!response || !response->ok) {
            DLOG("response is null or response->ok == false");
            return;
          }
        });

    videoProducer->Close();
    videoProducer = nullptr;
  }
}

void MediaController::muteVideo(const std::string &pid, bool muted) {
  if (!_mediasoupApi) {
    DLOG("_mediasoupApi is null");
    return;
  }

  for (const auto &pair : _consumerIdToPeerId) {
    if (pair.second == pid) {
      auto tid = pair.first;
      if (_consumerMap.find(tid) == _consumerMap.end()) {
        return;
      }

      auto consumer = _consumerMap[tid];

      if (consumer->GetKind() == "video") {
        if (muted) {
          consumer->Pause();

          _mediasoupApi->pauseConsumer(
              consumer->GetId(),
              [wself = weak_from_this()](
                  int32_t errorCode, const std::string &errorInfo,
                  std::shared_ptr<signaling::BasicResponse> response) {
                auto self = wself.lock();
                if (!self) {
                  DLOG("RoomClient is null");
                  return;
                }
                if (errorCode != 0) {
                  DLOG("pauseConsumer failed, error code: {}, error info: {}",
                       errorCode, errorInfo);
                  return;
                }
                if (!response || !response->ok) {
                  DLOG("response is null or response->ok == false");
                  return;
                }
              });
        } else {
          consumer->Resume();

          _mediasoupApi->resumeConsumer(
              consumer->GetId(),
              [wself = weak_from_this()](
                  int32_t errorCode, const std::string &errorInfo,
                  std::shared_ptr<signaling::BasicResponse> response) {
                auto self = wself.lock();
                if (!self) {
                  DLOG("RoomClient is null");
                  return;
                }
                if (errorCode != 0) {
                  DLOG("resumeConsumer failed, error code: {}, error info: {}",
                       errorCode, errorInfo);
                  return;
                }
                if (!response || !response->ok) {
                  DLOG("response is null or response->ok == false");
                  return;
                }
              });
        }
      }
    }
  }
}

bool MediaController::isVideoMuted(const std::string &pid) {
  for (const auto &pair : _consumerIdToPeerId) {
    if (pair.second == pid) {
      auto tid = pair.first;
      auto consumer = _consumerMap[tid];
      if (consumer->GetKind() == "video" && !consumer->IsPaused()) {
        return false;
      }
    }
  }
  return true;
}

std::unordered_map<std::string,
                   rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
MediaController::getRemoteAudioTracks(const std::string &pid) {
  std::unordered_map<std::string,
                     rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
      tracks;
  for (const auto &pair : _consumerMap) {
    if (pair.second->GetKind() == "audio") {
      tracks[pair.first] = pair.second->GetTrack();
    }
  }
  return tracks;
}

std::unordered_map<std::string,
                   rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
MediaController::getRemoteVideoTracks(const std::string &pid) {
  std::unordered_map<std::string,
                     rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>
      tracks;
  for (const auto &pair : _consumerMap) {
    auto peerId = _consumerIdToPeerId[pair.first];
    if (pair.second->GetKind() == "video" && peerId == pid) {
      tracks[pair.first] = pair.second->GetTrack();
    }
  }
  return tracks;
}

void MediaController::inputFrame(const std::string &trackName,
                                 const uint8_t *data, uint32_t len) {
  auto &ref = _sourceMap[trackName];
  if (ref) {
    ref->InputFrame(data, len);
  }
}

void MediaController::muteAudio(const std::string &pid, bool muted) {
  if (!_mediasoupApi) {
    DLOG("_mediasoupApi is null");
    return;
  }

  for (const auto &pair : _consumerIdToPeerId) {
    if (pair.second == pid) {
      auto tid = pair.first;
      if (_consumerMap.find(tid) == _consumerMap.end()) {
        return;
      }

      auto consumer = _consumerMap[tid];

      if (consumer->GetKind() == "audio") {
        if (muted) {
          consumer->Pause();

          _mediasoupApi->pauseConsumer(
              consumer->GetId(),
              [wself = weak_from_this()](
                  int32_t errorCode, const std::string &errorInfo,
                  std::shared_ptr<signaling::BasicResponse> response) {
                auto self = wself.lock();
                if (!self) {
                  DLOG("RoomClient is null");
                  return;
                }
                if (errorCode != 0) {
                  DLOG("pauseConsumer failed, error code: {}, error info: {}",
                       errorCode, errorInfo);
                  return;
                }
                if (!response || !response->ok) {
                  DLOG("response is null or response->ok == false");
                  return;
                }
              });
        } else {
          consumer->Resume();

          _mediasoupApi->resumeConsumer(
              consumer->GetId(),
              [wself = weak_from_this()](
                  int32_t errorCode, const std::string &errorInfo,
                  std::shared_ptr<signaling::BasicResponse> response) {
                auto self = wself.lock();
                if (!self) {
                  DLOG("RoomClient is null");
                  return;
                }
                if (errorCode != 0) {
                  DLOG("resumeConsumer failed, error code: {}, error info: {}",
                       errorCode, errorInfo);
                  return;
                }
                if (!response || !response->ok) {
                  DLOG("response is null or response->ok == false");
                  return;
                }
              });
        }
      }
    }
  }
}

bool MediaController::isAudioMuted(const std::string &pid) {
  for (const auto &pair : _consumerIdToPeerId) {
    if (pair.second == pid) {
      auto tid = pair.first;
      auto consumer = _consumerMap[tid];
      if (consumer->GetKind() == "audio" && !consumer->IsPaused()) {
        return false;
      }
    }
  }
  return true;
}

void MediaController::updateConsumer(const std::string &tid, bool paused) {
  for (const auto &consumer : _consumerMap) {
    if (consumer.second->GetId() == tid) {
      auto peerId = _consumerIdToPeerId[consumer.second->GetId()];
      if (paused) {
        consumer.second->Pause();
      } else {
        consumer.second->Resume();
      }
      auto paused = consumer.second->IsPaused();
      if (consumer.second->GetKind() == "audio") {
        UniversalObservable<IMediaEventHandler>::notifyObservers(
            [wself = weak_from_this(), peerId, paused](const auto &observer) {
              auto self = wself.lock();
              if (!self) {
                DLOG("RoomClient is null");
                return;
              }

              observer->onRemoteAudioStateChanged(peerId, paused);
            });
      } else if (consumer.second->GetKind() == "video") {
        UniversalObservable<IMediaEventHandler>::notifyObservers(
            [wself = weak_from_this(), peerId, paused](const auto &observer) {
              auto self = wself.lock();
              if (!self) {
                DLOG("RoomClient is null");
                return;
              }

              observer->onRemoteVideoStateChanged(peerId, paused);
            });
      }
    }
  }
}

void MediaController::createNewConsumer(
    std::shared_ptr<signaling::NewConsumerRequest> request) {
  if (!request) {
    DLOG("request is null");
    return;
  }

  if (!_options->consume.value_or(false)) {
    DLOG("consuming is disabled");
    return;
  }

  if (!_recvTransport) {
    DLOG("_recvTransport is null");
    return;
  }

  nlohmann::json rtpParameters =
      nlohmann::json::parse(request->data->rtpParameters->toJsonStr());
  nlohmann::json appData =
      nlohmann::json::parse(request->data->appData->toJsonStr());
  mediasoupclient::Consumer *consumer = _recvTransport->Consume(
      this, request->data->id.value(), request->data->producerId.value(),
      request->data->kind.value(), &rtpParameters, appData);

  std::shared_ptr<mediasoupclient::Consumer> ptr;
  ptr.reset(consumer);
  _consumerMap[request->data->id.value()] = ptr;
  _consumerIdToPeerId[request->data->id.value()] =
      request->data->peerId.value_or("");
  bool producerPaused = request->data->producerPaused.value();

  UniversalObservable<IMediaEventHandler>::notifyObservers(
      [wself = weak_from_this(), ptr, request, producerPaused,
       appData](const auto &observer) {
        auto self = wself.lock();
        if (!self) {
          DLOG("RoomClient is null");
          return;
        }
        if (ptr->GetKind() == "audio") {
          observer->onCreateRemoteAudioTrack(request->data->peerId.value(),
                                             ptr->GetId(), ptr->GetTrack());
          observer->onRemoteAudioStateChanged(request->data->peerId.value(),
                                              producerPaused);
        } else if (ptr->GetKind() == "video") {
          observer->onCreateRemoteVideoTrack(request->data->peerId.value(),
                                             ptr->GetId(), ptr->GetTrack(),
                                             appData);
          observer->onRemoteVideoStateChanged(request->data->peerId.value(),
                                              producerPaused);
        }
      });

  if (!_mediasoupApi) {
    DLOG("_mediasoupApi is null");
    return;
  }

  auto response = std::make_shared<signaling::BasicResponse>();
  response->response = true;
  response->id = request->id;
  response->ok = true;
  _mediasoupApi->response(response);

  _mediasoupApi->setConsumerPreferredLayers(request->data->id.value(), 1, 1,
                                            nullptr);
}

void MediaController::createNewDataConsumer(
    std::shared_ptr<signaling::NewDataConsumerRequest> request) {
  if (!request) {
    DLOG("request is null");
    return;
  }

  if (!_options->consume.value_or(false)) {
    DLOG("consuming is disabled");
    return;
  }

  if (!_options->datachannel.value_or(false)) {
    DLOG("datachannel is disabled");
    return;
  }

  if (!_recvTransport) {
    DLOG("_recvTransport is null");
    return;
  }

  nlohmann::json appData =
      nlohmann::json::parse(request->data->appData->toJsonStr());
  mediasoupclient::DataConsumer *consumer = _recvTransport->ConsumeData(
      this, request->data->id.value(), request->data->dataProducerId.value(),
      request->data->sctpStreamParameters->streamId.value(),
      request->data->label.value(), request->data->protocol.value(), appData);
  std::shared_ptr<mediasoupclient::DataConsumer> ptr;
  ptr.reset(consumer);
  _dataConsumerMap[request->data->id.value()] = ptr;
  _consumerIdToPeerId[request->data->id.value()] =
      request->data->peerId.value_or("");

  auto id1 = request->data->id.value();
  auto id2 = request->data->dataProducerId.value();

  if (!_mediasoupApi) {
    DLOG("_mediasoupApi is null");
    return;
  }

  auto response = std::make_shared<signaling::BasicResponse>();
  response->response = true;
  response->id = request->id;
  response->ok = true;
  _mediasoupApi->response(response);
}

void MediaController::OnTransportClose(mediasoupclient::Producer *producer) {}

void MediaController::OnTransportClose(mediasoupclient::Consumer *consumer) {}

void MediaController::OnOpen(mediasoupclient::DataProducer *dataProducer) {}

void MediaController::OnClose(mediasoupclient::DataProducer *dataProducer) {}

void MediaController::OnBufferedAmountChange(
    mediasoupclient::DataProducer *dataProducer, uint64_t sentDataSize) {}

void MediaController::OnTransportClose(
    mediasoupclient::DataProducer *dataProducer) {}

void MediaController::OnConnecting(
    mediasoupclient::DataConsumer *dataConsumer) {}

void MediaController::OnOpen(mediasoupclient::DataConsumer *dataConsumer) {}

void MediaController::OnClosing(mediasoupclient::DataConsumer *dataConsumer) {}

void MediaController::OnClose(mediasoupclient::DataConsumer *dataConsumer) {}

void MediaController::OnMessage(mediasoupclient::DataConsumer *dataConsumer,
                                const webrtc::DataBuffer &buffer) {
  std::string msg = std::string(buffer.data.data<char>(), buffer.data.size());
  DLOG("OnMessage: {}", msg);
}

void MediaController::OnTransportClose(
    mediasoupclient::DataConsumer *dataConsumer) {}

// Request from SFU
void MediaController::onNewConsumer(
    std::shared_ptr<signaling::NewConsumerRequest> request) {
  _mediasoupThread->PostTask(RTC_FROM_HERE,
                             [wself = weak_from_this(), request]() {
                               auto self = wself.lock();
                               if (!self) {
                                 DLOG("RoomClient is null");
                                 return;
                               }
                               self->createNewConsumer(request);
                             });
}

void MediaController::onNewDataConsumer(
    std::shared_ptr<signaling::NewDataConsumerRequest> request) {
  _mediasoupThread->PostTask(RTC_FROM_HERE,
                             [wself = weak_from_this(), request]() {
                               auto self = wself.lock();
                               if (!self) {
                                 DLOG("RoomClient is null");
                                 return;
                               }
                               self->createNewDataConsumer(request);
                             });
}

// Notification from SFU
void MediaController::onProducerScore(
    std::shared_ptr<signaling::ProducerScoreNotification> notification) {}

void MediaController::onConsumerScore(
    std::shared_ptr<signaling::ConsumerScoreNotification> notification) {}

void MediaController::onConsumerPaused(
    std::shared_ptr<signaling::ConsumerPausedNotification> notification) {
  if (!notification) {
    return;
  }

  auto tid = notification->data->consumerId.value_or("");
  if (tid.empty()) {
    return;
  }

  _mediasoupThread->PostTask(RTC_FROM_HERE, [wself = weak_from_this(), tid]() {
    auto self = wself.lock();
    if (!self) {
      DLOG("RoomClient is null");
      return;
    }

    self->updateConsumer(tid, true);
  });
}

void MediaController::onConsumerResumed(
    std::shared_ptr<signaling::ConsumerResumedNotification> notification) {
  if (!notification) {
    return;
  }

  auto tid = notification->data->consumerId.value_or("");
  if (tid.empty()) {
    return;
  }

  _mediasoupThread->PostTask(RTC_FROM_HERE, [wself = weak_from_this(), tid]() {
    auto self = wself.lock();
    if (!self) {
      DLOG("RoomClient is null");
      return;
    }
    self->updateConsumer(tid, false);
  });
}

void MediaController::onConsumerClosed(
    std::shared_ptr<signaling::ConsumerClosedNotification> notification) {
  _mediasoupThread->PostTask(
      RTC_FROM_HERE, [wself = weak_from_this(), notification]() {
        auto self = wself.lock();
        if (!self) {
          DLOG("RoomClient is null");
          return;
        }

        for (const auto &consumer : self->_consumerMap) {
          if (consumer.second->GetId() ==
              notification->data->consumerId.value_or("")) {
            auto peerId = self->_consumerIdToPeerId[consumer.second->GetId()];
            auto tid = consumer.second->GetId();
            // auto track = consumer.second->GetTrack();

            if (consumer.second->GetKind() == "audio") {
              self->UniversalObservable<IMediaEventHandler>::notifyObservers(
                  [wself, peerId, tid](const auto &observer) {
                    auto self = wself.lock();
                    if (!self) {
                      DLOG("RoomClient is null");
                      return;
                    }
                    observer->onRemoteAudioStateChanged(peerId, true);
                    observer->onRemoveRemoteAudioTrack(peerId, tid, nullptr);
                  });
            } else if (consumer.second->GetKind() == "video") {
              self->UniversalObservable<IMediaEventHandler>::notifyObservers(
                  [wself, peerId, tid](const auto &observer) {
                    auto self = wself.lock();
                    if (!self) {
                      DLOG("RoomClient is null");
                      return;
                    }
                    observer->onRemoteVideoStateChanged(peerId, true);
                    observer->onRemoveRemoteVideoTrack(peerId, tid, nullptr);
                  });
            }
            consumer.second->Close();
            self->_consumerIdToPeerId.erase(tid);
            self->_consumerMap.erase(tid);
            return;
          }
        }
      });
}

void MediaController::onConsumerLayersChanged(
    std::shared_ptr<signaling::ConsumerLayersChangedNotification>
        notification) {}

void MediaController::onDataConsumerClosed(
    std::shared_ptr<signaling::DataConsumerClosedNotification> notification) {}

void MediaController::onDownlinkBwe(
    std::shared_ptr<signaling::DownlinkBweNotification> notification) {}

} // namespace vi
