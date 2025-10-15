#ifndef _CAPTURER_H_
#define _CAPTURER_H_
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/video/i420_buffer.h"
#include "media/base/adapted_video_track_source.h"
#include "modules/video_capture/video_capture_factory.h"
#include "pc/video_track_source.h"
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class EncodedVideoI420Buffer : public webrtc::I420BufferInterface {
public:
  EncodedVideoI420Buffer(int32_t width, int32_t height,
                         const std::string &encoded_data)
      : m_width(width), m_height(height), m_encoded_data(encoded_data) {}
  virtual ~EncodedVideoI420Buffer() {}
  virtual int32_t width() const { return m_width; }
  virtual int32_t height() const { return m_height; }
  virtual const uint8_t *DataY() const {
    return (const uint8_t *)m_encoded_data.data();
  }
  virtual const uint8_t *DataU() const { return NULL; }
  virtual const uint8_t *DataV() const { return NULL; }
  virtual int StrideY() const { return m_encoded_data.size(); }
  virtual int StrideU() const { return 0; }
  virtual int StrideV() const { return 0; }

private:
  const int32_t m_width;
  const int32_t m_height;
  std::string m_encoded_data;
};

class EncodedVideoFrameBuffer : public webrtc::VideoFrameBuffer {
public:
  EncodedVideoFrameBuffer(int32_t width, int32_t height,
                          const std::string &encoded_data);
  virtual Type type() const { return webrtc::VideoFrameBuffer::Type::kNative; }
  virtual rtc::scoped_refptr<webrtc::I420BufferInterface> ToI420() {
    return nullptr;
  }
  virtual int width() const { return m_buffer->width(); }
  virtual int height() const { return m_buffer->height(); }
  virtual const webrtc::I420BufferInterface *GetI420() const final {
    return m_buffer.get();
  }

private:
  rtc::scoped_refptr<EncodedVideoI420Buffer> m_buffer;
};

class DummyVideoSource : public rtc::AdaptedVideoTrackSource {
public:
  void InputFrame(const uint8_t *data, uint32_t len);

  virtual webrtc::MediaSourceInterface::SourceState state() const override;
  virtual bool remote() const override;
  virtual bool is_screencast() const override;
  virtual absl::optional<bool> needs_denoising() const override;

  virtual void AddRef() const override;
  virtual rtc::RefCountReleaseStatus Release() const override;

  explicit DummyVideoSource(uint32_t width, uint32_t height);

  ~DummyVideoSource();

private:
  mutable volatile int ref_count_;
  uint32_t m_width;
  uint32_t m_height;
  std::string m_sps, m_pps;
};
#endif // !_CAPTURER_H_
