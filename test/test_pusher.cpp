#include "reader.h"
#include "rtc_client.h"
#include "service/core.h"
#include <csignal>
bool is_exit = false;

void signal_handler(int) { is_exit = true; }

int main(int argc, char *argv[]) {
  vi::Core::init("/home/quxiaohui/workspace/mediasoup-sdk/logs/pusher");

  if (argc != 5) {
    return -1;
  }

  auto pusher = std::make_shared<RtcClient>(argv[1], std::stoi(argv[2]),
                                            argv[3], argv[4], 0);
  pusher->init();

  auto front = std::make_shared<H264Reader>(
      "/home/quxiaohui/workspace/cowa/mediasoup-client/Test/test.h264", 10);

  front->SetDataCallback([pusher](const char *data, int size) {
    if (size > 0) {
      pusher->inputFrame("front", (const uint8_t *)data, (uint32_t)size);
    }
  });

  front->Start();

  // auto back = std::make_shared<H264Reader>(
  //     "/home/quxiaohui/workspace/cowa/mediasoup-client/Test/test.h264", 15);

  // back->SetDataCallback([pusher](const char *data, int size) {
  //   if (size > 0) {
  //     pusher->inputFrame("back", (const uint8_t *)data, (uint32_t)size);
  //   }
  // });

  // back->Start();

  // auto left = std::make_shared<H264Reader>(
  //     "/home/quxiaohui/workspace/cowa/mediasoup-client/Test/test.h264", 20);

  // left->SetDataCallback([pusher](const char *data, int size) {
  //   if (size > 0) {
  //     pusher->inputFrame("left", (const uint8_t *)data, (uint32_t)size);
  //   }
  // });

  // left->Start();

  // auto right = std::make_shared<H264Reader>(
  //     "/home/quxiaohui/workspace/cowa/mediasoup-client/Test/test.h264", 25);

  // right->SetDataCallback([pusher](const char *data, int size) {
  //   if (size > 0) {
  //     pusher->inputFrame("right", (const uint8_t *)data, (uint32_t)size);
  //   }
  // });

  // right->Start();

  signal(SIGINT, signal_handler);

  while (!is_exit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  front->Stop();
  // back->Stop();
  // left->Stop();
  // right->Stop();

  pusher->destroy();

  vi::Core::destroy();

  return 0;
}
