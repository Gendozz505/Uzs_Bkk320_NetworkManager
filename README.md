# Uzs Bkk320 Network Manager

A C++ network management application designed for BKK320 devices, implementing both TCP and UDP communication protocols with custom message handling.

## Overview

This application provides a robust network management solution for BKK320 devices, featuring:

- **Dual Protocol Support**: Both TCP and UDP communication
- **Custom Message Protocol**: Binary message format with CRC16 validation
- **Async I/O**: Built on Boost.Asio for high-performance networking
- **Signal-Slot Architecture**: Event-driven design using Boost.Signals2
- **Cross-Platform Build**: Supports both host (x86_64) and target (ARM64) compilation
- **Configurable Logging**: Multi-level logging with spdlog

## Architecture

### Core Components

- **TcpAcceptor/TcpSession**: Handle TCP connections and sessions
- **UdpSocket**: Manages UDP communication
- **MessageParser**: Parses incoming binary messages with validation
- **MessageManager**: Processes business logic and command handling
- **Bkk32Info**: Manages device configuration and serial number retrieval
- **SpdlogManager**: Centralized logging configuration

### Message Protocol

The application implements a custom binary protocol:

```
[CMD:1][SERIAL:2][STATUS:1][DATALEN:4][PAYLOAD:N][CRC16:2]
```

- **CMD**: Command byte (e.g., 0xF6 for IP request)
- **SERIAL**: Device serial number
- **STATUS**: Status/error code
- **DATALEN**: Payload length
- **PAYLOAD**: Variable-length data
- **CRC16**: Checksum for integrity validation

## Building

### Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 5+)
- Boost libraries (system, program_options, thread)
- nlohmann/json
- spdlog
- fmt

### Build Commands

```bash
# Host build (development)
make host

# Target build (ARM64 deployment)
make target

# Run the application
make run

# Clean build artifacts
make clean
```

### Build Options

- **Host Build**: Debug symbols, trace logging, dynamic linking
- **Target Build**: Optimized, info logging, static linking for embedded deployment

## Usage

### Command Line Options

```bash
./uzsBkk320NetworkManager [options]

Options:
  -h, --help                    Display help message
  -p, --port <port>            Port to listen on (default: 30720)
  -m, --main-cfg-file <file>   Path to main configuration file
```

### Example

```bash
# Run with custom port and config file
./uzsBkk320NetworkManager -p 8080 -m /path/to/MainCfg.json

# Run with default settings
./uzsBkk320NetworkManager
```

## Configuration

The application uses a JSON configuration file (`MainCfg.json`) containing:

- Device serial number

## Current Features

- ✅ UDP message handling with IP request/response
- ✅ TCP connection acceptance
- ✅ CRC16 message validation
- ✅ Signal-based event handling
- ✅ Graceful shutdown with signal handling
- ✅ Cross-platform build system
- ✅ Configurable logging levels

## Future Tasks

1. **TCP Message Handler Implementation**
   - Complete TCP message processing pipeline
   - Integrate TCP sessions with MessageParser/MessageManager
   - Add TCP-specific command handling
   - Implement session management and cleanup

2. **Per-Module Spdlog Objects**
   - Create dedicated logger instances for each component
   - Implement module-specific log formatting
   - Add log rotation and file management
   - Enable runtime log level changes per module