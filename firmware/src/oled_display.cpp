#include "dixpas/oled_display.hpp"

#include <ctype.h>
#include <string.h>

#if defined(ARDUINO)
#include <Wire.h>
#endif

namespace dixpas {

namespace {

constexpr uint8_t kOledControlCommand = 0x00U;
constexpr uint8_t kOledControlData = 0x40U;
constexpr uint8_t kOledCharWidth = 6U;

const uint8_t* glyph_for_char(char c) {
  static constexpr uint8_t kGlyphSpace[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
  static constexpr uint8_t kGlyphBang[5] = {0x00, 0x00, 0x5F, 0x00, 0x00};
  static constexpr uint8_t kGlyphHash[5] = {0x14, 0x7F, 0x14, 0x7F, 0x14};
  static constexpr uint8_t kGlyphPercent[5] = {0x23, 0x13, 0x08, 0x64, 0x62};
  static constexpr uint8_t kGlyphPlus[5] = {0x08, 0x08, 0x3E, 0x08, 0x08};
  static constexpr uint8_t kGlyphComma[5] = {0x00, 0x50, 0x30, 0x00, 0x00};
  static constexpr uint8_t kGlyphDash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
  static constexpr uint8_t kGlyphDot[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
  static constexpr uint8_t kGlyphSlash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
  static constexpr uint8_t kGlyphColon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
  static constexpr uint8_t kGlyphQuestion[5] = {0x02, 0x01, 0x51, 0x09, 0x06};
  static constexpr uint8_t kGlyphGreater[5] = {0x08, 0x14, 0x22, 0x41, 0x00};
  static constexpr uint8_t kGlyphEqual[5] = {0x14, 0x14, 0x14, 0x14, 0x14};

  static constexpr uint8_t kGlyph0[5] = {0x3E, 0x51, 0x49, 0x45, 0x3E};
  static constexpr uint8_t kGlyph1[5] = {0x00, 0x42, 0x7F, 0x40, 0x00};
  static constexpr uint8_t kGlyph2[5] = {0x42, 0x61, 0x51, 0x49, 0x46};
  static constexpr uint8_t kGlyph3[5] = {0x21, 0x41, 0x45, 0x4B, 0x31};
  static constexpr uint8_t kGlyph4[5] = {0x18, 0x14, 0x12, 0x7F, 0x10};
  static constexpr uint8_t kGlyph5[5] = {0x27, 0x45, 0x45, 0x45, 0x39};
  static constexpr uint8_t kGlyph6[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30};
  static constexpr uint8_t kGlyph7[5] = {0x01, 0x71, 0x09, 0x05, 0x03};
  static constexpr uint8_t kGlyph8[5] = {0x36, 0x49, 0x49, 0x49, 0x36};
  static constexpr uint8_t kGlyph9[5] = {0x06, 0x49, 0x49, 0x29, 0x1E};

  static constexpr uint8_t kGlyphA[5] = {0x7E, 0x11, 0x11, 0x11, 0x7E};
  static constexpr uint8_t kGlyphB[5] = {0x7F, 0x49, 0x49, 0x49, 0x36};
  static constexpr uint8_t kGlyphC[5] = {0x3E, 0x41, 0x41, 0x41, 0x22};
  static constexpr uint8_t kGlyphD[5] = {0x7F, 0x41, 0x41, 0x22, 0x1C};
  static constexpr uint8_t kGlyphE[5] = {0x7F, 0x49, 0x49, 0x49, 0x41};
  static constexpr uint8_t kGlyphF[5] = {0x7F, 0x09, 0x09, 0x09, 0x01};
  static constexpr uint8_t kGlyphG[5] = {0x3E, 0x41, 0x49, 0x49, 0x7A};
  static constexpr uint8_t kGlyphH[5] = {0x7F, 0x08, 0x08, 0x08, 0x7F};
  static constexpr uint8_t kGlyphI[5] = {0x00, 0x41, 0x7F, 0x41, 0x00};
  static constexpr uint8_t kGlyphJ[5] = {0x20, 0x40, 0x41, 0x3F, 0x01};
  static constexpr uint8_t kGlyphK[5] = {0x7F, 0x08, 0x14, 0x22, 0x41};
  static constexpr uint8_t kGlyphL[5] = {0x7F, 0x40, 0x40, 0x40, 0x40};
  static constexpr uint8_t kGlyphM[5] = {0x7F, 0x02, 0x0C, 0x02, 0x7F};
  static constexpr uint8_t kGlyphN[5] = {0x7F, 0x04, 0x08, 0x10, 0x7F};
  static constexpr uint8_t kGlyphO[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};
  static constexpr uint8_t kGlyphP[5] = {0x7F, 0x09, 0x09, 0x09, 0x06};
  static constexpr uint8_t kGlyphQ[5] = {0x3E, 0x41, 0x51, 0x21, 0x5E};
  static constexpr uint8_t kGlyphR[5] = {0x7F, 0x09, 0x19, 0x29, 0x46};
  static constexpr uint8_t kGlyphS[5] = {0x46, 0x49, 0x49, 0x49, 0x31};
  static constexpr uint8_t kGlyphT[5] = {0x01, 0x01, 0x7F, 0x01, 0x01};
  static constexpr uint8_t kGlyphU[5] = {0x3F, 0x40, 0x40, 0x40, 0x3F};
  static constexpr uint8_t kGlyphV[5] = {0x1F, 0x20, 0x40, 0x20, 0x1F};
  static constexpr uint8_t kGlyphW[5] = {0x7F, 0x20, 0x18, 0x20, 0x7F};
  static constexpr uint8_t kGlyphX[5] = {0x63, 0x14, 0x08, 0x14, 0x63};
  static constexpr uint8_t kGlyphY[5] = {0x03, 0x04, 0x78, 0x04, 0x03};
  static constexpr uint8_t kGlyphZ[5] = {0x61, 0x51, 0x49, 0x45, 0x43};

  const unsigned char normalized =
      static_cast<unsigned char>(islower(static_cast<unsigned char>(c)) ? toupper(c) : c);

  switch (normalized) {
    case ' ':
      return kGlyphSpace;
    case '!':
      return kGlyphBang;
    case '#':
      return kGlyphHash;
    case '%':
      return kGlyphPercent;
    case '+':
      return kGlyphPlus;
    case ',':
      return kGlyphComma;
    case '-':
      return kGlyphDash;
    case '.':
      return kGlyphDot;
    case '/':
      return kGlyphSlash;
    case ':':
      return kGlyphColon;
    case '?':
      return kGlyphQuestion;
    case '>':
      return kGlyphGreater;
    case '=':
      return kGlyphEqual;
    case '0':
      return kGlyph0;
    case '1':
      return kGlyph1;
    case '2':
      return kGlyph2;
    case '3':
      return kGlyph3;
    case '4':
      return kGlyph4;
    case '5':
      return kGlyph5;
    case '6':
      return kGlyph6;
    case '7':
      return kGlyph7;
    case '8':
      return kGlyph8;
    case '9':
      return kGlyph9;
    case 'A':
      return kGlyphA;
    case 'B':
      return kGlyphB;
    case 'C':
      return kGlyphC;
    case 'D':
      return kGlyphD;
    case 'E':
      return kGlyphE;
    case 'F':
      return kGlyphF;
    case 'G':
      return kGlyphG;
    case 'H':
      return kGlyphH;
    case 'I':
      return kGlyphI;
    case 'J':
      return kGlyphJ;
    case 'K':
      return kGlyphK;
    case 'L':
      return kGlyphL;
    case 'M':
      return kGlyphM;
    case 'N':
      return kGlyphN;
    case 'O':
      return kGlyphO;
    case 'P':
      return kGlyphP;
    case 'Q':
      return kGlyphQ;
    case 'R':
      return kGlyphR;
    case 'S':
      return kGlyphS;
    case 'T':
      return kGlyphT;
    case 'U':
      return kGlyphU;
    case 'V':
      return kGlyphV;
    case 'W':
      return kGlyphW;
    case 'X':
      return kGlyphX;
    case 'Y':
      return kGlyphY;
    case 'Z':
      return kGlyphZ;
    default:
      return kGlyphQuestion;
  }
}

}  // namespace

OledDisplay::OledDisplay(OledI2cPort& port, uint8_t device_address, uint8_t transfer_size)
    : port_(port),
      device_address_(device_address),
      transfer_size_(transfer_size == 0U
                         ? 1U
                         : (transfer_size > kOledMaxTransferSize ? kOledMaxTransferSize
                                                                 : transfer_size)) {}

bool OledDisplay::begin() {
  ready_ = false;
  if (!port_.begin()) {
    return false;
  }

  static constexpr uint8_t kInitSequence[] = {
      0xAE,       0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
      0x8D,       0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
      0x81,       0xCF, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6,
      0x2E,       0xAF,
  };

  if (!send_commands(kInitSequence, sizeof(kInitSequence))) {
    return false;
  }

  ready_ = true;
  return clear();
}

bool OledDisplay::clear() {
  if (!ready_) {
    return false;
  }

  uint8_t page_buffer[kOledWidth]{};
  memset(page_buffer, 0, sizeof(page_buffer));

  for (uint8_t page = 0; page < kOledPageCount; ++page) {
    const uint8_t commands[] = {
        static_cast<uint8_t>(0xB0U + page), 0x00U, 0x10U,
    };

    if (!send_commands(commands, sizeof(commands))) {
      return false;
    }

    for (uint8_t x = 0; x < kOledWidth; x = static_cast<uint8_t>(x + transfer_size_)) {
      const size_t chunk =
          (kOledWidth - x) > transfer_size_ ? transfer_size_ : (kOledWidth - x);
      if (!send_data_chunk(page_buffer + x, chunk)) {
        return false;
      }
    }
  }

  return true;
}

bool OledDisplay::render(const DisplayFrame& frame) {
  if (!ready_) {
    return false;
  }

  for (uint8_t page = 0; page < kOledPageCount; ++page) {
    uint8_t page_buffer[kOledWidth]{};
    memset(page_buffer, 0, sizeof(page_buffer));

    if ((page % 2U) == 0U) {
      const uint8_t line_index = static_cast<uint8_t>(page / 2U);
      if (line_index < kDisplayLineCount) {
        draw_text(page_buffer, 0U, frame.lines[line_index]);
      }
    }

    const uint8_t commands[] = {
        static_cast<uint8_t>(0xB0U + page), 0x00U, 0x10U,
    };

    if (!send_commands(commands, sizeof(commands))) {
      return false;
    }

    for (uint8_t x = 0; x < kOledWidth; x = static_cast<uint8_t>(x + transfer_size_)) {
      const size_t chunk =
          (kOledWidth - x) > transfer_size_ ? transfer_size_ : (kOledWidth - x);
      if (!send_data_chunk(page_buffer + x, chunk)) {
        return false;
      }
    }
  }

  return true;
}

bool OledDisplay::send_commands(const uint8_t* commands, size_t size) {
  if (commands == nullptr) {
    return false;
  }

  uint8_t packet[kOledMaxTransferSize + 1U]{};
  for (size_t offset = 0; offset < size; offset += transfer_size_) {
    const size_t chunk = (size - offset) > transfer_size_ ? transfer_size_ : (size - offset);
    packet[0] = kOledControlCommand;
    memcpy(packet + 1U, commands + offset, chunk);
    if (!port_.write_bytes(device_address_, packet, chunk + 1U)) {
      return false;
    }
  }

  return true;
}

bool OledDisplay::send_data_chunk(const uint8_t* data, size_t size) {
  if (data == nullptr || size == 0U) {
    return false;
  }

  uint8_t packet[kOledMaxTransferSize + 1U]{};
  packet[0] = kOledControlData;
  memcpy(packet + 1U, data, size);
  return port_.write_bytes(device_address_, packet, size + 1U);
}

void OledDisplay::draw_text(uint8_t* page_buffer, uint8_t x, const char* text) {
  if (text == nullptr || page_buffer == nullptr) {
    return;
  }

  uint8_t cursor_x = x;
  for (size_t index = 0; text[index] != '\0'; ++index) {
    if ((cursor_x + kOledCharWidth) > kOledWidth) {
      break;
    }
    draw_char(page_buffer, cursor_x, text[index]);
    cursor_x = static_cast<uint8_t>(cursor_x + kOledCharWidth);
  }
}

void OledDisplay::draw_char(uint8_t* page_buffer, uint8_t x, char c) {
  const uint8_t* glyph = glyph_for_char(c);
  for (uint8_t column = 0; column < 5U; ++column) {
    if ((x + column) < kOledWidth) {
      page_buffer[x + column] = glyph[column];
    }
  }
}

#if defined(ARDUINO)
bool WireOledI2cPort::begin() {
  Wire.begin();
  if (clock_hz_ > 0U) {
    Wire.setClock(clock_hz_);
  }
  return true;
}

bool WireOledI2cPort::write_bytes(uint8_t device_address, const uint8_t* data, size_t size) {
  if (data == nullptr || size == 0U) {
    return false;
  }

  Wire.beginTransmission(device_address);
  const size_t written = Wire.write(data, size);
  return written == size && Wire.endTransmission() == 0;
}
#endif

}  // namespace dixpas
