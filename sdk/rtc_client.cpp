#include "rtc_client.h"
#include "logger/spd_logger.h"
#include "service/component_factory.h"
#include "service/engine.h"
#include "service/i_media_controller.h"
#include "service/i_participant_controller.h"
#include "service/participant.h"
#include "service/room_client.h"
#include "service/signaling_client.h"
#include "service/signaling_models.h"
#include "utils/thread_provider.h"
#include "websocket/websocket_request.h"

RtcClient::RtcClient(std::string ip, uint16_t port, std::string room_id,
                     std::string user_name, int role)
    : _ip(ip), _port(port), _room_id(room_id), _user_name(user_name),
      _role(role), _cb(nullptr) {
  DLOG("RtcClient() Construct, Ip:{}, Port:{}, Room Id:{}, User Name:{}, "
       "Role:{}",
       ip, port, room_id, user_name, role);
}

RtcClient::~RtcClient() {
  DLOG("~RtcClient() Desstruct, Ip:{}, Port:{}, Room Id:{}, User Name:{}", _ip,
       _port, _room_id, _user_name);
}

void RtcClient::init() {
  _room_client = getEngine()->createRoomClient();
  rtc::Thread *callbackThread = getThread("main");
  _room_client->addObserver(shared_from_this(), callbackThread);
  std::shared_ptr<vi::IParticipantController> participantController =
      _room_client->getParticipantController();
  if (participantController) {
    participantController->addObserver(shared_from_this(), callbackThread);
  }
  if (_room_client) {
    _room_client->join(_ip, _port, _room_id, _user_name, nullptr);
  }
}

void RtcClient::destroy() {
  _room_client->removeObserver(shared_from_this());
  std::shared_ptr<vi::IParticipantController> participantController =
      _room_client->getParticipantController();
  if (participantController) {
    participantController->removeObserver(shared_from_this());
  }
  if (_room_client) {
    _room_client->leave();
    _room_client = nullptr;
  }
}

void RtcClient::setYuvCallback(YuvCallback cb) {
  //拉流端
  if (_role == 1) {
    _cb = cb;
  }
}

void RtcClient::inputFrame(const std::string &trackName, const uint8_t *data,
                           uint32_t len) {
  std::shared_ptr<vi::IMediaController> mediaController =
      _room_client->getMediaController();
  if (mediaController) {
    mediaController->inputFrame(trackName, data, len);
  }
}

void RtcClient::onRoomStateChanged(vi::RoomState state) {
  DLOG("onRoomStateChanged, state: {}", (int32_t)state);
  if (state == vi::RoomState::CONNECTED) {
    if (_room_client) {
      //推流端
      if (_role == 0) {
        //_room_client->enableAudio(true);
        _room_client->enableVideo(true, "front", 1280, 720);
        // _room_client->enableVideo(true, "back", 1280, 720);
        // _room_client->enableVideo(true, "left", 1280, 720);
        // _room_client->enableVideo(true, "right", 1280, 720);
      }
    }
  } else if (state == vi::RoomState::CLOSED) {
  }
}

void RtcClient::onLocalActiveSpeaker(int32_t volume) {
  DLOG("onLocalActiveSpeaker, volume: {}", volume);
}

void RtcClient::onParticipantJoin(
    std::shared_ptr<vi::IParticipant> participant) {
  DLOG("onParticipantJoin, display name: {}", participant->displayName());
}

void RtcClient::onParticipantLeave(
    std::shared_ptr<vi::IParticipant> participant) {
  DLOG("onParticipantLeave, display name: {}", participant->displayName());
}

void RtcClient::onRemoteActiveSpeaker(
    std::shared_ptr<vi::IParticipant> participant, int32_t volume) {
  DLOG("onRemoteActiveSpeaker, display name :{}, volume: {}",
       participant->displayName(), volume);
}

void RtcClient::onDisplayNameChanged(
    std::shared_ptr<vi::IParticipant> participant) {
  DLOG("onDisplayNameChanged, display name: {}", participant->displayName());
}

void RtcClient::onCreateRemoteVideoTrack(
    std::shared_ptr<vi::IParticipant> participant, const std::string &tid,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
    const nlohmann::json &appData) {
  signaling::SharingAppData sharingAppData;
  signaling::from_json(appData, sharingAppData);
  DLOG("onCreateRemoteVideoTrack, display name: {}, tid: {}, track id: {}, "
       "type: {}",
       participant->displayName(), tid, track->id(),
       sharingAppData.sharing.trackName);
  //拉流端
  if (_role == 1) {
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
        (webrtc::VideoTrackInterface *)track.get());
    auto video_render =
        std::make_shared<VideoReceiver>(sharingAppData.sharing.trackName, _cb);
    video_track->AddOrUpdateSink(video_render.get(), rtc::VideoSinkWants());
    _video_receivers[track.get()] = video_render;
  }
}

void RtcClient::onRemoveRemoteVideoTrack(
    std::shared_ptr<vi::IParticipant> participant, const std::string &tid,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track) {
  DLOG("onRemoveRemoteVideoTrack, display name: {}, tid: {}, track id: {}",
       participant->displayName(), tid, track->id());
  //拉流端
  if (_role == 1) {
    _video_receivers.erase(track.get());
  }
}

void RtcClient::onRemoteAudioStateChanged(
    std::shared_ptr<vi::IParticipant> participant, bool muted) {
  DLOG("onRemoteAudioStateChanged, display name: {}, muted: {}",
       participant->displayName(), muted);
}

void RtcClient::onRemoteVideoStateChanged(
    std::shared_ptr<vi::IParticipant> participant, bool muted) {
  DLOG("onRemoteVideoStateChanged, display name: {}, muted: {}",
       participant->displayName(), muted);
}
