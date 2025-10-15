/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "video_receiver.h"
#include <sstream>

VideoReceiver::VideoReceiver(const std::string &trackName, YuvCallback cb)
    : _trackName(trackName), _cb(cb) {}

VideoReceiver::~VideoReceiver() {}

void VideoReceiver::OnFrame(const webrtc::VideoFrame &frame) {
  rtc::scoped_refptr<webrtc::VideoFrameBuffer> vfb = frame.video_frame_buffer();
  if (_cb) {
    _cb(_trackName, vfb.get()->GetI420()->DataY(),
        vfb.get()->GetI420()->DataU(), vfb.get()->GetI420()->DataV(),
        frame.width(), frame.height());
  }
}