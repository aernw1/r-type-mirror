# R-Type

A multiplayer horizontal shoot'em up (Shmup) game implemented in C++ with a client-server architecture.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Documentation](#documentation)
- [Requirements](#requirements)
- [Building](#building)
- [Running the Game](#running-the-game)
- [Project Structure](#project-structure)
- [Authors](#authors)
- [License](#license)

---

## Overview

R-Type is a classic arcade-style horizontal scrolling shooter game reimagined as a modern multiplayer experience. The game supports up to 4 players simultaneously and features an authoritative server architecture for fair and synchronized gameplay.

### Key Features

- **Multiplayer**: Up to 4 players can play together
- **Authoritative Server**: Server maintains game state for fair gameplay
- **Binary Protocol**: Efficient UDP protocol for real-time gameplay
- **Delta Compression**: Optimized bandwidth with delta encoding and LZ4 compression
- **Modular Engine**: Reusable game engine with ECS architecture
- **Cross-Platform**: Supports Linux (primary) and Windows

---

## Features

### Gameplay
- Horizontal scrolling star-field background
- Multiple enemy types with unique behaviors
- Boss battles at the end of each level
- Power-ups: Spread shot, Laser beam, Shield, Speed boost
- Force Pod companion
- Multiple levels

### Technical
- Entity Component System (ECS) architecture
- Plugin-based module system
- TCP lobby for room management
- UDP game protocol for low-latency gameplay
- Client-side prediction with server reconciliation
- Delta encoding for bandwidth optimization
- LZ4 compression support

---

## Documentation

### Protocol Documentation (RFC)
- [UDP Game Protocol](docs/RFC_UDP_PROTOCOL.md) - Real-time game communication protocol
- [TCP Lobby Protocol](docs/RFC_TCP_PROTOCOL.md) - Lobby and room management protocol

### Engine Documentation
- [Engine & ECS Documentation](libs/engine/README.md) - Game engine architecture and ECS system

### Demos
- [Breakout Game](breakout/README.md) - Proof of concept demonstrating engine reusability

---

## Requirements

### Build Requirements
- **C++17** compatible compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **CMake** 3.16 or higher
- **vcpkg** (included as submodule)

### Dependencies (managed by vcpkg)
- SFML 2.6.2 (graphics, audio, window)
- Asio (networking)
- nlohmann-json (JSON parsing)
- LZ4 (compression)

---

## Building

### Clone the Repository

```bash
git clone --recursive https://github.com/aernw1/r-type-mirror.git
cd r-type-mirror
```

### Linux Build

```bash
# Bootstrap vcpkg (first time only)
./vcpkg/bootstrap-vcpkg.sh

# Configure and build
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build . --parallel
```

### Windows Build (MSVC)

```powershell
# Bootstrap vcpkg (first time only)
.\vcpkg\bootstrap-vcpkg.bat

# Configure and build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release
```

### Build Output

After building, the following executables are available in `build/bin/`:

| Binary | Description |
|--------|-------------|
| `r-type_server` | Game server (lobby + game) |
| `r-type_client` | Game client |
| `breakout` | Breakout demo game |

---

## Running the Game

### Start the Server

```bash
cd build
./bin/r-type_server
```

The server will:
1. Start the lobby server on port **7777** (TCP)
2. Dynamically allocate UDP ports for game sessions

### Start a Client

```bash
cd build
./bin/r-type_client
```

### Gameplay Instructions

1. Launch the client
2. Enter your player name
3. Create a room or join an existing one
4. Wait for other players (1-4 players)
5. All players must click "Ready"
6. Host starts the game

### Controls

| Key | Action |
|-----|--------|
| Arrow Keys | Move ship |
| Space | Fire weapon |
| Escape | Pause / Menu |

---

## Project Structure

```
r-type-mirror/
├── README.md                 # This file
├── CMakeLists.txt            # Root CMake configuration
├── vcpkg.json                # vcpkg dependencies
├── vcpkg/                    # vcpkg package manager (submodule)
├── assets/                   # Game assets (sprites, sounds, levels)
│   ├── sprites/
│   ├── sounds/
│   └── levels/
├── client/                   # Game client application
│   ├── include/
│   └── src/
├── server/                   # Game server application
│   └── src/
├── libs/                     # Shared libraries
│   ├── engine/               # Game engine (ECS, Core)
│   │   ├── include/
│   │   │   ├── Core/         # Engine core (Logger, Module system)
│   │   │   └── ECS/          # Entity Component System
│   │   ├── src/
│   │   └── README.md         # Engine documentation
│   ├── network/              # Network library
│   │   ├── include/
│   │   │   ├── Protocol.hpp  # Binary protocol definitions
│   │   │   └── ...
│   │   └── src/
│   ├── sfml_renderer/        # SFML rendering module
│   └── sfml_audio/           # SFML audio module
├── breakout/                 # Breakout demo game
│   ├── src/
│   └── README.md
├── docs/                     # Documentation
│   ├── RFC_UDP_PROTOCOL.md   # UDP protocol specification
│   └── RFC_TCP_PROTOCOL.md   # TCP protocol specification
└── tests/                    # Unit tests
```

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                         CLIENT                              │
│  ┌─────────┐  ┌──────────┐  ┌──────────┐  ┌─────────────┐  │
│  │ Input   │  │ Renderer │  │  Audio   │  │   Network   │  │
│  │ System  │  │  (SFML)  │  │  (SFML)  │  │   Client    │  │
│  └────┬────┘  └────┬─────┘  └────┬─────┘  └──────┬──────┘  │
│       │            │             │               │          │
│       └────────────┴─────────────┴───────────────┘          │
│                           │                                 │
│                    ┌──────┴──────┐                          │
│                    │   ECS       │                          │
│                    │  Registry   │                          │
│                    └─────────────┘                          │
└───────────────────────────┬─────────────────────────────────┘
                            │ UDP (Game)
                            │ TCP (Lobby)
┌───────────────────────────┼─────────────────────────────────┐
│                           │                                 │
│                    ┌──────┴──────┐                          │
│                    │   ECS       │                          │
│                    │  Registry   │                          │
│                    └──────┬──────┘                          │
│                           │                                 │
│  ┌─────────────┐  ┌───────┴───────┐  ┌──────────────────┐  │
│  │   Lobby     │  │    Game       │  │     Systems      │  │
│  │   Server    │  │    Server     │  │  (Collision,     │  │
│  │   (TCP)     │  │    (UDP)      │  │   Movement...)   │  │
│  └─────────────┘  └───────────────┘  └──────────────────┘  │
│                         SERVER                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Network Architecture

### TCP Lobby (Port 7777)
- Room creation and management
- Player session management
- Ready state synchronization
- Game start coordination

### UDP Game (Dynamic Port)
- Real-time game state synchronization
- Player input transmission
- Delta encoding for bandwidth optimization
- LZ4 compression for large packets

See [RFC_UDP_PROTOCOL.md](docs/RFC_UDP_PROTOCOL.md) and [RFC_TCP_PROTOCOL.md](docs/RFC_TCP_PROTOCOL.md) for detailed protocol specifications.

---

## Authors

EPITECH Project - 2025

---

## License

This project is part of the EPITECH curriculum.
