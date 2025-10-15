
#pragma once

#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include <functional>
using YuvCallback =
    std::function<void(const std::string &, const uint8_t *, const uint8_t *,
                       const uint8_t *, int32_t, int32_t)>; //(T,Y,U,V,W,H)
class VideoReceiver : public rtc::VideoSinkInterface<webrtc::VideoFrame>,
                      public std::enable_shared_from_this<VideoReceiver> {
public:
  VideoReceiver(const std::string &trackName, YuvCallback cb = nullptr);

  ~VideoReceiver();

  void OnFrame(const webrtc::VideoFrame &frame) override;

private:
  std::string _trackName; //相机名称
  YuvCallback _cb;        //回调函数
};