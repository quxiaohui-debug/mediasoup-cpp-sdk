#ifndef _ENCODER_H_
#define _ENCODER_H_
#include "absl/strings/match.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_encoder.h"
#include "common_video/h264/h264_common.h"
#include "media/base/media_constants.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/include/video_error_codes.h"
#include "rtc_base/logging.h"

class DummyEncoder : public webrtc::VideoEncoder {
public:
  DummyEncoder() : m_encoded_image_callback(nullptr) {}

  virtual ~DummyEncoder() override {}

  int32_t InitEncode(const webrtc::VideoCodec *codec_settings,
                     const webrtc::VideoEncoder::Settings &settings) override {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  int32_t RegisterEncodeCompleteCallback(
      webrtc::EncodedImageCallback *callback) override {
    m_encoded_image_callback = callback;
    return WEBRTC_VIDEO_CODEC_OK;
  }

  int32_t Release() override { return WEBRTC_VIDEO_CODEC_OK; }

  int32_t
  Encode(const webrtc::VideoFrame &frame,
         const std::vector<webrtc::VideoFrameType> *frame_types) override {
    return WEBRTC_VIDEO_CODEC_OK;
  }

  void SetRates(
      const webrtc::VideoEncoder::RateControlParameters &parameters) override {}

  webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override;

protected:
  webrtc::EncodedImageCallback *m_encoded_image_callback;
};

class DummyH264Encoder : public DummyEncoder {
public:
  DummyH264Encoder() {}

  virtual ~DummyH264Encoder() override {}

  int32_t
  Encode(const webrtc::VideoFrame &frame,
         const std::vector<webrtc::VideoFrameType> *frame_types) override;

  webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override;
};

class DummyVideoEncoderFactory : public webrtc::VideoEncoderFactory {
public:
  DummyVideoEncoderFactory();

  virtual ~DummyVideoEncoderFactory() override {}

  std::unique_ptr<webrtc::VideoEncoder>
  CreateVideoEncoder(const webrtc::SdpVideoFormat &format) override {
    if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
      return std::make_unique<DummyH264Encoder>();
    else
      return nullptr;
  }

  std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override {
    return supported_formats_;
  }

private:
  std::vector<webrtc::SdpVideoFormat> supported_formats_;
};

#endif // !_ENCODER_H_