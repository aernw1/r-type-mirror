# RFC: R-Type TCP Lobby Protocol

**Status:** Implemented
**Version:** 1.0
**Date:** January 2025

## Abstract

This document specifies the TCP-based binary protocol used for lobby management in R-Type. The protocol handles room creation, player management, ready states, and game session initiation before transitioning to UDP for gameplay.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Data Types](#3-data-types)
4. [Packet Structure](#4-packet-structure)
5. [Packet Types](#5-packet-types)
6. [Connection Flow](#6-connection-flow)
7. [Error Handling](#7-error-handling)
8. [Implementation Notes](#8-implementation-notes)

---

## 1. Introduction

### 1.1 Purpose

The R-Type TCP Lobby Protocol provides reliable communication for:
- Room discovery and management
- Player session management
- Ready state synchronization
- Game start coordination
- Transition to UDP gameplay

### 1.2 Design Rationale

TCP is used for lobby operations because:
- Reliability is critical for room state changes
- Order preservation prevents race conditions
- Connection-oriented nature simplifies session tracking
- Latency is less critical in lobby context

### 1.3 Terminology

- **Lobby Server**: Central server managing rooms and players
- **Room**: A game session waiting area for up to 4 players
- **Host**: The player who created the room
- **Ready State**: Player indication of readiness to start

---

## 2. Protocol Overview

### 2.1 Transport

- **Protocol**: TCP (Transmission Control Protocol)
- **Default Port**: 7777
- **Byte Order**: Little-endian
- **Alignment**: Packed structures (no padding)
- **Framing**: Length-prefixed messages

### 2.2 Message Framing

All TCP messages are prefixed with a 4-byte length header:

```
+--------+------------------------+
| Length | Payload                |
| 4 bytes| Variable               |
+--------+------------------------+
```

The length field contains the size of the payload only (excluding the 4-byte header).

### 2.3 Packet Types

| Type ID | Name | Direction | Description |
|---------|------|-----------|-------------|
| 0x01 | LIST_ROOMS_REQ | Client → Server | Request room list |
| 0x02 | LIST_ROOMS_ACK | Server → Client | Room list response |
| 0x03 | CREATE_ROOM_REQ | Client → Server | Create new room |
| 0x04 | CREATE_ROOM_ACK | Server → Client | Room creation result |
| 0x05 | JOIN_ROOM_REQ | Client → Server | Join existing room |
| 0x06 | JOIN_ROOM_ACK | Server → Client | Join result |
| 0x07 | ROOM_UPDATE | Server → All | Room state changed |
| 0x10 | CONNECT_REQ | Client → Server | Initial connection |
| 0x11 | CONNECT_ACK | Server → Client | Connection accepted |
| 0x12 | PLAYER_JOIN | Server → All | Player joined room |
| 0x13 | READY_REQ | Client → Server | Toggle ready state |
| 0x14 | PLAYER_READY | Server → All | Player ready state changed |
| 0x15 | START_REQ | Client → Server | Request game start (host only) |
| 0x16 | GAME_START | Server → All | Game starting |
| 0x17 | DISCONNECT | Client → Server | Player disconnecting |
| 0x18 | PLAYER_LEFT | Server → All | Player left room |
| 0x19 | COUNTDOWN | Server → All | Countdown tick |
| 0x1F | ERROR_MSG | Server → Client | Error notification |

---

## 3. Data Types

### 3.1 Primitive Types

| Type | Size | Description |
|------|------|-------------|
| `uint8_t` | 1 byte | Unsigned 8-bit integer |
| `uint16_t` | 2 bytes | Unsigned 16-bit integer (little-endian) |
| `uint32_t` | 4 bytes | Unsigned 32-bit integer (little-endian) |
| `uint64_t` | 8 bytes | Unsigned 64-bit integer (little-endian) |
| `bool` | 1 byte | Boolean (0 = false, 1 = true) |
| `char[N]` | N bytes | Fixed-size null-terminated string |

### 3.2 Constants

```
PLAYER_NAME_SIZE = 32
ROOM_NAME_SIZE = 32
MAX_PLAYERS = 4
MAX_ROOMS = 16
```

### 3.3 Common Structures

#### PlayerInfo

```
+--------+------------+------------------+-------+
| Number | Hash       | Name             | Ready |
| 1 byte | 8 bytes    | 32 bytes         | 1 byte|
+--------+------------+------------------+-------+
Total: 42 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| Number | uint8_t | Player slot number (1-4) |
| Hash | uint64_t | Unique player identifier |
| Name | char[32] | Null-terminated player name |
| Ready | bool | Ready state |

#### RoomInfo

```
+------+------------------+-------------+------------+--------+
| ID   | Name             | PlayerCount | MaxPlayers | InGame |
| 4B   | 32 bytes         | 1 byte      | 1 byte     | 1 byte |
+------+------------------+-------------+------------+--------+
Total: 39 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| ID | uint32_t | Unique room identifier |
| Name | char[32] | Null-terminated room name |
| PlayerCount | uint8_t | Current player count |
| MaxPlayers | uint8_t | Maximum players (default: 4) |
| InGame | bool | True if game is in progress |

#### GameStartInfo

```
+------------------+----------+-------------+---------------------+------------------------+
| ServerIP         | UDPPort  | PlayerCount | Players[4]          | SpawnPositions[4][2]   |
| 16 bytes         | 2 bytes  | 1 byte      | 42 * 4 = 168 bytes  | 4 * 2 * 4 = 32 bytes   |
+------------------+----------+-------------+---------------------+------------------------+
Total: 219 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| ServerIP | char[16] | Game server IP address |
| UDPPort | uint16_t | Game server UDP port |
| PlayerCount | uint8_t | Number of players |
| Players | PlayerInfo[4] | Player information array |
| SpawnPositions | float[4][2] | Player spawn coordinates [x, y] |

---

## 4. Packet Structure

All packets begin with a single byte identifying the packet type, followed by type-specific payload.

```
+--------+------------------------+
| Type   | Payload                |
| 1 byte | Variable               |
+--------+------------------------+
```

---

## 5. Packet Types

### 5.1 Room Management

#### LIST_ROOMS_REQ (0x01)

Request list of available rooms.

```
+--------+
| Type   |
| 1 byte |
+--------+
Total: 1 byte
```

#### LIST_ROOMS_ACK (0x02)

Response containing room list.

```
+--------+-----------+------------------------+
| Type   | RoomCount | Rooms[]                |
| 1 byte | 1 byte    | RoomCount * 39 bytes   |
+--------+-----------+------------------------+
```

| Field | Type | Description |
|-------|------|-------------|
| RoomCount | uint8_t | Number of rooms (0-16) |
| Rooms | RoomInfo[] | Array of room information |

#### CREATE_ROOM_REQ (0x03)

Request to create a new room.

```
+--------+------------------+------------------+
| Type   | RoomName         | PlayerName       |
| 1 byte | 32 bytes         | 32 bytes         |
+--------+------------------+------------------+
Total: 65 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| RoomName | char[32] | Desired room name |
| PlayerName | char[32] | Creating player's name |

#### CREATE_ROOM_ACK (0x04)

Room creation response.

```
+--------+--------+------------+
| Type   | RoomID | PlayerHash |
| 1 byte | 4 bytes| 8 bytes    |
+--------+--------+------------+
Total: 13 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| RoomID | uint32_t | Assigned room ID (0 = failed) |
| PlayerHash | uint64_t | Assigned player hash |

#### JOIN_ROOM_REQ (0x05)

Request to join an existing room.

```
+--------+--------+------------------+
| Type   | RoomID | PlayerName       |
| 1 byte | 4 bytes| 32 bytes         |
+--------+--------+------------------+
Total: 37 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| RoomID | uint32_t | Target room ID |
| PlayerName | char[32] | Joining player's name |

#### JOIN_ROOM_ACK (0x06)

Join room response.

```
+--------+--------+------------+-------------+---------------------+
| Type   | Status | PlayerHash | PlayerCount | Players[]           |
| 1 byte | 1 byte | 8 bytes    | 1 byte      | PlayerCount * 42B   |
+--------+--------+------------+-------------+---------------------+
```

| Field | Type | Description |
|-------|------|-------------|
| Status | JoinRoomStatus | Result code (see below) |
| PlayerHash | uint64_t | Assigned player hash (0 if failed) |
| PlayerCount | uint8_t | Current players in room |
| Players | PlayerInfo[] | Array of player information |

**JoinRoomStatus values:**
| Value | Name | Description |
|-------|------|-------------|
| 0x00 | SUCCESS | Join successful |
| 0x01 | ROOM_FULL | Room has maximum players |
| 0x02 | ROOM_NOT_FOUND | Room ID does not exist |
| 0x03 | ROOM_IN_GAME | Room game already started |

#### ROOM_UPDATE (0x07)

Broadcast when room state changes.

```
+--------+-------------+---------------------+
| Type   | PlayerCount | Players[]           |
| 1 byte | 1 byte      | PlayerCount * 42B   |
+--------+-------------+---------------------+
```

### 5.2 Connection Management

#### CONNECT_REQ (0x10)

Initial connection request (alternative to join flow).

```
+--------+------------------+
| Type   | PlayerName       |
| 1 byte | 32 bytes         |
+--------+------------------+
Total: 33 bytes
```

#### CONNECT_ACK (0x11)

Connection acknowledgment.

```
+--------+---------+------------+
| Type   | Success | PlayerHash |
| 1 byte | 1 byte  | 8 bytes    |
+--------+---------+------------+
Total: 10 bytes
```

#### DISCONNECT (0x17)

Player disconnection notification.

```
+--------+
| Type   |
| 1 byte |
+--------+
Total: 1 byte
```

#### PLAYER_LEFT (0x18)

Broadcast when player leaves room.

```
+--------+------------+
| Type   | PlayerHash |
| 1 byte | 8 bytes    |
+--------+------------+
Total: 9 bytes
```

### 5.3 Ready State Management

#### READY_REQ (0x13)

Toggle player ready state.

```
+--------+
| Type   |
| 1 byte |
+--------+
Total: 1 byte
```

#### PLAYER_READY (0x14)

Broadcast when player ready state changes.

```
+--------+------------+-------+
| Type   | PlayerHash | Ready |
| 1 byte | 8 bytes    | 1 byte|
+--------+------------+-------+
Total: 10 bytes
```

#### PLAYER_JOIN (0x12)

Broadcast when new player joins room.

```
+--------+--------------------+
| Type   | PlayerInfo         |
| 1 byte | 42 bytes           |
+--------+--------------------+
Total: 43 bytes
```

### 5.4 Game Start

#### START_REQ (0x15)

Host request to start game (requires all players ready).

```
+--------+
| Type   |
| 1 byte |
+--------+
Total: 1 byte
```

#### COUNTDOWN (0x19)

Countdown tick before game start.

```
+--------+------------------+
| Type   | SecondsRemaining |
| 1 byte | 1 byte           |
+--------+------------------+
Total: 2 bytes
```

#### GAME_START (0x16)

Game starting notification with connection info.

```
+--------+---------------------+
| Type   | GameStartInfo       |
| 1 byte | 219 bytes           |
+--------+---------------------+
Total: 220 bytes
```

### 5.5 Error Handling

#### ERROR_MSG (0x1F)

Error notification.

```
+--------+-----------+------------------+
| Type   | ErrorCode | Message          |
| 1 byte | 1 byte    | Variable         |
+--------+-----------+------------------+
```

**Common Error Codes:**
| Code | Description |
|------|-------------|
| 0x01 | Room full |
| 0x02 | Room not found |
| 0x03 | Not host |
| 0x04 | Not all players ready |
| 0x05 | Invalid player name |
| 0x06 | Server error |

---

## 6. Connection Flow

### 6.1 Room Creation Flow

```
Client                              Server
   |                                   |
   |-------- CREATE_ROOM_REQ --------->|
   |                                   |
   |<------- CREATE_ROOM_ACK ----------|
   |         (roomId, playerHash)      |
   |                                   |
   |======= In Room (as Host) =========|
```

### 6.2 Room Join Flow

```
Client                              Server                    Other Clients
   |                                   |                            |
   |-------- JOIN_ROOM_REQ ----------->|                            |
   |                                   |                            |
   |<------- JOIN_ROOM_ACK ------------|                            |
   |         (status, players)         |                            |
   |                                   |------- PLAYER_JOIN ------->|
   |                                   |                            |
   |======= In Room ===================|============================|
```

### 6.3 Ready and Start Flow

```
Client                              Server                    Other Clients
   |                                   |                            |
   |-------- READY_REQ --------------->|                            |
   |                                   |------- PLAYER_READY ------>|
   |<------- PLAYER_READY -------------|                            |
   |                                   |                            |
   [All players ready]                 |                            |
   |                                   |                            |
   |-------- START_REQ --------------->| (Host only)                |
   |                                   |                            |
   |<------- COUNTDOWN ----------------|------- COUNTDOWN --------->|
   |         (3)                       |         (3)                |
   |<------- COUNTDOWN ----------------|------- COUNTDOWN --------->|
   |         (2)                       |         (2)                |
   |<------- COUNTDOWN ----------------|------- COUNTDOWN --------->|
   |         (1)                       |         (1)                |
   |                                   |                            |
   |<------- GAME_START ---------------|------- GAME_START -------->|
   |         (serverIP, udpPort, ...)  |                            |
   |                                   |                            |
   [Transition to UDP Game Protocol]   |                            |
```

### 6.4 Disconnection Flow

```
Client                              Server                    Other Clients
   |                                   |                            |
   |-------- DISCONNECT -------------->|                            |
   |                                   |------- PLAYER_LEFT ------->|
   |                                   |                            |
   (Connection closed)                 |                            |
```

---

## 7. Error Handling

### 7.1 Connection Errors

- Server must send ERROR_MSG before closing connection
- Client should handle unexpected disconnection gracefully
- Reconnection requires full room rejoin

### 7.2 Room Errors

- JOIN_ROOM_ACK with status != SUCCESS indicates failure
- Client should return to room list on join failure
- Room removal when last player leaves

### 7.3 Game Start Errors

- START_REQ from non-host is ignored or returns error
- START_REQ with not all players ready fails
- Countdown can be cancelled if player leaves

---

## 8. Implementation Notes

### 8.1 Byte Order

All multi-byte values are transmitted in **little-endian** byte order.

### 8.2 Structure Packing

All structures use `#pragma pack(push, 1)` to eliminate padding.

### 8.3 String Handling

- All strings are fixed-size, null-terminated
- Unused bytes after null should be zero-filled
- Strings exceeding size limit are truncated

### 8.4 Player Hash Generation

Player hash is a unique 64-bit identifier generated by:
1. Server generates random hash on CREATE_ROOM or JOIN_ROOM
2. Hash must be unique within the room
3. Hash is used for UDP protocol player identification

### 8.5 Room Lifecycle

1. Room created on CREATE_ROOM_REQ
2. Room destroyed when:
   - Last player leaves
   - Game ends and all players disconnect
   - Server shutdown

### 8.6 Host Privileges

The first player in a room (creator) is the host:
- Only host can send START_REQ
- Host leaving before game start destroys room or transfers host

### 8.7 Language Agnostic Implementation

To implement this protocol in any language:

1. **TCP Connection:**
   - Connect to lobby server on port 7777
   - Read/write with length-prefixed framing

2. **Message Parsing:**
   - Read 4-byte length header
   - Read payload of that length
   - First byte of payload is packet type
   - Parse remaining bytes according to type

3. **Client Implementation:**
   - Send LIST_ROOMS_REQ to get available rooms
   - Send CREATE_ROOM_REQ or JOIN_ROOM_REQ
   - Wait for corresponding ACK
   - Handle PLAYER_JOIN, PLAYER_LEFT, PLAYER_READY broadcasts
   - Send READY_REQ when ready
   - If host, send START_REQ when all ready
   - Handle COUNTDOWN and GAME_START
   - On GAME_START, connect to UDP game server

4. **Server Implementation:**
   - Accept TCP connections
   - Track rooms and players
   - Broadcast room changes to all room members
   - Validate host-only operations
   - Generate unique player hashes
   - Spawn UDP game server on game start

---

## Appendix A: Packet Size Summary

| Packet | Size (bytes) |
|--------|--------------|
| LIST_ROOMS_REQ | 1 |
| LIST_ROOMS_ACK (header) | 2 |
| RoomInfo | 39 |
| CREATE_ROOM_REQ | 65 |
| CREATE_ROOM_ACK | 13 |
| JOIN_ROOM_REQ | 37 |
| JOIN_ROOM_ACK (header) | 11 |
| PlayerInfo | 42 |
| ROOM_UPDATE (header) | 2 |
| CONNECT_REQ | 33 |
| CONNECT_ACK | 10 |
| PLAYER_JOIN | 43 |
| READY_REQ | 1 |
| PLAYER_READY | 10 |
| START_REQ | 1 |
| COUNTDOWN | 2 |
| GAME_START | 220 |
| DISCONNECT | 1 |
| PLAYER_LEFT | 9 |

---

## Appendix B: Example Packets

### B.1 CREATE_ROOM_REQ (Hex)

```
Length (4 bytes): 41 00 00 00
Payload:
03                              # Type: CREATE_ROOM_REQ
4D 79 52 6F 6F 6D 00 00        # RoomName: "MyRoom" + null padding
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
50 6C 61 79 65 72 31 00        # PlayerName: "Player1" + null padding
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
```

### B.2 JOIN_ROOM_ACK (Success, 2 players)

```
Length: dependent on player count
Payload:
06                              # Type: JOIN_ROOM_ACK
00                              # Status: SUCCESS
78 56 34 12 00 00 00 00        # PlayerHash: 0x12345678
02                              # PlayerCount: 2
[Player 1 Info - 42 bytes]
[Player 2 Info - 42 bytes]
```

### B.3 GAME_START (Hex Structure)

```
16                              # Type: GAME_START
31 32 37 2E 30 2E 30 2E 31 00  # ServerIP: "127.0.0.1"
00 00 00 00 00 00
62 1E                           # UDPPort: 7778
02                              # PlayerCount: 2
[PlayerInfo 1 - 42 bytes]
[PlayerInfo 2 - 42 bytes]
[PlayerInfo 3 - 42 bytes (empty)]
[PlayerInfo 4 - 42 bytes (empty)]
[SpawnPositions - 32 bytes]
```

---

## Appendix C: State Diagram

```
                    ┌─────────────┐
                    │ Disconnected│
                    └──────┬──────┘
                           │ Connect
                           ▼
                    ┌─────────────┐
                    │   Lobby     │◄────────────────┐
                    │  (Room List)│                 │
                    └──────┬──────┘                 │
                           │                        │
           ┌───────────────┼───────────────┐       │
           │ Create        │ Join          │       │
           ▼               ▼               │       │
    ┌─────────────┐ ┌─────────────┐       │       │
    │ In Room     │ │ Join Failed │───────┘       │
    │ (Waiting)   │ └─────────────┘               │
    └──────┬──────┘                               │
           │ All Ready + Start                     │
           ▼                                       │
    ┌─────────────┐                               │
    │  Countdown  │                               │
    └──────┬──────┘                               │
           │ Complete                              │
           ▼                                       │
    ┌─────────────┐                               │
    │  In Game    │───────────────────────────────┘
    │   (UDP)     │ Game Over
    └─────────────┘
```
