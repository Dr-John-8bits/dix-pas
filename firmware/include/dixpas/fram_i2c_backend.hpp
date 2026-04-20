#pragma once

#include <stddef.h>
#include <stdint.h>

#include "dixpas/storage_engine.hpp"

namespace dixpas {

constexpr uint8_t kDefaultFramI2cAddress = 0x50U;
constexpr uint8_t kFramI2cMaxTransferSize = 24U;

class FramI2cPort {
 public:
  virtual ~FramI2cPort() = default;

  virtual bool begin() = 0;
  virtual bool probe(uint8_t device_address) = 0;
  virtual bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) = 0;
  virtual bool read_bytes(uint8_t device_address, uint8_t* dst, size_t size) = 0;
};

class FramI2cBackend : public StorageBackend {
 public:
  explicit FramI2cBackend(FramI2cPort& port, uint8_t device_address = kDefaultFramI2cAddress,
                          uint8_t transfer_size = kFramI2cMaxTransferSize);

  bool begin(bool probe_device = true);
  [[nodiscard]] bool is_ready() const { return ready_; }

  bool read(uint16_t address, void* dst, size_t size) override;
  bool write(uint16_t address, const void* src, size_t size) override;

 private:
  FramI2cPort& port_;
  uint8_t device_address_ = kDefaultFramI2cAddress;
  uint8_t transfer_size_ = kFramI2cMaxTransferSize;
  bool ready_ = false;

  bool check_bounds(uint16_t address, size_t size) const;
  bool read_chunk(uint16_t address, uint8_t* dst, size_t size);
  bool write_chunk(uint16_t address, const uint8_t* src, size_t size);
};

#if defined(ARDUINO)
class WireFramI2cPort : public FramI2cPort {
 public:
  explicit WireFramI2cPort(uint32_t clock_hz = 400000U) : clock_hz_(clock_hz) {}

  bool begin() override;
  bool probe(uint8_t device_address) override;
  bool write_bytes(uint8_t device_address, const uint8_t* data, size_t size) override;
  bool read_bytes(uint8_t device_address, uint8_t* dst, size_t size) override;

 private:
  uint32_t clock_hz_ = 400000U;
};
#endif

}  // namespace dixpas
