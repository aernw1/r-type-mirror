# Guide d'intégration : Lobby TCP + Game UDP avec Sync

## Objectif

Garder **100% de ton lobby TCP** fonctionnel et ajouter le système **Sync UDP** pour la partie jeu.

## Architecture finale

```
1. [Lobby TCP] ──> LobbyServer + LobbyClient
   │
   │ (Les joueurs se connectent, marquent "ready", countdown)
   │
   ↓
2. [Transition] ──> Extraction des PlayerInfo depuis le lobby
   │
   ↓
3. [Game UDP avec ECS] ──> GameServer crée un Registry ECS + SyncServer
                          ──> GameClient crée un Registry ECS + SyncClient
```

## Modifications nécessaires

### Option A : Approche minimale (recommandée)

Garde ton `GameServer` actuel **EXACTEMENT comme il est** et lance le serveur ECS **en parallèle** sur un autre port.

#### Dans ton `server/main.cpp` (après le lobby) :

```cpp
// Après le countdown du lobby
auto players = lobbyServer.GetPlayers();

// 1. Lance ton GameServer actuel (sur port 4242)
std::thread oldGameThread([&players]() {
    network::GameServer oldGame(4242, players);
    oldGame.Run();
});

// 2. Lance AUSSI le nouveau serveur ECS (sur port 4243) EN PARALLÈLE
RType::ECS::Registry ecsRegistry;
RType::Network::SyncServer syncServer(4243, ecsRegistry);

// Ajoute les joueurs au SyncServer
for (auto& player : players) {
    // Trouve l'endpoint UDP depuis le lobby (à adapter)
    network::Endpoint udpEndpoint(player.address, 4243);
    syncServer.AddClient(player.hash, player.number, udpEndpoint);

    // Crée l'entité joueur dans le registry ECS
    auto entity = ecsRegistry.CreateEntity();
    ecsRegistry.AddComponent<RType::ECS::Position>(
        entity, RType::ECS::Position(100, 300 + player.number * 100)
    );
    ecsRegistry.AddComponent<RType::ECS::Player>(
        entity, RType::ECS::Player(player.number, player.hash, false)
    );
    ecsRegistry.AddComponent<RType::ECS::Health>(entity, RType::ECS::Health(100));
}

// Game loop ECS (60 ticks/sec)
while (true) {
    // Update game logic dans le registry
    // ...

    // Sync réseau
    syncServer.Update();

    std::this_thread::sleep_for(std::chrono::milliseconds(16));
}
```

#### Dans ton `client` (GameState) :

```cpp
// Dans GameState::Enter() ou après la transition du lobby

// 1. Garde ton ancien GameClient (si tu veux)
m_gameClient = std::make_unique<network::GameClient>(...);

// 2. Ajoute le SyncClient
m_ecsRegistry = std::make_unique<RType::ECS::Registry>();
m_syncClient = std::make_unique<RType::Network::SyncClient>(
    serverAddress, 4243,  // Port ECS
    m_localPlayerHash,
    m_localPlayerNumber,
    *m_ecsRegistry
);

// Dans GameState::Update(float dt)
void GameState::Update(float dt) {
    // Update réseau ECS
    m_syncClient->Update();

    // Render les entités depuis le registry ECS
    auto entities = m_ecsRegistry->GetEntitiesWithComponent<RType::ECS::Position>();
    for (auto entity : entities) {
        auto& pos = m_ecsRegistry->GetComponent<RType::ECS::Position>(entity);
        auto& player = m_ecsRegistry->GetComponent<RType::ECS::Player>(entity);

        // Dessine le vaisseau à la position (pos.x, pos.y)
        if (player.playerNumber == m_localPlayerNumber) {
            // C'est MOI → dessine en vert
            DrawPlayerShip(pos.x, pos.y, GREEN);
        } else {
            // C'est un AUTRE joueur → dessine en bleu
            DrawPlayerShip(pos.x, pos.y, BLUE);
        }
    }
}
```

### Option B : Remplacer complètement GameServer

Si tu veux **remplacer** ton ancien GameServer par le nouveau :

#### 1. Modifie `GameServer.hpp`

```cpp
#include "SyncServer.hpp"
#include "ECS/Registry.hpp"

class GameServer {
public:
    GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers);
    void Run();

private:
    std::unique_ptr<RType::ECS::Registry> m_registry;
    std::unique_ptr<RType::Network::SyncServer> m_syncServer;

    // Garde tes méthodes actuelles
    void ProcessInputs();
    void UpdateGameLogic(float dt);
};
```

#### 2. Modifie `GameServer.cpp`

```cpp
GameServer::GameServer(uint16_t port, const std::vector<PlayerInfo>& expectedPlayers) {
    m_registry = std::make_unique<RType::ECS::Registry>();
    m_syncServer = std::make_unique<RType::Network::SyncServer>(port, *m_registry);

    // Crée les entités joueurs dans le registry
    for (auto& player : expectedPlayers) {
        auto entity = m_registry->CreateEntity();
        m_registry->AddComponent<RType::ECS::Position>(
            entity, RType::ECS::Position(100, 300)
        );
        m_registry->AddComponent<RType::ECS::Player>(
            entity, RType::ECS::Player(player.number, player.hash, false)
        );
    }
}

void GameServer::Run() {
    const float TICK_RATE = 1.0f / 60.0f;

    while (m_running) {
        auto start = std::chrono::steady_clock::now();

        // 1. Update game logic dans le registry
        auto entities = m_registry->GetEntitiesWithComponent<RType::ECS::Position>();
        for (auto entity : entities) {
            if (m_registry->HasComponent<RType::ECS::Velocity>(entity)) {
                auto& pos = m_registry->GetComponent<RType::ECS::Position>(entity);
                auto& vel = m_registry->GetComponent<RType::ECS::Velocity>(entity);

                pos.x += vel.dx * TICK_RATE;
                pos.y += vel.dy * TICK_RATE;
            }
        }

        // 2. Sync réseau (envoie snapshots automatiquement)
        m_syncServer->Update();

        // 3. Sleep pour maintenir 60 tick/s
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto sleepTime = std::chrono::duration<float>(TICK_RATE) - elapsed;
        if (sleepTime.count() > 0) {
            std::this_thread::sleep_for(sleepTime);
        }
    }
}
```

## Quelle option choisir ?

### ✅ **Option A** si :
- Tu veux tester rapidement
- Tu ne veux PAS casser ton code actuel
- Tu veux comparer les deux systèmes

### ✅ **Option B** si :
- Tu veux un code propre et unifié
- Tu es prêt à refactoriser
- Tu veux utiliser seulement le système ECS

## Migration progressive (recommandé)

1. **Étape 1** : Utilise Option A (2 serveurs en parallèle)
2. **Étape 2** : Vérifie que le système ECS fonctionne bien
3. **Étape 3** : Migre progressivement vers Option B
4. **Étape 4** : Supprime l'ancien GameServer custom

## Points clés à retenir

✅ Le **lobby TCP reste INTACT** - zéro modification
✅ Le **système Sync UDP** s'ajoute **après** le lobby
✅ Les **PlayerInfo du lobby** sont transmis au GameServer ECS
✅ Chaque client contrôle **uniquement SON vaisseau**
✅ Le serveur ECS synchronise **automatiquement** tous les clients

## Troubleshooting

### "Mon lobby ne lance plus le jeu"
→ Vérifie que tu appelles bien `GameServer` après le countdown

### "Les clients ne voient rien"
→ Ajoute des logs dans `SyncClient::HandleUpdatePacket()` pour voir si les snapshots arrivent

### "Lag / rollback"
→ Normal si la latence est élevée. Ajoute la client-side prediction (étape suivante)

---

**TL;DR** : Lance `SyncServer` et `SyncClient` **APRÈS** ton lobby TCP. Rien d'autre ne change ! 🚀
