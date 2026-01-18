# RFC: R-Type UDP Game Protocol

**Status:** Implemented
**Version:** 1.0
**Date:** January 2025

## Abstract

This document specifies the UDP-based binary protocol used for real-time game communication between R-Type clients and game servers. The protocol is designed for low-latency gameplay with support for delta compression and LZ4 packet compression.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Protocol Overview](#2-protocol-overview)
3. [Data Types](#3-data-types)
4. [Packet Structure](#4-packet-structure)
5. [Packet Types](#5-packet-types)
6. [Entity Types](#6-entity-types)
7. [Input Flags](#7-input-flags)
8. [Power-Up Flags](#8-power-up-flags)
9. [Delta Encoding](#9-delta-encoding)
10. [Compression](#10-compression)
11. [Connection Flow](#11-connection-flow)
12. [Implementation Notes](#12-implementation-notes)

---

## 1. Introduction

### 1.1 Purpose

The R-Type UDP protocol enables real-time synchronization of game state between an authoritative server and multiple clients. The server maintains the canonical game state and broadcasts updates to all connected clients.

### 1.2 Requirements

- Binary format for bandwidth efficiency
- UDP transport for low latency
- Support for up to 4 players
- Maximum 256 entities per game state snapshot
- Delta encoding for bandwidth optimization
- Optional LZ4 compression

### 1.3 Terminology

- **Server**: The authoritative game instance that processes inputs and manages game state
- **Client**: A player's game instance that sends inputs and renders received state
- **Tick**: A single server update cycle (typically 60 per second)
- **Snapshot**: A complete or partial representation of game state

---

## 2. Protocol Overview

### 2.1 Transport

- **Protocol**: UDP (User Datagram Protocol)
- **Default Port**: Dynamically assigned by lobby (typically 7778+)
- **Byte Order**: Little-endian
- **Alignment**: Packed structures (no padding)

### 2.2 Packet Types

| Type ID | Name | Direction | Description |
|---------|------|-----------|-------------|
| 0x00 | HELLO | Client → Server | Initial connection request |
| 0x01 | WELCOME | Server → Client | Connection accepted |
| 0x02 | INPUT | Client → Server | Player input state |
| 0x03 | STATE | Server → Client | Full game state snapshot |
| 0x04 | PING | Bidirectional | Keepalive request |
| 0x05 | PONG | Bidirectional | Keepalive response |
| 0x06 | DISCONNECT | Client → Server | Graceful disconnection |
| 0x07 | LEVEL_COMPLETE | Server → Client | Level transition notification |
| 0x08 | STATE_DELTA | Server → Client | Delta state snapshot |
| 0x09 | STATE_ACK | Client → Server | State receipt acknowledgment |

---

## 3. Data Types

### 3.1 Primitive Types

| Type | Size | Description |
|------|------|-------------|
| `uint8_t` | 1 byte | Unsigned 8-bit integer |
| `uint16_t` | 2 bytes | Unsigned 16-bit integer (little-endian) |
| `uint32_t` | 4 bytes | Unsigned 32-bit integer (little-endian) |
| `uint64_t` | 8 bytes | Unsigned 64-bit integer (little-endian) |
| `float` | 4 bytes | IEEE 754 single-precision (little-endian) |
| `char[N]` | N bytes | Fixed-size null-terminated string |

### 3.2 Constants

```
PLAYER_NAME_SIZE = 32
MAX_PLAYERS = 4
MAX_ENTITIES = 256
```

---

## 4. Packet Structure

All packets begin with a single byte identifying the packet type.

```
+--------+------------------------+
| Type   | Payload                |
| 1 byte | Variable               |
+--------+------------------------+
```

---

## 5. Packet Types

### 5.1 HELLO Packet (0x00)

Sent by client to initiate connection to game server.

```
+--------+------------+------------------+
| Type   | PlayerHash | PlayerName       |
| 1 byte | 8 bytes    | 32 bytes         |
+--------+------------+------------------+
Total: 41 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| Type | uint8_t | 0x00 |
| PlayerHash | uint64_t | Unique player identifier from lobby |
| PlayerName | char[32] | Null-terminated player name |

### 5.2 WELCOME Packet (0x01)

Sent by server to confirm client connection.

```
+--------+------------------+------------+
| Type   | PlayersConnected | ServerTick |
| 1 byte | 1 byte           | 4 bytes    |
+--------+------------------+------------+
Total: 6 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| Type | uint8_t | 0x01 |
| PlayersConnected | uint8_t | Number of players currently connected |
| ServerTick | uint32_t | Current server tick number |

### 5.3 INPUT Packet (0x02)

Sent by client to transmit player input state.

```
+--------+----------+------------+--------+-----------+
| Type   | Sequence | PlayerHash | Inputs | Timestamp |
| 1 byte | 4 bytes  | 8 bytes    | 1 byte | 4 bytes   |
+--------+----------+------------+--------+-----------+
Total: 18 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| Type | uint8_t | 0x02 |
| Sequence | uint32_t | Client-side input sequence number |
| PlayerHash | uint64_t | Player identifier |
| Inputs | uint8_t | Bitfield of input flags (see Section 7) |
| Timestamp | uint32_t | Client timestamp in milliseconds |

### 5.4 STATE Packet (0x03)

Sent by server to broadcast full game state snapshot.

**Header:**
```
+--------+------+-----------+-------------+--------------+--------------+---------------+
| Type   | Tick | Timestamp | EntityCount | ScrollOffset | InputAckCount| StateSequence |
| 1 byte | 4B   | 4 bytes   | 2 bytes     | 4 bytes      | 1 byte       | 4 bytes       |
+--------+------+-----------+-------------+--------------+--------------+---------------+
Header: 20 bytes
```

**Followed by InputAck entries (if InputAckCount > 0):**
```
+------------+------------------+-------------+-------------+
| PlayerHash | LastProcessedSeq | ServerPosX  | ServerPosY  |
| 8 bytes    | 4 bytes          | 4 bytes     | 4 bytes     |
+------------+------------------+-------------+-------------+
Per InputAck: 20 bytes
```

**Followed by EntityState entries:**
```
+----------+------------+----+----+----+----+--------+-------+-----------+-------+-------------+-----------------+------------+----------+
| EntityId | EntityType | X  | Y  | VX | VY | Health | Flags | OwnerHash | Score | PowerUpFlags| SpeedMultiplier | WeaponType | FireRate |
| 4 bytes  | 1 byte     | 4B | 4B | 4B | 4B | 2 bytes| 1 byte| 8 bytes   | 4B    | 1 byte      | 1 byte          | 1 byte     | 1 byte   |
+----------+------------+----+----+----+----+--------+-------+-----------+-------+-------------+-----------------+------------+----------+
Per Entity: 40 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| Tick | uint32_t | Server tick number |
| Timestamp | uint32_t | Server timestamp (ms) |
| EntityCount | uint16_t | Number of entity states following |
| ScrollOffset | float | Background scroll position |
| InputAckCount | uint8_t | Number of input acknowledgments |
| StateSequence | uint32_t | Sequence number for delta tracking |

### 5.5 PING Packet (0x04)

Keepalive request.

```
+--------+-----------+
| Type   | Timestamp |
| 1 byte | 4 bytes   |
+--------+-----------+
Total: 5 bytes
```

### 5.6 PONG Packet (0x05)

Keepalive response (echoes PING timestamp).

```
+--------+-----------+
| Type   | Timestamp |
| 1 byte | 4 bytes   |
+--------+-----------+
Total: 5 bytes
```

### 5.7 DISCONNECT Packet (0x06)

Graceful disconnection notification.

```
+--------+
| Type   |
| 1 byte |
+--------+
Total: 1 byte
```

### 5.8 LEVEL_COMPLETE Packet (0x07)

Notification that current level is complete.

```
+--------+----------------+-----------+
| Type   | CompletedLevel | NextLevel |
| 1 byte | 1 byte         | 1 byte    |
+--------+----------------+-----------+
Total: 3 bytes
```

| Field | Type | Description |
|-------|------|-------------|
| CompletedLevel | uint8_t | Level number just completed |
| NextLevel | uint8_t | Next level to load (0 = game complete) |

### 5.9 STATE_DELTA Packet (0x08)

Optimized delta state snapshot (only changed entities).

**Header:**
```
+--------+------+-----------+---------------+--------------+------------------+--------------+--------------+--------------+------------------+------------------+
| Type   | Tick | Timestamp | StateSequence | BaseSequence | DeltaEntityCount | DestroyedCnt | NewEntityCnt | ScrollOffset | InputAckCount    | CompressionFlags |
| 1 byte | 4B   | 4 bytes   | 4 bytes       | 4 bytes      | 2 bytes          | 2 bytes      | 2 bytes      | 4 bytes      | 1 byte           | 1 byte           |
+--------+------+-----------+---------------+--------------+------------------+--------------+--------------+--------------+------------------+------------------+
```

**Followed by UncompressedSize (4 bytes) if compression is enabled.**

**Payload (potentially compressed):**
1. InputAck entries
2. Delta entity updates (changed fields only)
3. Destroyed entity IDs (uint32_t each)
4. New full entity states

**Delta Entity Header:**
```
+----------+------------+
| EntityId | DeltaFlags |
| 4 bytes  | 1 byte     |
+----------+------------+
```

Only fields indicated by DeltaFlags are included after the header.

### 5.10 STATE_ACK Packet (0x09)

Client acknowledgment of received state.

```
+--------+------------+-----------------+
| Type   | PlayerHash | LastReceivedSeq |
| 1 byte | 8 bytes    | 4 bytes         |
+--------+------------+-----------------+
Total: 13 bytes
```

---

## 6. Entity Types

| Value | Name | Description |
|-------|------|-------------|
| 0x01 | PLAYER | Player ship |
| 0x02 | ENEMY | Enemy ship |
| 0x03 | BULLET | Projectile (player or enemy) |
| 0x04 | POWERUP | Collectible power-up |
| 0x05 | OBSTACLE | Static or scrolling obstacle |
| 0x06 | BOSS | Boss enemy |

### 6.1 Entity-Specific Flags

**PLAYER flags:**
| Flag | Description |
|------|-------------|
| 0x80 | Force Pod (attached helper) |

**BULLET flags:**
| Value | Description |
|-------|-------------|
| 0 | Standard player bullet |
| 10-12 | Enemy bullets (type = flags - 10) |
| 13 | Boss bullet |
| 14 | Black Orb |
| 15 | Third Bullet |
| 16 | Wave Attack |
| 17 | Second Attack |
| 18 | Fire Bullet |
| 19 | Mine (inactive) |
| 20 | Mine (exploding) |

**BOSS flags:**
| Value | Description |
|-------|-------------|
| 0 | Normal state |
| 1 | Damage flash active |

---

## 7. Input Flags

Bitfield representing player input state:

| Bit | Value | Action |
|-----|-------|--------|
| 0 | 0x01 | UP - Move up |
| 1 | 0x02 | DOWN - Move down |
| 2 | 0x04 | LEFT - Move left |
| 3 | 0x08 | RIGHT - Move right |
| 4 | 0x10 | SHOOT - Fire weapon |

**Example:** Moving up while shooting = 0x01 | 0x10 = 0x11

---

## 8. Power-Up Flags

Bitfield in EntityState for player power-ups:

| Bit | Value | Power-Up |
|-----|-------|----------|
| 0 | 0x01 | Fire Rate Boost |
| 1 | 0x02 | Spread Shot |
| 2 | 0x04 | Laser Beam |
| 3 | 0x08 | Shield |
| 4 | 0x10 | Force Pod |

---

## 9. Delta Encoding

### 9.1 Delta Flags

Bitfield indicating which fields changed:

| Bit | Value | Field(s) |
|-----|-------|----------|
| 0 | 0x01 | Position (x, y) |
| 1 | 0x02 | Velocity (vx, vy) |
| 2 | 0x04 | Health |
| 3 | 0x08 | Flags |
| 4 | 0x10 | Score |
| 5 | 0x20 | Power-up flags |
| 6 | 0x40 | Weapon type / Fire rate |
| 7 | 0x80 | Entity destroyed |

### 9.2 Delta Payload Format

For each changed entity:
1. EntityId (4 bytes)
2. DeltaFlags (1 byte)
3. Only fields with corresponding flag bit set

**Example:** Entity 42 with position changed (deltaFlags = 0x01):
```
+----------+------------+------+------+
| EntityId | DeltaFlags | X    | Y    |
| 0x2A     | 0x01       | 4B   | 4B   |
+----------+------------+------+------+
Total: 13 bytes (vs 40 bytes for full entity)
```

### 9.3 Position Threshold

Position changes below 0.5 units are not transmitted to reduce bandwidth.

---

## 10. Compression

### 10.1 Compression Flags

| Value | Algorithm |
|-------|-----------|
| 0x00 | None |
| 0x01 | LZ4 |

### 10.2 Compression Rules

- Only STATE_DELTA packets may be compressed
- Compression is applied to the payload only (after header)
- Minimum size for compression: 100 bytes
- Compression must provide at least 10% reduction
- UncompressedSize field contains original payload size

### 10.3 LZ4 Implementation

- Uses LZ4 default compression (`LZ4_compress_default`)
- Decompression uses `LZ4_decompress_safe`

---

## 11. Connection Flow

### 11.1 Connection Establishment

```
Client                              Server
   |                                   |
   |-------- HELLO (hash, name) ------>|
   |                                   |
   |<------- WELCOME (tick, count) ----|
   |                                   |
   |======= Game Session Active =======|
```

### 11.2 Gameplay Loop

```
Client                              Server
   |                                   |
   |-------- INPUT (seq, inputs) ----->| (every frame)
   |                                   |
   |<------- STATE/STATE_DELTA --------| (60 Hz)
   |                                   |
   |-------- STATE_ACK --------------->| (periodic)
   |                                   |
```

### 11.3 Keepalive

- PING/PONG exchanged every 1 second
- Disconnect timeout: 10 seconds without response

### 11.4 Disconnection

```
Client                              Server
   |                                   |
   |-------- DISCONNECT -------------->|
   |                                   |
   (Connection closed)
```

---

## 12. Implementation Notes

### 12.1 Byte Order

All multi-byte values are transmitted in **little-endian** byte order.

### 12.2 Structure Packing

All structures use `#pragma pack(push, 1)` to eliminate padding.

### 12.3 Server Authority

The server is fully authoritative:
- Server processes all inputs
- Server computes all physics and collisions
- Client only renders received state
- Client may perform prediction but must reconcile with server

### 12.4 Tick Rate

- Server tick rate: 60 Hz
- State broadcast rate: 60 Hz
- Full snapshot interval: Every 60 ticks (1 second)
- Delta snapshots: Between full snapshots

### 12.5 Error Handling

- Malformed packets must be silently discarded
- Unknown packet types must be ignored
- Server must not crash on invalid client data

### 12.6 Language Agnostic Implementation

To implement this protocol in any language:

1. **Packet Serialization:**
   - Read/write bytes in little-endian order
   - Use packed structures or manual byte manipulation
   - First byte always identifies packet type

2. **Client Implementation:**
   - Send HELLO on connection
   - Wait for WELCOME
   - Send INPUT packets at regular intervals (e.g., 60 Hz)
   - Process STATE/STATE_DELTA packets to update local game state
   - Send STATE_ACK periodically
   - Handle LEVEL_COMPLETE for level transitions
   - Send DISCONNECT on exit

3. **Server Implementation:**
   - Listen for HELLO packets
   - Validate player hash against expected players
   - Send WELCOME on successful validation
   - Process INPUT packets to update player state
   - Broadcast STATE/STATE_DELTA at 60 Hz
   - Track client acknowledgments for delta encoding
   - Handle DISCONNECT for graceful shutdown

---

## Appendix A: Packet Size Summary

| Packet | Size (bytes) |
|--------|--------------|
| HELLO | 41 |
| WELCOME | 6 |
| INPUT | 18 |
| STATE (header) | 20 |
| EntityState | 40 |
| InputAck | 20 |
| PING | 5 |
| PONG | 5 |
| DISCONNECT | 1 |
| LEVEL_COMPLETE | 3 |
| STATE_DELTA (header) | 28 |
| STATE_ACK | 13 |
| DeltaEntityHeader | 5 |

---

## Appendix B: Example Packets

### B.1 HELLO Packet (Hex)

```
00                              # Type: HELLO
78 56 34 12 00 00 00 00        # PlayerHash: 0x12345678
50 6C 61 79 65 72 31 00        # PlayerName: "Player1" + null padding
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
```

### B.2 INPUT Packet (Hex)

```
02                              # Type: INPUT
2A 00 00 00                    # Sequence: 42
78 56 34 12 00 00 00 00        # PlayerHash: 0x12345678
11                              # Inputs: UP + SHOOT
E8 03 00 00                    # Timestamp: 1000ms
```

### B.3 STATE Packet Structure

```
03                              # Type: STATE
3C 00 00 00                    # Tick: 60
E8 03 00 00                    # Timestamp: 1000ms
02 00                          # EntityCount: 2
00 00 00 00                    # ScrollOffset: 0.0
01                              # InputAckCount: 1
01 00 00 00                    # StateSequence: 1
[InputAck data]                # 20 bytes per ack
[EntityState data]             # 40 bytes per entity
```
