#include "LobbyClient.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "ECS/RenderingSystem.hpp"
#include "ECS/TextRenderingSystem.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include <iostream>
#include <memory>
#include <chrono>
#include <unordered_map>

using namespace RType::ECS;

class VisualLobbyClient {
public:
    VisualLobbyClient(const std::string& serverAddr, uint16_t port, const std::string& playerName)
        : m_client(serverAddr, port), m_playerName(playerName) {

        m_renderer = std::make_unique<Renderer::SFMLRenderer>();

        Renderer::WindowConfig config;
        config.title = "R-Type Lobby - " + playerName;
        config.width = 800;
        config.height = 600;
        config.targetFramerate = 60;

        if (!m_renderer->CreateWindow(config)) {
            throw std::runtime_error("Failed to create window");
        }

        m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
        m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());

        m_font = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 14);
        if (m_font == Renderer::INVALID_FONT_ID) {
            std::cerr << "Warning: Could not load font, text won't display properly" << std::endl;
        }

        m_client.onPlayerLeft([this](uint8_t playerNum) {
            removePlayer(playerNum);
        });

        createTitleEntity();
        createInstructionsEntity();
    }

    void run() {
        m_client.connect(m_playerName);

        auto lastTime = std::chrono::steady_clock::now();

        while (m_renderer->IsWindowOpen()) {
            auto currentTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            m_renderer->Update(deltaTime);

            m_client.update();

            updateLobbyState();

            m_renderer->BeginFrame();
            m_renderer->Clear(Math::Color{0.1f, 0.1f, 0.15f, 1.0f});

            m_renderingSystem->Update(m_registry, deltaTime);
            m_textSystem->Update(m_registry, deltaTime);

            m_renderer->EndFrame();

            handleInput();

            if (m_client.isGameStarted()) {
                std::cout << "Game started! Seed: " << m_client.getGameSeed() << std::endl;
                break;
            }
        }
    }

private:
    void createTitleEntity() {
        if (m_font == Renderer::INVALID_FONT_ID) return;
        Entity title = m_registry.CreateEntity();
        m_registry.AddComponent<Position>(title, Position{400.0f, 50.0f});
        m_registry.AddComponent<TextLabel>(title, TextLabel{"R-Type Lobby", m_font});
        auto& titleLabel = m_registry.GetComponent<TextLabel>(title);
        titleLabel.scale = 2.0f;
        titleLabel.color = Math::Color{1.0f, 0.8f, 0.2f, 1.0f};
    }

    void createInstructionsEntity() {
        if (m_font == Renderer::INVALID_FONT_ID) return;
        Entity instructions = m_registry.CreateEntity();
        m_registry.AddComponent<Position>(instructions, Position{50.0f, 500.0f});
        m_registry.AddComponent<TextLabel>(instructions, TextLabel{"Press R to ready | Press S to start game", m_font});
        auto& instrLabel = m_registry.GetComponent<TextLabel>(instructions);
        instrLabel.color = Math::Color{0.7f, 0.7f, 0.7f, 1.0f};
    }

    void updateLobbyState() {
        if (!m_client.isConnected()) {
            updateStatusText("Connecting...");
            return;
        }

        const auto& myInfo = m_client.getMyInfo();
        std::string status = "Connected as Player " + std::to_string(myInfo.number);
        if (myInfo.ready) {
            status += " [READY]";
        } else {
            status += " [Not Ready]";
        }
        updateStatusText(status);

        const auto& players = m_client.getPlayers();
        for (const auto& player : players) {
            updateOrCreatePlayerEntity(player);
        }
    }

    void updateStatusText(const std::string& status) {
        if (m_lastStatus == status) return;
        m_lastStatus = status;

        if (m_font == Renderer::INVALID_FONT_ID) {
            std::cout << "[Status] " << status << std::endl;
            return;
        }
        if (m_statusEntity == NULL_ENTITY) {
            m_statusEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_statusEntity, Position{50.0f, 150.0f});
            m_registry.AddComponent<TextLabel>(m_statusEntity, TextLabel{status, m_font});
        } else {
            auto& label = m_registry.GetComponent<TextLabel>(m_statusEntity);
            label.text = status;
        }
    }

    void updateOrCreatePlayerEntity(const network::PlayerInfo& player) {
        Entity entity;
        auto it = m_playerEntities.find(player.number);

        bool isNewPlayer = false;
        if (it == m_playerEntities.end()) {
            isNewPlayer = true;
            entity = m_registry.CreateEntity();
            m_playerEntities[player.number] = entity;

            float yPos = 250.0f + (player.number * 50.0f);
            m_registry.AddComponent<Position>(entity, Position{50.0f, yPos});
            m_registry.AddComponent<NetworkPlayer>(entity,
                NetworkPlayer{player.number, player.hash, player.name, player.ready});
            if (m_font != Renderer::INVALID_FONT_ID) {
                m_registry.AddComponent<TextLabel>(entity, TextLabel{"", m_font});
            }
        } else {
            entity = it->second;
            auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(entity);
            if (netPlayer.ready == player.ready) return;
            netPlayer.ready = player.ready;
        }

        const auto& netPlayer = m_registry.GetComponent<NetworkPlayer>(entity);
        std::string displayText = "Player " + std::to_string(netPlayer.playerNumber) +
                                  ": " + std::string(netPlayer.name);
        if (netPlayer.ready) {
            displayText += " [READY]";
        }

        if (m_font != Renderer::INVALID_FONT_ID) {
            auto& label = m_registry.GetComponent<TextLabel>(entity);
            label.text = displayText;
            label.color = netPlayer.ready ? Math::Color{0.2f, 1.0f, 0.2f, 1.0f}
                                          : Math::Color{1.0f, 1.0f, 1.0f, 1.0f};
        } else if (isNewPlayer) {
            std::cout << "[Player] " << displayText << std::endl;
        }
    }

    void removePlayer(uint8_t playerNum) {
        auto it = m_playerEntities.find(playerNum);
        if (it != m_playerEntities.end()) {
            m_registry.DestroyEntity(it->second);
            m_playerEntities.erase(it);
        }
    }

    void handleInput() {
        if (m_renderer->IsKeyPressed(Renderer::Key::R) && !m_rKeyPressed) {
            m_rKeyPressed = true;
            if (m_client.isConnected()) {
                m_client.ready();
                std::cout << "Marked as ready!" << std::endl;
            }
        } else if (!m_renderer->IsKeyPressed(Renderer::Key::R)) {
            m_rKeyPressed = false;
        }

        if (m_renderer->IsKeyPressed(Renderer::Key::S) && !m_sKeyPressed) {
            m_sKeyPressed = true;
            if (m_client.isConnected()) {
                m_client.requestStart();
                std::cout << "Requested game start!" << std::endl;
            }
        } else if (!m_renderer->IsKeyPressed(Renderer::Key::S)) {
            m_sKeyPressed = false;
        }

        if (m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
            m_client.disconnect();
        }
    }

    network::LobbyClient m_client;
    std::string m_playerName;

    Registry m_registry;
    std::unique_ptr<Renderer::SFMLRenderer> m_renderer;
    std::unique_ptr<RenderingSystem> m_renderingSystem;
    std::unique_ptr<TextRenderingSystem> m_textSystem;

    Renderer::FontId m_font = Renderer::INVALID_FONT_ID;
    Entity m_statusEntity = NULL_ENTITY;
    std::unordered_map<uint8_t, Entity> m_playerEntities;

    std::string m_lastStatus;
    bool m_rKeyPressed = false;
    bool m_sKeyPressed = false;
};

int main(int argc, char* argv[]) {
    std::string addr = "127.0.0.1";
    uint16_t port = 4242;
    std::string name = "Player";

    if (argc > 1)
        addr = argv[1];
    if (argc > 2)
        port = std::stoi(argv[2]);
    if (argc > 3)
        name = argv[3];

    try {
        VisualLobbyClient client(addr, port, name);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
