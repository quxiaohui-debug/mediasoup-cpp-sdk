#include "dummy_video_source.h"
#include "rtc_base/atomic_ops.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/video_common.h"

void DummyVideoSource::AddRef() const {
  rtc::AtomicOps::Increment(&ref_count_);
}

rtc::RefCountReleaseStatus DummyVideoSource::Release() const {
  const int count = rtc::AtomicOps::Decrement(&ref_count_);
  if (count == 0) {
    return rtc::RefCountReleaseStatus::kDroppedLastRef;
  }
  return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}

DummyVideoSource::DummyVideoSource(uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
}

DummyVideoSource::~DummyVideoSource() {}

webrtc::MediaSourceInterface::SourceState DummyVideoSource::state() const {
  return webrtc::MediaSourceInterface::kLive;
}

bool DummyVideoSource::remote() const { return false; }

bool DummyVideoSource::is_screencast() const { return true; }

absl::optional<bool> DummyVideoSource::needs_denoising() const { return false; }

void DummyVideoSource::InputFrame(const uint8_t *data, uint32_t len) {
  rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer =
      new rtc::RefCountedObject<EncodedVideoFrameBuffer>(
          m_width, m_height, std::string((const char *)data, len));

  webrtc::VideoFrame video_frame = webrtc::VideoFrame::Builder()
                                       .set_video_frame_buffer(buffer)
                                       .set_timestamp_ms(rtc::TimeMillis())
                                       .set_rotation(webrtc::kVideoRotation_0)
                                       .build();

  OnFrame(video_frame);
}

EncodedVideoFrameBuffer::EncodedVideoFrameBuffer(
    int32_t width, int32_t height, const std::string &encoded_data) {
  m_buffer = new rtc::RefCountedObject<EncodedVideoI420Buffer>(width, height,
                                                               encoded_data);
}
