#pragma once

#include <fstream>
#include <spdlog/spdlog.h>
#include <boost/crc.hpp>
#include <vector>
#include <string>
#include <type_traits>
#include <ifaddrs.h>
#include <netdb.h>

#ifndef SPDLOG_LEVEL
    #define SPDLOG_LEVEL "info" // Can be overridden by the Makefile
#endif

// Hex for fixed-width integral scalars (little-endian by default)
template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value || std::is_enum<T>::value>::type>
inline std::string u8ScalarToHex(T value, size_t numBytes, bool littleEndian = true) {
    std::string hex;
    hex.reserve(numBytes * 3);
    for (size_t i = 0; i < numBytes; ++i) {
        size_t shift = littleEndian ? (i * 8) : ((numBytes - 1 - i) * 8);
        uint8_t byteVal = static_cast<uint8_t>((static_cast<uint64_t>(value) >> shift) & 0xFF);
        fmt::format_to(std::back_inserter(hex), "{:02X} ", byteVal);
    }
    return hex;
}

// Hex for a byte span (pointer + size)
inline std::string u8BytesToHex(const uint8_t* data, size_t size) {
    std::string hex;
    hex.reserve(size * 3);
    for (size_t i = 0; i < size; ++i) {
        fmt::format_to(std::back_inserter(hex), "{:02X} ", data[i]);
    }
    return hex;
}

// Returns IP address in string format
inline std::string getIpAddress()
{
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
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
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
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s == 0 && strcmp(host, "127.0.0.1") != 0) {
                macAddress = host;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return macAddress;
}

inline uint16_t calculateCRC16(const std::vector<uint8_t> &data) {
    boost::crc_optimal<16, 0x1021, 0xFFFF, 0, true, true> crc_calculator;
    crc_calculator.process_bytes(data.data(), data.size());
    return static_cast<uint16_t>(crc_calculator.checksum());
  }