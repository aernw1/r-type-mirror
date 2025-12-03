# RFC - R-Type UDP Game Protocol

**Request for Comments:** R-Type-UDP-001
**Category:** Standards Track
**Status:** Stable
**Date:** December 2025
**Authors:** R-Type Network Team

---

## Abstract

This document specifies the UDP-based game protocol for R-Type multiplayer gameplay. The protocol provides low-latency, real-time communication between game clients and an authoritative server, enabling synchronized multiplayer experiences with up to 4 players.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Connection Model](#3-connection-model)
4. [Message Format](#4-message-format)
5. [Packet Types](#5-packet-types)
6. [Game Loop Architecture](#6-game-loop-architecture)
7. [Error Handling & Reliability](#7-error-handling--reliability)
8. [Performance Characteristics](#8-performance-characteristics)
9. [Security Considerations](#9-security-considerations)
10. [Implementation Guide](#10-implementation-guide)

---

## 1. Introduction

### 1.1 Purpose

The R-Type UDP Game Protocol enables:
- Real-time player input transmission
- Authoritative game state synchronization
- Low-latency multiplayer gameplay
- Graceful handling of packet loss

### 1.2 Design Principles

1. **Server Authority**: Server is the single source of truth
2. **Stateless Snapshots**: Each STATE packet contains complete world state
3. **Best-effort Delivery**: Accepts UDP's unreliable nature
4. **Binary Efficiency**: Compact binary encoding for minimal bandwidth
5. **Tick-based Simulation**: Fixed 60 Hz server tick rate

### 1.3 Prerequisites

This protocol assumes:
- Players have completed TCP lobby phase (see RFC_TCP_LOBBY.md)
- Each player has a unique 64-bit hash from lobby
- Server has been initialized with player list
- Clients know server IP and UDP port

### 1.4 Terminology

- **Tick**: A single iteration of the game loop (1/60 second)
- **Snapshot**: A STATE packet containing all entity data
- **Sequence Number**: Monotonically increasing counter for INPUT packets
- **Entity**: Any game object (player, enemy, bullet, power-up)

---

## 2. Protocol Overview

### 2.1 Transport Protocol

The game protocol operates over **UDP (User Datagram Protocol)** to provide:
- Minimal latency (no connection overhead)
- No head-of-line blocking
- Tolerance for packet loss

**Default Port:** 4243 (configurable, assigned by lobby)

### 2.2 Communication Pattern

```
Client A (60Hz)                    Server                     Client B (60Hz)
     │                               │                               │
     │────── INPUT (seq=N) ─────────►│                               │
     │                               │◄────── INPUT (seq=M) ─────────│
     │                               │                               │
     │                               │  Server Logic (60Hz):          │
     │                               │  - Apply inputs                │
     │                               │  - Update physics              │
     │                               │  - Spawn entities              │
     │                               │  - Check collisions            │
     │                               │                               │
     │◄────── STATE (tick=T) ────────┤────── STATE (tick=T) ────────►│
     │      (all entities)           │      (all entities)           │
     │                               │                               │
```

**Frequencies:**
- Client → Server: **60 Hz** (INPUT packets)
- Server → Clients: **20 Hz** (STATE packets, broadcast)

---

## 3. Connection Model

### 3.1 Initial Handshake

Unlike TCP, UDP has no built-in connection. This protocol implements an application-level handshake:

```
Client                                  Server
  |                                       |
  |──────── HELLO (playerHash) ─────────►|
  |                                       |
  |                      [Server validates hash against expected players]
  |                                       |
  |◄─────── WELCOME (playersConnected) ──|
  |                                       |
  |──────── INPUT (first input) ────────►|
  |                                       |
  |◄──────── STATE (snapshot) ───────────|
  |                                       |
```

**Timeout:** If server doesn't receive HELLO within 10 seconds, connection attempt is abandoned.

### 3.2 Connection Maintenance

**Client Responsibilities:**
- Send INPUT packets every frame (~60 Hz)
- Consider disconnected if no STATE received for 5 seconds

**Server Responsibilities:**
- Track last INPUT timestamp per player
- Consider player disconnected if no INPUT for 5 seconds
- Remove disconnected player's entity from game

### 3.3 Graceful Disconnect

**Client:**
```
Client sends DISCONNECT packet
Client closes UDP socket
```

**Server:**
```
Server receives DISCONNECT
Server removes player entity
Server continues broadcasting STATE (player entity gone)
```

---

## 4. Message Format

### 4.1 Common Header

All packets start with a 1-byte type identifier:

```
 0               1
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Packet Type  |   Payload...
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

### 4.2 Byte Order

- **Integers**: Little-endian
- **Floats**: IEEE 754 single-precision (little-endian)

### 4.3 Packet Structure Alignment

All packet structures use `#pragma pack(1)` (no padding) for consistency across platforms.

---

## 5. Packet Types

### 5.1 Packet Type Enumeration

| Value | Name       | Direction     | Frequency | Description                    |
|-------|------------|---------------|-----------|--------------------------------|
| 0x00  | HELLO      | Client→Server | Once      | Initial connection request     |
| 0x01  | WELCOME    | Server→Client | Once      | Connection accepted            |
| 0x02  | INPUT      | Client→Server | ~60 Hz    | Player input commands          |
| 0x03  | STATE      | Server→Client | ~20 Hz    | Complete game state snapshot   |
| 0x04  | PING       | Bidirectional | ~1 Hz     | Keepalive / RTT measurement    |
| 0x05  | PONG       | Bidirectional | Response  | Keepalive response             |
| 0x06  | DISCONNECT | Client→Server | Once      | Graceful disconnect            |

---

### 5.2 HELLO (0x00)

**Direction:** Client → Server
**Purpose:** Initiate UDP connection with hash from lobby
**Size:** 41 bytes

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     0x00      |               Player Hash (8 bytes)           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Hash (continued)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Name (32 bytes)                      |
|                      UTF-8, null-terminated                   |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Type` (1 byte): 0x00
- `Player Hash` (8 bytes): Unique identifier from lobby
- `Player Name` (32 bytes): Display name (null-terminated UTF-8)

**Server Validation:**
- Hash MUST match an expected player from lobby
- Server MAY reject duplicate HELLO from same hash

---

### 5.3 WELCOME (0x01)

**Direction:** Server → Client
**Purpose:** Acknowledge connection and provide initial state
**Size:** 6 bytes

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     0x01      | Players Conn. |        Server Tick            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Server Tick  |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Type` (1 byte): 0x01
- `Players Connected` (1 byte): Number of players currently connected
- `Server Tick` (4 bytes): Current server tick number

**Semantics:**
- Client SHOULD start sending INPUT packets upon receiving WELCOME
- Client initializes local tick counter to server tick

---

### 5.4 INPUT (0x02)

**Direction:** Client → Server
**Purpose:** Transmit player input commands
**Size:** 18 bytes
**Frequency:** ~60 Hz (every client frame)

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     0x02      |           Sequence Number (4 bytes)           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                     Player Hash (8 bytes)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Hash (continued)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Inputs     |              Timestamp (4 bytes)              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Timestamp   |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Type` (1 byte): 0x02
- `Sequence` (4 bytes): Monotonically increasing counter (starts at 0)
- `Player Hash` (8 bytes): Player identifier
- `Inputs` (1 byte): Bitfield of input flags (see below)
- `Timestamp` (4 bytes): Client timestamp in milliseconds

**Input Flags (Bitfield):**

| Bit | Value | Name    | Description       |
|-----|-------|---------|-------------------|
| 0   | 0x01  | UP      | Move up           |
| 1   | 0x02  | DOWN    | Move down         |
| 2   | 0x04  | LEFT    | Move left         |
| 3   | 0x08  | RIGHT   | Move right        |
| 4   | 0x10  | SHOOT   | Fire weapon       |
| 5-7 | -     | Unused  | Reserved (0)      |

**Example:**
```
UP + SHOOT = 0x01 | 0x10 = 0x11
LEFT + RIGHT + DOWN = 0x04 | 0x08 | 0x02 = 0x0E
```

**Server Processing:**
- Server applies inputs immediately to player entity
- Sequence number used to detect duplicates/out-of-order packets
- Timestamp used for lag compensation (optional)

---

### 5.5 STATE (0x03)

**Direction:** Server → Client (broadcast to all)
**Purpose:** Synchronize complete game state
**Size:** Variable (11 + 30×N bytes, where N = entity count)
**Frequency:** ~20 Hz (every 3rd tick)

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     0x03      |               Server Tick (4 bytes)           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Timestamp (4 bytes)                         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Entity Count (2 bytes)    |   EntityState[0] ...          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      EntityState[1] ...                       |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Header Fields:**
- `Type` (1 byte): 0x03
- `Tick` (4 bytes): Server tick number
- `Timestamp` (4 bytes): Server timestamp in milliseconds
- `Entity Count` (2 bytes): Number of entities following (0-256)

**Followed by N × EntityState:**

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Entity ID (4 bytes)                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Entity Type  |              X Position (float, 4 bytes)      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Y Position (float, 4 bytes)                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Velocity X (float, 4 bytes)                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                  Velocity Y (float, 4 bytes)                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|    Health     |     Flags     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**EntityState Structure (30 bytes):**
- `Entity ID` (4 bytes): Unique identifier for this entity
- `Entity Type` (1 byte): Type identifier (see below)
- `X Position` (4 bytes, float): Horizontal position (pixels)
- `Y Position` (4 bytes, float): Vertical position (pixels)
- `Velocity X` (4 bytes, float): Horizontal velocity (pixels/second)
- `Velocity Y` (4 bytes, float): Vertical velocity (pixels/second)
- `Health` (1 byte): Current health points (0-255)
- `Flags` (1 byte): Entity-specific flags

**Entity Type Values:**

| Value | Type    | Description                |
|-------|---------|----------------------------|
| 0x01  | PLAYER  | Player spaceship           |
| 0x02  | ENEMY   | Enemy spaceship            |
| 0x03  | BULLET  | Projectile                 |
| 0x04  | POWERUP | Power-up item              |

**Flags Interpretation (examples):**
- PLAYER: 0x01 = invulnerable, 0x02 = powered-up
- ENEMY: 0x01 = boss, 0x02 = elite
- BULLET: 0x01 = player-owned, 0x02 = enemy-owned

**Client Processing:**
- Replace local entity list with received state
- Use tick number to reject old snapshots (tick <= lastReceivedTick)
- Interpolate between snapshots for smooth rendering (optional)

---

### 5.6 PING (0x04) / PONG (0x05)

**Direction:** Bidirectional
**Purpose:** Measure round-trip time (RTT) and keepalive
**Size:** 5 bytes
**Frequency:** ~1 Hz (optional, for diagnostics)

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   0x04/0x05   |              Timestamp (4 bytes)              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Timestamp   |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Type` (1 byte): 0x04 (PING) or 0x05 (PONG)
- `Timestamp` (4 bytes): Sender's timestamp in milliseconds

**RTT Calculation:**
```
Sender sends PING with timestamp T1
Receiver echoes PONG with same timestamp T1
Sender receives PONG at time T2
RTT = T2 - T1
```

**Note:** PING/PONG is OPTIONAL. INPUT packets serve as implicit keepalive.

---

### 5.7 DISCONNECT (0x06)

**Direction:** Client → Server
**Purpose:** Graceful disconnect notification
**Size:** 1 byte

```
 0
 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+
|     0x06      |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Type` (1 byte): 0x06

**Semantics:**
- Client SHOULD send before closing socket
- Server removes player entity from next STATE snapshot
- No acknowledgment sent (fire-and-forget)

---

## 6. Game Loop Architecture

### 6.1 Server Loop (Authoritative)

```cpp
const float TICK_RATE = 1.0f / 60.0f;  // 60 Hz
const float SNAPSHOT_RATE = 1.0f / 20.0f;  // 20 Hz

while (running) {
    auto frameStart = now();

    // 1. Process all incoming INPUT packets (non-blocking)
    ProcessIncomingPackets();

    // 2. Update game simulation (authoritative)
    UpdateGameLogic(TICK_RATE);
    currentTick++;

    // 3. Send STATE snapshots every 3rd tick (20 Hz)
    if (currentTick % 3 == 0) {
        SendStateSnapshots();
    }

    // 4. Sleep to maintain 60 Hz
    auto elapsed = now() - frameStart;
    Sleep(TICK_RATE - elapsed);
}
```

**Update Order:**
```
UpdateGameLogic():
  1. Apply player inputs to velocities
  2. Update entity positions (physics)
  3. Spawn new enemies (time-based)
  4. Update bullets (lifetime, out-of-bounds)
  5. Check collisions (bullets vs enemies, players vs enemies)
  6. Update health / destroy entities
  7. Cleanup dead entities
```

### 6.2 Client Loop

```cpp
const float FRAME_RATE = 1.0f / 60.0f;  // 60 FPS

while (running) {
    auto frameStart = now();

    // 1. Gather user inputs
    uint8_t inputs = GatherInputs();  // Keyboard/gamepad

    // 2. Send INPUT to server
    SendInput(inputs);
    inputSequence++;

    // 3. Receive STATE packets (non-blocking)
    ReceivePackets();

    // 4. Render game (using last received STATE)
    Render();

    // 5. Sleep to maintain 60 FPS
    auto elapsed = now() - frameStart;
    Sleep(FRAME_RATE - elapsed);
}
```

---

## 7. Error Handling & Reliability

### 7.1 Packet Loss Handling

**Philosophy:** Accept loss, rely on next snapshot

| Packet Lost | Impact | Mitigation |
|-------------|--------|------------|
| INPUT       | Server uses last known velocity | Client re-sends inputs next frame |
| STATE       | Client renders stale state briefly | Next STATE arrives (~50ms later) |
| HELLO       | Connection fails | Client retries with timeout |
| WELCOME     | Client doesn't start | Client retries HELLO |

**Key Insight:** STATE packets are complete snapshots (not deltas), so a single lost packet doesn't corrupt state.

### 7.2 Out-of-Order Delivery

**INPUT Packets:**
```cpp
if (inputPacket.sequence <= lastProcessedSequence) {
    discard();  // Old packet, ignore
}
```

**STATE Packets:**
```cpp
if (statePacket.tick <= lastReceivedTick) {
    discard();  // Old snapshot, ignore
}
```

### 7.3 Duplicate Packets

UDP may deliver duplicates. Use sequence/tick numbers to detect:

```cpp
if (packet.sequence == lastProcessedSequence) {
    discard();  // Duplicate
}
```

### 7.4 Malformed Packets

**Server/Client:**
```cpp
if (packetSize < expectedSize) {
    log("Malformed packet");
    discard();
}

if (entityCount > MAX_ENTITIES) {
    log("Invalid entity count");
    discard();
}
```

**Action:** Silently discard invalid packets (no error response).

### 7.5 Player Timeout

**Timeout:** 5 seconds without INPUT (server) or STATE (client)

**Server Action:**
```cpp
if (now() - player.lastInputTime > 5.0s) {
    RemovePlayer(player);
    // Player entity disappears in next STATE
}
```

**Client Action:**
```cpp
if (now() - lastStateTime > 5.0s) {
    ShowDisconnectMessage();
    ReturnToLobby();
}
```

---

## 8. Performance Characteristics

### 8.1 Bandwidth Usage

**Per Player (Downstream):**
- STATE packets: (11 + 30×N) bytes × 20 Hz
- Example (N=20 entities): (11 + 600) × 20 = **12,220 bytes/sec = 97.7 Kbps**

**Per Player (Upstream):**
- INPUT packets: 18 bytes × 60 Hz = **1,080 bytes/sec = 8.6 Kbps**

**Total per Player:** ~106 Kbps (downstream) + 8.6 Kbps (upstream)

**4-Player Session (Server):**
- Total upstream: 4 × 1,080 = 4,320 bytes/sec = **34.6 Kbps**
- Total downstream: 12,220 bytes/sec = **97.7 Kbps** (broadcast, same data to all)

### 8.2 Latency

**Best Case (LAN):**
- Input latency: ~16ms (1 frame) + network RTT (~1-5ms) = **17-21ms**
- State latency: ~50ms (3 ticks @ 60Hz) + network RTT = **51-55ms**

**Typical (Internet, <50ms RTT):**
- Input latency: ~16ms + 25ms RTT = **41ms**
- State latency: ~50ms + 25ms RTT = **75ms**

### 8.3 CPU Usage

**Server (4 players, 20 entities):**
- Input processing: Negligible
- Game logic: ~1-2ms per tick
- State serialization: ~0.5ms per snapshot
- Total: **~2.5ms / 16.67ms tick = 15% CPU**

---

## 9. Security Considerations

### 9.1 Authentication

**Current Implementation:**
- Player hash from lobby serves as authentication token
- No encryption or signature

**Limitations:**
- Hash may be intercepted/replayed
- No protection against man-in-the-middle

**Recommendations for Production:**
- Implement DTLS (Datagram TLS) for encryption
- Use signed packets with HMAC
- Rotate hashes per session

### 9.2 Cheating Prevention

**Vulnerabilities:**

| Cheat Type | Current Protection | Recommended |
|------------|-------------------|-------------|
| Speed hack | None (trusts client inputs) | Server-side velocity validation |
| Teleport | None | Server-side position validation |
| Aimbot | None (client-side) | Server-side hit validation |
| Wallhack | Not applicable (2D) | N/A |

**Server-side Validation Example:**
```cpp
// Reject impossible movement
float maxSpeed = 300.0f;  // pixels/second
float distance = sqrt(dx*dx + dy*dy);
if (distance > maxSpeed * dt) {
    RejectInput();  // Suspected speed hack
}
```

### 9.3 Denial of Service

**Vulnerability:** Flood server with INPUT packets

**Mitigation:**
```cpp
// Rate limit: max 100 packets/second per player
if (player.inputRate > 100) {
    DropPacket();
}
```

---

## 10. Implementation Guide

### 10.1 Server Implementation

**File:** `libs/network/src/GameServer.cpp`

```cpp
#include "GameServer.hpp"

network::GameServer server(4243, expectedPlayers);
server.Run();  // Blocks until game ends
```

**Key Methods:**
- `WaitForAllPlayers()` - Waits for HELLO from all
- `ProcessIncomingPackets()` - Handles INPUT, PING
- `UpdateGameLogic(dt)` - Game simulation
- `SendStateSnapshots()` - Broadcasts STATE

### 10.2 Client Implementation

**File:** Custom implementation (example: `RealGameClient.cpp`)

```cpp
#include "GameClient.hpp"

class RealGameClient : public GameClient {
    // Override GetInputs() with real keyboard/gamepad
    // Override OnStateReceived() to update visuals
};

RealGameClient client("127.0.0.1", 4243, playerInfo);
client.ConnectToServer();
client.Run();
```

### 10.3 Testing

**Headless Test (10 seconds):**
```bash
cd build
make test_udp_protocol
./bin/test_udp_protocol
```

**Expected Output:**
```
🎉 ALL TESTS PASSED! UDP protocol working correctly.
✅ Protocol validation:
  - Authoritative server ✅
  - UDP communication ✅
  - Binary protocol ✅
  - Client inputs → Server ✅
  - Server state → Clients ✅
  - No crashes ✅
```

### 10.4 Integration with Lobby

**Lobby → Game Transition:**

1. **Lobby Phase (TCP):**
   - Players connect, ready up
   - `GAME_START` packet sent with UDP port

2. **Game Initialization:**
   ```cpp
   // Server
   GameServer udpServer(udpPort, playersFromLobby);
   std::thread([&]() { udpServer.Run(); }).detach();

   // Client receives GAME_START
   auto gameInfo = deserialize(gameStartPacket);
   RealGameClient gameClient(gameInfo.serverIp, gameInfo.udpPort, myPlayerInfo);
   gameClient.ConnectToServer();
   ```

3. **Game Phase (UDP):**
   - Clients send INPUT, receive STATE
   - TCP lobby connection closed

---

## Appendix A: Packet Size Reference

| Packet    | Size (bytes) | Frequency | Bandwidth      |
|-----------|--------------|-----------|----------------|
| HELLO     | 41           | Once      | N/A            |
| WELCOME   | 6            | Once      | N/A            |
| INPUT     | 18           | 60 Hz     | 1,080 B/s      |
| STATE     | 11 + 30×N    | 20 Hz     | ~12,220 B/s    |
| PING      | 5            | 1 Hz      | 5 B/s          |
| PONG      | 5            | 1 Hz      | 5 B/s          |
| DISCONNECT| 1            | Once      | N/A            |

---

## Appendix B: Entity ID Assignment

**Server Responsibility:**
- Assign monotonically increasing IDs
- ID 0 is invalid (NULL_ENTITY)
- IDs never reused within a session

```cpp
uint32_t nextEntityId = 1;

uint32_t AllocateEntityId() {
    return nextEntityId++;
}
```

---

## Appendix C: Example Session

```
[Time 0.000s] Client Alice connects
C→S: HELLO (hash=12345, name="Alice")
S→C: WELCOME (players=1, tick=0)

[Time 0.500s] Client Bob connects
C→S: HELLO (hash=67890, name="Bob")
S→C: WELCOME (players=2, tick=30)

[Time 0.516s] Server sends first STATE
S→C: STATE (tick=3, entities=[Alice, Bob])

[Time 1.000s] Alice presses UP + SHOOT
C→S: INPUT (seq=60, inputs=0x11, ts=1000)

[Time 1.016s] Server updates, Alice moves up, spawns bullet
S→C: STATE (tick=63, entities=[Alice, Bob, Bullet1])

[Time 2.000s] Enemy spawns
S→C: STATE (tick=123, entities=[Alice, Bob, Bullet1, Enemy1])

[Time 10.000s] Alice quits
C→S: DISCONNECT
S→C: STATE (tick=603, entities=[Bob, Enemy1])  // Alice gone
```

---

## Appendix D: Change Log

**Version 1.0 (December 2025)**
- Initial specification
- Defined all packet types
- Specified game loop architecture
- Added performance metrics
- Security considerations

---

## References

- **RFC 768** - User Datagram Protocol (UDP)
- **RFC 2119** - Key words for RFCs
- **IEEE 754** - Floating-Point Arithmetic
- **R-Type TCP Lobby Protocol** - See `RFC_TCP_LOBBY.md`

---

**End of Document**
