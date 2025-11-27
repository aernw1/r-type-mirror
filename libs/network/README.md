# Network Module

## Quick Start

### Server
```cpp
#include "LobbyServer.hpp"

network::LobbyServer server(4242, 4, 2); // port, max_players, min_players

while (!server.isGameStarted()) {
    server.update();
    server.printStatus(); // Optional: shows connected players
}
// Game starts here - switch to UDP game protocol
```

### Client
```cpp
#include "LobbyClient.hpp"

network::LobbyClient client("127.0.0.1", 4242);
client.connect("PlayerName");

// Game loop
while (window.isOpen()) {
    client.update();

    // Display players in UI
    for (const auto& player : client.getPlayers()) {
        // player.name, player.number, player.ready
    }

    // On "Ready" button click
    if (readyButtonClicked) {
        client.ready();
    }

    // On "Start" button click (only works if all players ready + min players reached)
    if (startButtonClicked) {
        client.requestStart();
    }

    // Check if game started
    if (client.isGameStarted()) {
        // Switch to game scene
        break;
    }
}
```

## Client API

| Method | Description |
|--------|-------------|
| `connect(name)` | Connect to server with player name |
| `ready()` | Mark player as ready |
| `requestStart()` | Request game start |
| `update()` | Process incoming packets (call every frame) |
| `isConnected()` | Returns true if connected |
| `isReady()` | Returns true if player is ready |
| `isGameStarted()` | Returns true if game has started |
| `getPlayers()` | Returns vector of PlayerInfo |
| `getPlayerNumber()` | Returns local player number (1-4) |
| `getGameSeed()` | Returns game seed (after game start) |

## PlayerInfo struct

```cpp
struct PlayerInfo {
    uint8_t number;      // Player slot (1-4)
    char name[32];       // Player name
    bool ready;          // Ready status
    uint64_t hash;       // Unique identifier
};
```

## Build

Link with `rtype_network`:
```cmake
target_link_libraries(your_target PRIVATE rtype_network)
```
