# RFC - R-Type TCP Lobby Protocol

**Request for Comments:** R-Type-TCP-001
**Category:** Standards Track
**Status:** Stable
**Date:** December 2025
**Authors:** R-Type Network Team

---

## Abstract

This document specifies the TCP-based lobby protocol for R-Type multiplayer game. The protocol enables player matchmaking, room management, and game session initialization before transitioning to the UDP-based game protocol.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Connection Model](#3-connection-model)
4. [Message Format](#4-message-format)
5. [Packet Types](#5-packet-types)
6. [State Machine](#6-state-machine)
7. [Error Handling](#7-error-handling)
8. [Security Considerations](#8-security-considerations)
9. [References](#9-references)

---

## 1. Introduction

### 1.1 Purpose

The R-Type TCP Lobby Protocol provides a reliable mechanism for:
- Player authentication and identification
- Pre-game lobby management
- Ready-check coordination
- Synchronized game session initialization

### 1.2 Terminology

- **Lobby Server**: TCP server managing player connections and game sessions
- **Lobby Client**: TCP client representing a player in the lobby
- **Player Hash**: 64-bit unique identifier assigned to each player
- **Game Session**: A multiplayer game instance initiated after lobby phase

### 1.3 Conventions

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in RFC 2119.

---

## 2. Protocol Overview

### 2.1 Transport Protocol

The lobby protocol operates over TCP (Transmission Control Protocol) to ensure:
- Reliable, ordered delivery of messages
- Connection-oriented communication
- Flow control and congestion management

**Default Port:** 4242 (configurable)

### 2.2 Message Exchange Pattern

```
Client                                  Server
  |                                       |
  |──────── TCP Connect ────────────────►|
  |                                       |
  |──────── CONNECT_REQ ────────────────►|
  |                 (player name)         |
  |                                       |
  |◄─────── CONNECT_ACK ─────────────────|
  |          (player hash, number)        |
  |                                       |
  |◄─────── PLAYER_JOIN ──────────────────| (broadcast)
  |              (new player)             |
  |                                       |
  |──────── READY_REQ ──────────────────►|
  |                                       |
  |◄─────── PLAYER_READY ─────────────────| (broadcast)
  |                                       |
  |──────── START_REQ ──────────────────►|
  |         (only if host/all ready)      |
  |                                       |
  |◄─────── COUNTDOWN ────────────────────| (broadcast)
  |           (5, 4, 3, 2, 1...)          |
  |                                       |
  |◄─────── GAME_START ────────────────────| (broadcast)
  |        (UDP server info, spawn data)  |
  |                                       |
  |──────── TCP Disconnect ──────────────►|
  |     (transition to UDP game)          |
```

---

## 3. Connection Model

### 3.1 Connection Establishment

1. Client establishes TCP connection to server
2. Client sends `CONNECT_REQ` with player name
3. Server validates and assigns unique player hash
4. Server responds with `CONNECT_ACK`
5. Server broadcasts `PLAYER_JOIN` to all existing players

### 3.2 Connection Maintenance

- TCP keep-alive SHOULD be enabled
- Idle timeout: 30 seconds (configurable)
- Heartbeat: Not required (TCP handles connection state)

### 3.3 Connection Termination

**Graceful:**
- Client sends `DISCONNECT` packet
- Server broadcasts `PLAYER_LEFT`
- TCP connection closed

**Ungraceful:**
- TCP timeout or RST
- Server detects disconnect
- Server broadcasts `PLAYER_LEFT`

---

## 4. Message Format

### 4.1 Binary Format

All messages follow this structure:

```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Packet Type  |        Payload Length (2 bytes, LE)          |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                       Payload (variable)                      |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Packet Type` (1 byte): Message type identifier
- `Payload Length` (2 bytes, little-endian): Length of payload in bytes
- `Payload` (variable): Message-specific data

### 4.2 Encoding

- **Integers**: Little-endian byte order
- **Strings**: UTF-8 encoding, null-terminated
- **Booleans**: 1 byte (0x00 = false, 0x01 = true)

---

## 5. Packet Types

### 5.1 Packet Type Enumeration

| Value | Name         | Direction     | Description                    |
|-------|--------------|---------------|--------------------------------|
| 0x10  | CONNECT_REQ  | Client→Server | Connection request             |
| 0x11  | CONNECT_ACK  | Server→Client | Connection acknowledgment      |
| 0x12  | PLAYER_JOIN  | Server→Client | New player joined (broadcast)  |
| 0x13  | READY_REQ    | Client→Server | Player ready request           |
| 0x14  | PLAYER_READY | Server→Client | Player ready status (broadcast)|
| 0x15  | START_REQ    | Client→Server | Game start request             |
| 0x16  | GAME_START   | Server→Client | Game starting (broadcast)      |
| 0x17  | DISCONNECT   | Client→Server | Graceful disconnect            |
| 0x18  | PLAYER_LEFT  | Server→Client | Player left (broadcast)        |
| 0x19  | COUNTDOWN    | Server→Client | Countdown tick (broadcast)     |
| 0x1F  | ERROR_MSG    | Server→Client | Error notification             |

---

### 5.2 CONNECT_REQ (0x10)

**Direction:** Client → Server
**Purpose:** Initiate connection and provide player name

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Name (32 bytes)                      |
|                       UTF-8, null-terminated                  |
|                             ...                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Constraints:**
- Player name MUST be 1-31 characters (plus null terminator)
- Player name MUST NOT contain control characters
- Player name SHOULD be unique (server MAY enforce)

**Example:**
```
Hex: 10 20 00 41 6C 69 63 65 00 ...
     │   │   └─────────────────── "Alice" (+ padding)
     │   └─────────────────────── Payload length: 32
     └─────────────────────────── Type: CONNECT_REQ
```

---

### 5.3 CONNECT_ACK (0x11)

**Direction:** Server → Client
**Purpose:** Acknowledge connection and assign player identity

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|               Player Hash (8 bytes, little-endian)            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Hash (continued)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Player Number |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Player Hash` (8 bytes): Unique 64-bit identifier for this player session
- `Player Number` (1 byte): Slot number (1-4)

**Semantics:**
- Player hash MUST be unique per session
- Player hash is used for UDP game protocol authentication
- Player number determines spawn position and UI slot

---

### 5.4 PLAYER_JOIN (0x12)

**Direction:** Server → Client (broadcast)
**Purpose:** Notify all clients of new player

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Player Number |               Player Hash (8 bytes)           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Hash (continued)                     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player Name (32 bytes)                      |
|                       UTF-8, null-terminated                  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Ready     |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Player Number` (1 byte): Slot number (1-4)
- `Player Hash` (8 bytes): Unique identifier
- `Player Name` (32 bytes): Display name
- `Ready` (1 byte): Ready status (0x00 or 0x01)

**Semantics:**
- Sent to all connected clients when a new player joins
- Includes the newly joined player's information

---

### 5.5 READY_REQ (0x13)

**Direction:** Client → Server
**Purpose:** Toggle ready status

**Payload:**
```
 0               1
 0 1 2 3 4 5 6 7 8
+-+-+-+-+-+-+-+-+
|     Ready     |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Ready` (1 byte): Desired ready state (0x00 = not ready, 0x01 = ready)

**Semantics:**
- Client can toggle ready status any time before game starts
- Server MUST validate player is connected before updating

---

### 5.6 PLAYER_READY (0x14)

**Direction:** Server → Client (broadcast)
**Purpose:** Broadcast player ready status change

**Payload:**
```
 0               1               2
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Player Number |     Ready     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Player Number` (1 byte): Player who changed status
- `Ready` (1 byte): New ready state

---

### 5.7 START_REQ (0x15)

**Direction:** Client → Server
**Purpose:** Request game start

**Payload:** Empty (no payload)

**Server Validation:**
- All players MUST be ready
- Minimum player count MUST be reached (configurable, default: 2)
- Game MUST NOT have already started

**Response:**
- If valid: Server sends `COUNTDOWN` (0x19)
- If invalid: Server sends `ERROR_MSG` (0x1F)

---

### 5.8 GAME_START (0x16)

**Direction:** Server → Client (broadcast)
**Purpose:** Initiate transition to UDP game protocol

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Server IP (16 bytes)                        |
|                  IPv4 string: "xxx.xxx.xxx.xxx\0"             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         UDP Port (2 bytes)    | Player Count  |               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Player List (variable)                      |
|         PlayerInfo[0], PlayerInfo[1], ... PlayerInfo[N-1]     |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Spawn Positions (variable)                  |
|         [x0, y0], [x1, y1], ... [xN-1, yN-1]                  |
|                  (8 bytes per position: 2 floats)             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**PlayerInfo Structure (41 bytes):**
```
Player Number (1 byte)
Player Hash (8 bytes)
Player Name (32 bytes)
```

**Fields:**
- `Server IP` (16 bytes): UDP game server IP (null-terminated string)
- `UDP Port` (2 bytes): UDP game server port
- `Player Count` (1 byte): Number of players in session
- `Player List` (41 × N bytes): Array of PlayerInfo structures
- `Spawn Positions` (8 × N bytes): Initial positions (X, Y as float32)

**Semantics:**
- Clients MUST disconnect from TCP lobby after receiving this packet
- Clients MUST connect to UDP game server using provided IP:Port
- Clients MUST use their assigned Player Hash for UDP authentication

---

### 5.9 DISCONNECT (0x17)

**Direction:** Client → Server
**Purpose:** Graceful disconnect notification

**Payload:** Empty (no payload)

**Semantics:**
- Client SHOULD send before closing TCP connection
- Server broadcasts `PLAYER_LEFT` to remaining clients

---

### 5.10 PLAYER_LEFT (0x18)

**Direction:** Server → Client (broadcast)
**Purpose:** Notify remaining players of disconnect

**Payload:**
```
 0               1
 0 1 2 3 4 5 6 7 8
+-+-+-+-+-+-+-+-+
| Player Number |
+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Player Number` (1 byte): Player who disconnected

---

### 5.11 COUNTDOWN (0x19)

**Direction:** Server → Client (broadcast)
**Purpose:** Countdown to game start

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                   Remaining Time (4 bytes, float)             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Fields:**
- `Remaining Time` (4 bytes, IEEE 754 float): Seconds until game start

**Semantics:**
- Server sends at regular intervals (recommended: 100ms)
- Countdown typically starts at 5.0 seconds
- When remaining time reaches 0.0, `GAME_START` is sent

---

### 5.12 ERROR_MSG (0x1F)

**Direction:** Server → Client
**Purpose:** Error notification

**Payload:**
```
 0               1               2               3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Error Code   |         Error Message (variable)              |
|               |            UTF-8, null-terminated              |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**Error Codes:**
- `0x01`: Lobby full
- `0x02`: Name already taken
- `0x03`: Invalid name
- `0x04`: Game already started
- `0x05`: Not all players ready
- `0x06`: Minimum players not reached
- `0xFF`: Generic error

---

## 6. State Machine

### 6.1 Client States

```
┌─────────────┐
│ Disconnected│
└──────┬──────┘
       │ connect()
       ▼
┌─────────────┐  CONNECT_ACK
│ Connecting  ├──────────────►┌─────────────┐
└──────┬──────┘                │  Connected  │
       │ timeout               └──────┬──────┘
       ▼                              │ ready()
┌─────────────┐                       ▼
│   Error     │               ┌─────────────┐
└─────────────┘               │    Ready    │
                              └──────┬──────┘
                                     │ COUNTDOWN
                                     ▼
                              ┌─────────────┐
                              │  Starting   │
                              └──────┬──────┘
                                     │ GAME_START
                                     ▼
                              ┌─────────────┐
                              │   In Game   │
                              └─────────────┘
```

### 6.2 Server States (per player)

```
┌─────────────┐
│    Idle     │
└──────┬──────┘
       │ TCP accept
       ▼
┌─────────────┐  CONNECT_REQ
│   Pending   ├──────────────►┌─────────────┐
└──────┬──────┘                │  Connected  │
       │ timeout               └──────┬──────┘
       ▼                              │ READY_REQ
┌─────────────┐                       ▼
│ Disconnected│               ┌─────────────┐
└─────────────┘               │    Ready    │
                              └──────┬──────┘
                                     │ All ready + START_REQ
                                     ▼
                              ┌─────────────┐
                              │  Starting   │
                              └──────┬──────┘
                                     │ GAME_START sent
                                     ▼
                              ┌─────────────┐
                              │ Transferred │
                              └─────────────┘
```

---

## 7. Error Handling

### 7.1 Connection Errors

**Scenario:** TCP connection fails
- **Client Action:** Retry with exponential backoff (max 3 attempts)
- **User Notification:** "Unable to connect to server"

**Scenario:** Server rejects connection (lobby full)
- **Server Response:** `ERROR_MSG` (code 0x01)
- **Client Action:** Close connection
- **User Notification:** "Lobby is full"

### 7.2 Protocol Errors

**Scenario:** Malformed packet
- **Receiver Action:** Log error, ignore packet
- **Connection:** Maintain (unless repeated violations)

**Scenario:** Unexpected packet type
- **Receiver Action:** Log warning, ignore packet

### 7.3 Timeout Errors

**Scenario:** No data received for 30 seconds
- **Action:** Close connection
- **Notification:** Broadcast `PLAYER_LEFT`

---

## 8. Security Considerations

### 8.1 Authentication

The current protocol does NOT implement authentication. This is acceptable for:
- Local network play
- Trusted environments
- Prototype/development

**Recommendations for production:**
- Implement session tokens
- Add password-protected lobbies
- Use TLS for encryption

### 8.2 Denial of Service

**Vulnerability:** Rapid connection attempts
**Mitigation:** Implement rate limiting (max 5 connections/minute per IP)

**Vulnerability:** Large player names
**Mitigation:** Enforce 32-byte limit (already specified)

### 8.3 Information Disclosure

**Risk:** Player hashes may be predictable
**Mitigation:** Use cryptographically secure random number generator

---

## 9. References

### 9.1 Normative References

- **RFC 793** - Transmission Control Protocol
- **RFC 2119** - Key words for use in RFCs to Indicate Requirement Levels
- **RFC 3629** - UTF-8, a transformation format of ISO 10646

### 9.2 Informative References

- **R-Type UDP Game Protocol** - See `RFC_UDP_GAME.md`
- **ASIO Documentation** - https://think-async.com/Asio/

---

## 10. Implementation Notes

### 10.1 Example Usage

**Server:**
```cpp
#include "LobbyServer.hpp"

network::LobbyServer server(4242, 4, 2); // port, max_players, min_players

while (!server.isGameStarted()) {
    server.update();
}
// Transition to UDP game server
```

**Client:**
```cpp
#include "LobbyClient.hpp"

network::LobbyClient client("127.0.0.1", 4242);
client.connect("Alice");

while (!client.isGameStarted()) {
    client.update();

    if (readyButtonClicked) {
        client.ready();
    }

    if (startButtonClicked) {
        client.requestStart();
    }
}
// Transition to UDP game client
```

### 10.2 Testing

Test file: `tests/test_lobby.cpp`

```bash
cmake --build . --target test_lobby
./bin/test_lobby
```

---

## Appendix A: Packet Size Summary

| Packet         | Min Size | Max Size | Typical |
|----------------|----------|----------|---------|
| CONNECT_REQ    | 35 bytes | 35 bytes | 35 bytes|
| CONNECT_ACK    | 12 bytes | 12 bytes | 12 bytes|
| PLAYER_JOIN    | 44 bytes | 44 bytes | 44 bytes|
| READY_REQ      | 4 bytes  | 4 bytes  | 4 bytes |
| PLAYER_READY   | 5 bytes  | 5 bytes  | 5 bytes |
| START_REQ      | 3 bytes  | 3 bytes  | 3 bytes |
| GAME_START     | ~200 bytes| ~400 bytes| 250 bytes|
| DISCONNECT     | 3 bytes  | 3 bytes  | 3 bytes |
| PLAYER_LEFT    | 4 bytes  | 4 bytes  | 4 bytes |
| COUNTDOWN      | 7 bytes  | 7 bytes  | 7 bytes |
| ERROR_MSG      | 4 bytes  | 256 bytes| 50 bytes|

---

## Appendix B: Change Log

**Version 1.0 (December 2025)**
- Initial specification
- Defined all packet types
- Specified state machines
- Added security considerations

---

**End of Document**
