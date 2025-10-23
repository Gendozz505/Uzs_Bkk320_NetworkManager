#pragma once

#include <boost/crc.hpp>
#include <ifaddrs.h>
#include <netdb.h>
#include <spdlog/spdlog.h>
#include <string>
#include <type_traits>
#include <vector>

#define MESSAGE_MIN_SIZE 10

namespace Common {
struct NetMessage {
  uint8_t cmd;                  // 1 byte - Command
  uint16_t serialNumber;        // 2 bytes - Serial Number
  uint8_t status;               // 1 byte - Status
  uint32_t dataLen;             // 4 bytes - Data Length
  std::vector<uint8_t> payload; // N bytes - Data Payload
  uint16_t crc;                 // 2 bytes - CRC16 checksum

  NetMessage()
      : cmd(0), serialNumber(0), status(0), dataLen(0),
        payload(std::vector<uint8_t>()), crc(0) {}
};

// Hex for a byte span (pointer + size)
inline std::string u8BytesToHex(const uint8_t *data, size_t size) {
  std::string hex;
  hex.reserve(size * 3);
  for (size_t i = 0; i < size; ++i) {
    fmt::format_to(std::back_inserter(hex), "{:02X} ", data[i]);
  }
  return hex;
}

// Returns IP address in string format
inline std::string getIpAddress() {
  std::string ipAddress;
  struct ifaddrs *ifaddr, *ifa;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    return "";
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    if (ifa->ifa_addr->sa_family == AF_INET) {
      int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (s == 0 && strcmp(host, "127.0.0.1") != 0) {
        ipAddress = host;
        break;
      }
    }
  }

  freeifaddrs(ifaddr);
  return ipAddress;
}

inline std::string getMask() {
  std::string mask;
  struct ifaddrs *ifaddr, *ifa;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    return "";
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    if (ifa->ifa_addr->sa_family == AF_INET) {
      int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (s == 0 && strcmp(host, "127.0.0.1") != 0) {
        // Found the non-loopback interface, now get its netmask
        if (ifa->ifa_netmask != nullptr &&
            ifa->ifa_netmask->sa_family == AF_INET) {
          int mask_s =
              getnameinfo(ifa->ifa_netmask, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
          if (mask_s == 0) {
            mask = host;
            break;
          }
        }
      }
    }
  }

  freeifaddrs(ifaddr);
  return mask;
}

inline std::string getMacAddress() {
  std::string macAddress;
  struct ifaddrs *ifaddr, *ifa;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    return "";
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    if (ifa->ifa_addr->sa_family == AF_INET) {
      int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                          NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
      if (s == 0 && strcmp(host, "127.0.0.1") != 0) {
        macAddress = host;
        break;
      }
    }
  }

  freeifaddrs(ifaddr);
  return macAddress;
}

inline uint16_t calculateCRC16(const uint8_t *data, size_t size) {
  boost::crc_optimal<16, 0x1021, 0xFFFF, 0, true, true> crc_calculator;
  crc_calculator.process_bytes(data, size);
  return static_cast<uint16_t>(crc_calculator.checksum());
}
} // namespace Common