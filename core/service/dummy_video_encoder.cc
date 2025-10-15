#include "dummy_video_encoder.h"

int32_t DummyH264Encoder::Encode(
    const webrtc::VideoFrame &frame,
    const std::vector<webrtc::VideoFrameType> *frame_types) {
  if (!m_encoded_image_callback) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer =
      frame.video_frame_buffer();
  if (buffer->type() != webrtc::VideoFrameBuffer::Type::kNative) {
    return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
  }

  uint8_t *data = (uint8_t *)buffer->GetI420()->DataY();
  uint32_t len = buffer->GetI420()->StrideY();
  webrtc::VideoFrameType frameType = webrtc::VideoFrameType::kVideoFrameDelta;
  std::vector<webrtc::H264::NaluIndex> naluIndexes =
      webrtc::H264::FindNaluIndices(data, len);
  for (auto &index : naluIndexes) {
    webrtc::H264::NaluType nalu_type =
        webrtc::H264::ParseNaluType(data[index.payload_start_offset]);
    if (nalu_type == webrtc::H264::kIdr) {
      frameType = webrtc::VideoFrameType::kVideoFrameKey;
      break;
    }
  }

  webrtc::EncodedImage encoded_image;
  encoded_image.SetEncodedData(webrtc::EncodedImageBuffer::Create(data, len));

  encoded_image._encodedHeight = frame.height();
  encoded_image._encodedWidth = frame.width();
  encoded_image.capture_time_ms_ = frame.render_time_ms();

  encoded_image.SetTimestamp(frame.timestamp());
  encoded_image.ntp_time_ms_ = frame.ntp_time_ms();
  encoded_image._frameType = frameType;

  webrtc::CodecSpecificInfo codec_specific;
  codec_specific.codecType = webrtc::VideoCodecType::kVideoCodecH264;
  codec_specific.codecSpecific.H264.packetization_mode =
      webrtc::H264PacketizationMode::NonInterleaved;
  webrtc::EncodedImageCallback::Result result =
      m_encoded_image_callback->OnEncodedImage(encoded_image, &codec_specific);

  if (result.error == webrtc::EncodedImageCallback::Result::ERROR_SEND_FAILED) {
    RTC_LOG(LS_ERROR) << "Error in parsing EncodedImage"
                      << encoded_image._frameType;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

webrtc::VideoEncoder::EncoderInfo DummyH264Encoder::GetEncoderInfo() const {
  webrtc::VideoEncoder::EncoderInfo info;
  info.supports_native_handle = true;
  info.implementation_name = "NULL_H264Encoder";
  info.has_trusted_rate_controller = true;
  return info;
}

webrtc::VideoEncoder::EncoderInfo DummyEncoder::GetEncoderInfo() const {
  webrtc::VideoEncoder::EncoderInfo info;
  info.supports_native_handle = true;
  info.implementation_name = "NULL_Encoder";
  info.has_trusted_rate_controller = true;
  return info;
}

DummyVideoEncoderFactory::DummyVideoEncoderFactory() {
  for (const webrtc::SdpVideoFormat &format : webrtc::SupportedH264Codecs()) {
    supported_formats_.push_back(format);
  }
}
