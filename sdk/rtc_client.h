#ifndef RTC_CLIENT_H
#define RTC_CLIENT_H

#include "service/i_media_controller.h"
#include "service/i_participant_event_handler.h"
#include "service/i_room_client_event_handler.h"
#include "service/mediasoup_api.h"
#include "service/options.h"
#include "video_receiver.h"
#include <map>
#include <memory>
namespace vi {
class IParticipant;
class IRoomClient;
} // namespace vi

class RtcClient : public vi::IRoomClientEventHandler,
                  public vi::IParticipantEventHandler,
                  public std::enable_shared_from_this<RtcClient> {
public:
  RtcClient(std::string ip, uint16_t port, std::string room_id,
            std::string user_name, int role);

  ~RtcClient();

  void init();

  void destroy();

  void setYuvCallback(YuvCallback cb);

  void inputFrame(const std::string &trackName, const uint8_t *data,
                  uint32_t len);

  // IRoomClientEventHandler
  void onRoomStateChanged(vi::RoomState state) override;

  void onLocalActiveSpeaker(int32_t volume) override;

  // IParticipantEventHandler
  void
  onParticipantJoin(std::shared_ptr<vi::IParticipant> participant) override;

  void
  onParticipantLeave(std::shared_ptr<vi::IParticipant> participant) override;

  void onRemoteActiveSpeaker(std::shared_ptr<vi::IParticipant> participant,
                             int32_t volume) override;

  void
  onDisplayNameChanged(std::shared_ptr<vi::IParticipant> participant) override;

  void onCreateRemoteVideoTrack(
      std::shared_ptr<vi::IParticipant> participant, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track,
      const nlohmann::json &appData) override;

  void onRemoveRemoteVideoTrack(
      std::shared_ptr<vi::IParticipant> participant, const std::string &tid,
      rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track) override;

  void onRemoteAudioStateChanged(std::shared_ptr<vi::IParticipant> participant,
                                 bool muted) override;

  void onRemoteVideoStateChanged(std::shared_ptr<vi::IParticipant> participant,
                                 bool muted) override;

private:
  std::shared_ptr<vi::IRoomClient> _room_client;
  std::string _ip;
  uint16_t _port;
  std::string _room_id;
  std::string _user_name;
  int _role; // 0：推流端，1：拉流端
  std::map<webrtc::MediaStreamTrackInterface *, std::shared_ptr<VideoReceiver>>
      _video_receivers;
  YuvCallback _cb;
};
#endif // RTC_CLIENT_H
