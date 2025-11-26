#include "LobbyClient.hpp"
#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/Component.hpp"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>

namespace {

struct LobbyVisualState {
    std::unordered_map<uint8_t, RType::ECS::Entity> entities;
};

std::string sanitizePlayerName(const network::PlayerInfo& info) {
    size_t length = 0;
    while (length < network::PLAYER_NAME_SIZE && info.name[length] != '\0') {
        ++length;
    }
    return std::string(info.name, length);
}

std::vector<network::PlayerInfo> collectPlayers(const network::LobbyClient& client) {
    std::vector<network::PlayerInfo> players = client.getPlayers();

    if (client.isConnected()) {
        const auto& me = client.getMyInfo();
        if (me.number != 0) {
            auto it = std::find_if(players.begin(), players.end(), [&](const auto& entry) {
                return entry.number == me.number;
            });
            if (it == players.end()) {
                players.push_back(me);
            } else {
                *it = me;
            }
        }
    }

    std::sort(players.begin(), players.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.number < rhs.number;
    });
    return players;
}

Renderer::Color colorForPlayer(const network::PlayerInfo& info, uint8_t myNumber) {
    const bool isSelf = info.number == myNumber;
    if (info.ready) {
        return isSelf ? Renderer::Color{0.1f, 0.7f, 0.3f, 1.0f}
                      : Renderer::Color{0.2f, 0.6f, 0.2f, 1.0f};
    }
    return isSelf ? Renderer::Color{0.2f, 0.4f, 0.8f, 1.0f}
                  : Renderer::Color{0.35f, 0.35f, 0.35f, 1.0f};
}

void syncLobbyVisuals(RType::ECS::Registry& registry,
                      LobbyVisualState& state,
                      const std::vector<network::PlayerInfo>& players,
                      uint8_t myNumber) {
    std::unordered_set<uint8_t> seen;
    constexpr float baseX = 200.0f;
    constexpr float baseY = 200.0f;
    constexpr float spacing = 110.0f;

    for (const auto& player : players) {
        if (player.number == 0) {
            continue;
        }

        seen.insert(player.number);

        auto it = state.entities.find(player.number);
        if (it == state.entities.end()) {
            auto entity = registry.CreateEntity();
            registry.AddComponent(entity, RType::ECS::Position{});

            RType::ECS::Drawable drawable;
            drawable.type = RType::ECS::Drawable::Type::Rectangle;
            drawable.size = Renderer::Vector2{380.0f, 85.0f};
            drawable.layer = 10;
            registry.AddComponent(entity, std::move(drawable));

            it = state.entities.emplace(player.number, entity).first;
        }

        auto entity = it->second;
        auto& position = registry.GetComponent<RType::ECS::Position>(entity);
        position.x = baseX;
        position.y = baseY + (player.number - 1) * spacing;

        auto& drawable = registry.GetComponent<RType::ECS::Drawable>(entity);
        drawable.tint = colorForPlayer(player, myNumber);
    }

    for (auto it = state.entities.begin(); it != state.entities.end();) {
        if (!seen.count(it->first)) {
            registry.DestroyEntity(it->second);
            it = state.entities.erase(it);
        } else {
            ++it;
        }
    }
}

Renderer::FontId loadLobbyFont(Renderer::IRenderer* renderer) {
    const std::vector<std::string> candidates = {
        "assets/fonts/Roboto-Regular.ttf",
        "assets/fonts/DejaVuSans.ttf",
#ifdef _WIN32
        "C:/Windows/Fonts/arial.ttf",
#endif
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf"
    };

    for (const auto& path : candidates) {
        if (!std::filesystem::exists(path)) {
            continue;
        }
        auto fontId = renderer->LoadFont(path, 28);
        if (fontId != Renderer::INVALID_FONT_ID) {
            return fontId;
        }
    }

    return Renderer::INVALID_FONT_ID;
}

void drawPlayerLabels(Renderer::IRenderer* renderer,
                      Renderer::FontId fontId,
                      const std::vector<network::PlayerInfo>& players,
                      const LobbyVisualState& state,
                      RType::ECS::Registry& registry,
                      uint8_t myNumber) {
    if (fontId == Renderer::INVALID_FONT_ID) {
        return;
    }

    for (const auto& player : players) {
        auto it = state.entities.find(player.number);
        if (it == state.entities.end() || !registry.IsEntityAlive(it->second)) {
            continue;
        }

        const auto& position = registry.GetComponent<RType::ECS::Position>(it->second);

        Renderer::TextParams params;
        params.position = Renderer::Vector2(position.x + 18.0f, position.y + 46.0f);
        params.color = Renderer::Color{1.0f, 1.0f, 1.0f, 1.0f};
        params.scale = 0.9f;

        std::string label = std::to_string(player.number) + ". " + sanitizePlayerName(player);
        label += player.ready ? " (Ready)" : " (Waiting)";
        if (player.number == myNumber) {
            label += " - You";
        }

        renderer->DrawText(fontId, label, params);
    }
}

void drawInstructions(Renderer::IRenderer* renderer,
                      Renderer::FontId fontId,
                      const network::LobbyClient& client) {
    if (fontId == Renderer::INVALID_FONT_ID) {
        return;
    }

    Renderer::TextParams params;
    params.position = Renderer::Vector2(60.0f, 80.0f);
    params.color = Renderer::Color{1.0f, 0.9f, 0.7f, 1.0f};
    params.scale = 0.8f;

    std::string line = client.isConnected()
        ? "Space: Ready | Enter: Request start | Esc: Quit"
        : "Connecting to lobby ...";
    renderer->DrawText(fontId, line, params);

    if (client.isGameStarted()) {
        params.position = Renderer::Vector2(60.0f, 120.0f);
        params.color = Renderer::Color{0.4f, 1.0f, 0.6f, 1.0f};
        renderer->DrawText(fontId, "Game starting...", params);
    }
}

}

int main(int argc, char* argv[]) {
    std::string addr = "127.0.0.1";
    uint16_t port = 4242;
    std::string name = "Player";

    if (argc > 1)
        addr = argv[1];
    if (argc > 2)
        port = static_cast<uint16_t>(std::stoi(argv[2]));
    if (argc > 3)
        name = argv[3];

    network::LobbyClient lobbyClient(addr, port);
    lobbyClient.connect(name);

    auto engine = std::make_unique<RType::Core::Engine>();
    auto rendererModule = std::make_unique<Renderer::SFMLRenderer>();
    auto* renderer = rendererModule.get();

    engine->RegisterModule(std::move(rendererModule));
    engine->RegisterSystem(std::make_unique<RType::ECS::RenderingSystem>(renderer));

    if (!engine->Initialize()) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return 1;
    }

    Renderer::WindowConfig windowConfig;
    windowConfig.title = "R-Type Lobby";
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.resizable = true;

    if (!renderer->CreateWindow(windowConfig)) {
        std::cerr << "Failed to create rendering window" << std::endl;
        engine->Shutdown();
        return 1;
    }

    Renderer::FontId lobbyFont = loadLobbyFont(renderer);
    LobbyVisualState visualState;
    auto& registry = engine->GetRegistry();

    auto lastFrame = std::chrono::steady_clock::now();
    bool readyKeyHeld = false;
    bool startKeyHeld = false;

    while (renderer->IsWindowOpen()) {
        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        renderer->Update(deltaTime);
        lobbyClient.update();

        const auto players = collectPlayers(lobbyClient);
        syncLobbyVisuals(registry, visualState, players, lobbyClient.getMyInfo().number);

        if (renderer->IsKeyPressed(Renderer::Key::Space)) {
            if (!readyKeyHeld && lobbyClient.isConnected()) {
                lobbyClient.ready();
            }
            readyKeyHeld = true;
        } else {
            readyKeyHeld = false;
        }

        if (renderer->IsKeyPressed(Renderer::Key::Enter)) {
            if (!startKeyHeld && lobbyClient.isConnected()) {
                lobbyClient.requestStart();
            }
            startKeyHeld = true;
        } else {
            startKeyHeld = false;
        }

        if (renderer->IsKeyPressed(Renderer::Key::Escape)) {
            break;
        }

        renderer->BeginFrame();
        renderer->Clear(Renderer::Color{0.05f, 0.05f, 0.08f, 1.0f});

        engine->UpdateSystems(deltaTime);
        drawPlayerLabels(renderer, lobbyFont, players, visualState, registry, lobbyClient.getMyInfo().number);
        drawInstructions(renderer, lobbyFont, lobbyClient);

        renderer->EndFrame();

        if (lobbyClient.isGameStarted()) {
            break;
        }
    }

    lobbyClient.disconnect();
    renderer->DestroyWindow();
    renderer->Shutdown();
    engine->Shutdown();

    return 0;
}
