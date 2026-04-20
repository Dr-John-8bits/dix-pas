#include "dixpas/fram_i2c_backend.hpp"

#include <string.h>

#if defined(ARDUINO)
#include <Wire.h>
#endif

namespace dixpas {

namespace {

constexpr size_t kFramAddressBytes = 2U;

}  // namespace

FramI2cBackend::FramI2cBackend(FramI2cPort& port, uint8_t device_address,
                               uint8_t transfer_size)
    : port_(port),
      device_address_(device_address),
      transfer_size_(transfer_size == 0U
                         ? 1U
                         : (transfer_size > kFramI2cMaxTransferSize ? kFramI2cMaxTransferSize
                                                                    : transfer_size)) {}

bool FramI2cBackend::begin(bool probe_device) {
  ready_ = false;

  if (!port_.begin()) {
    return false;
  }

  ready_ = !probe_device || port_.probe(device_address_);
  return ready_;
}

bool FramI2cBackend::read(uint16_t address, void* dst, size_t size) {
  if (size == 0U) {
    return true;
  }

  if (!ready_ || dst == nullptr || !check_bounds(address, size)) {
    return false;
  }

  auto* bytes = static_cast<uint8_t*>(dst);
  size_t remaining = size;
  uint16_t cursor = address;

  while (remaining > 0U) {
    const size_t chunk = remaining > transfer_size_ ? transfer_size_ : remaining;
    if (!read_chunk(cursor, bytes, chunk)) {
      return false;
    }

    cursor = static_cast<uint16_t>(cursor + chunk);
    bytes += chunk;
    remaining -= chunk;
  }

  return true;
}

bool FramI2cBackend::write(uint16_t address, const void* src, size_t size) {
  if (size == 0U) {
    return true;
  }

  if (!ready_ || src == nullptr || !check_bounds(address, size)) {
    return false;
  }

  const auto* bytes = static_cast<const uint8_t*>(src);
  size_t remaining = size;
  uint16_t cursor = address;

  while (remaining > 0U) {
    const size_t chunk = remaining > transfer_size_ ? transfer_size_ : remaining;
    if (!write_chunk(cursor, bytes, chunk)) {
      return false;
    }

    cursor = static_cast<uint16_t>(cursor + chunk);
    bytes += chunk;
    remaining -= chunk;
  }

  return true;
}

bool FramI2cBackend::check_bounds(uint16_t address, size_t size) const {
  return (static_cast<size_t>(address) + size) <= kMemoryStorageCapacity;
}

bool FramI2cBackend::read_chunk(uint16_t address, uint8_t* dst, size_t size) {
  uint8_t address_bytes[kFramAddressBytes] = {
      static_cast<uint8_t>((address >> 8U) & 0xFFU),
      static_cast<uint8_t>(address & 0xFFU),
  };

  if (!port_.write_bytes(device_address_, address_bytes, sizeof(address_bytes))) {
    return false;
  }

  return port_.read_bytes(device_address_, dst, size);
}

bool FramI2cBackend::write_chunk(uint16_t address, const uint8_t* src, size_t size) {
  uint8_t packet[kFramAddressBytes + kFramI2cMaxTransferSize]{};
  packet[0] = static_cast<uint8_t>((address >> 8U) & 0xFFU);
  packet[1] = static_cast<uint8_t>(address & 0xFFU);
  memcpy(packet + kFramAddressBytes, src, size);
  return port_.write_bytes(device_address_, packet, size + kFramAddressBytes);
}

#if defined(ARDUINO)
bool WireFramI2cPort::begin() {
  Wire.begin();
  if (clock_hz_ > 0U) {
    Wire.setClock(clock_hz_);
  }
  return true;
}

bool WireFramI2cPort::probe(uint8_t device_address) {
  Wire.beginTransmission(device_address);
  return Wire.endTransmission() == 0;
}

bool WireFramI2cPort::write_bytes(uint8_t device_address, const uint8_t* data, size_t size) {
  if (size == 0U) {
    return true;
  }

  Wire.beginTransmission(device_address);
  const size_t written = Wire.write(data, size);
  return written == size && Wire.endTransmission() == 0;
}

bool WireFramI2cPort::read_bytes(uint8_t device_address, uint8_t* dst, size_t size) {
  if (size == 0U) {
    return true;
  }

  const size_t requested = Wire.requestFrom(static_cast<int>(device_address), static_cast<int>(size));
  if (requested != size) {
    return false;
  }

  for (size_t index = 0; index < size; ++index) {
    if (Wire.available() <= 0) {
      return false;
    }
    dst[index] = static_cast<uint8_t>(Wire.read());
  }

  return true;
}
#endif

}  // namespace dixpas
