#include "rtc_client.h"
#include "service/core.h"
#include <csignal>
bool is_exit = false;

void signal_handler(int) { is_exit = true; }

int main(int argc, char *argv[]) {
  vi::Core::init("/home/quxiaohui/workspace/mediasoup-sdk/logs/puller");

  if (argc != 5) {
    return -1;
  }

  auto puller = std::make_shared<RtcClient>(argv[1], std::stoi(argv[2]),
                                            argv[3], argv[4], 1);
  puller->init();

  puller->setYuvCallback([](const std::string &trackName, const uint8_t *y,
                            const uint8_t *u, const uint8_t *v, int32_t w,
                            int32_t h) {
    DLOG("Receive Yuv Frame, Track Name: {}, Width: {}, Height: {}", trackName,
         w, h);
  });

  signal(SIGINT, signal_handler);

  while (!is_exit) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  puller->destroy();

  vi::Core::destroy();

  return 0;
}
