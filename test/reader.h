#ifndef _READER_H_
#define _READER_H_
#include <atomic>
#include <cstring>
#include <functional>
#include <string>
#include <thread>
using DataCallback = std::function<void(const char *data, int size)>;
class Reader {
public:
  Reader(std::string path);
  virtual ~Reader();
  bool Start();
  void Stop();
  void SetDataCallback(DataCallback cb);
  virtual void RunLoop() = 0;

protected:
  std::string m_path;
  DataCallback m_cb;
  std::unique_ptr<std::thread> m_thread;
  std::atomic_bool m_bStart;
  FILE *m_fp;
  char *m_buf;
};

class H264Reader : public Reader {
public:
  H264Reader(std::string path, uint32_t fps);
  ~H264Reader();
  virtual void RunLoop() override;

private:
  void ReadOneNalu();

private:
  uint32_t m_fps;
  std::string m_sps;
  std::string m_pps;
  int64_t _lastFrameMillis;
};

class PCMReader : public Reader {
public:
  PCMReader(std::string path, int frameInterval, int frameSize);
  ~PCMReader();
  virtual void RunLoop() override;

private:
  int m_frameInterval;
  int m_frameSize;
};

class YUVReader : public Reader {
public:
  YUVReader(std::string path, int frameRate, int width, int height);
  ~YUVReader();
  virtual void RunLoop() override;

private:
  int m_frameRate;
  int m_width;
  int m_height;
};
#endif // !_READER_H_