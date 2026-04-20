#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/display_engine.hpp"

namespace dixpas {

constexpr uint8_t kDefaultOledI2cAddress = 0x3CU;
constexpr uint8_t kOledWidth = 128U;
constexpr uint8_t kOledHeight = 64U;
constexpr uint8_t kOledPageCount = kOledHeight / 8U;
constexpr uint8_t kOledMaxTransferSize = 16U;

class OledI2cPort {
 public:
  virtual ~OledI2cPort() = default;

  virtual bool begin() = 0;
  virtual bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) = 0;
};

class OledDisplay {
 public:
  explicit OledDisplay(OledI2cPort& port, uint8_t device_address = kDefaultOledI2cAddress,
                       uint8_t transfer_size = kOledMaxTransferSize);

  bool begin();
 bool clear();
  bool render(const DisplayFrame& frame);
  [[nodiscard]] bool is_ready() const { return ready_; }

 private:
  OledI2cPort& port_;
  uint8_t device_address_ = kDefaultOledI2cAddress;
  uint8_t transfer_size_ = kOledMaxTransferSize;
  bool ready_ = false;

  bool send_commands(const uint8_t* commands, size_t size);
  bool send_data_chunk(const uint8_t* data, size_t size);
  void draw_text(uint8_t* page_buffer, uint8_t x, const char* text);
  void draw_char(uint8_t* page_buffer, uint8_t x, char c);
};

#if defined(ARDUINO)
class WireOledI2cPort : public OledI2cPort {
 public:
  explicit WireOledI2cPort(uint32_t clock_hz = 400000U) : clock_hz_(clock_hz) {}

  bool begin() override;
  bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) override;

 private:
  uint32_t clock_hz_ = 400000U;
};
#endif

}  // namespace dixpas
