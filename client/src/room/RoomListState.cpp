/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RoomListState - Room selection screen implementation
*/

#include "../../include/RoomListState.hpp"
#include "../../include/LobbyState.hpp"
#include "ECS/Components/TextLabel.hpp"
#include <iostream>
#include <algorithm>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        RoomListState::RoomListState(GameStateMachine& machine, GameContext& context)
            : m_machine(machine), m_context(context) {
            m_renderer = context.renderer;
            m_renderingSystem = std::make_unique<RenderingSystem>(m_renderer.get());
            m_textSystem = std::make_unique<TextRenderingSystem>(m_renderer.get());
        }

        void RoomListState::Init() {
            std::cout << "[RoomListState] Initializing room selection..." << std::endl;

            m_fontLarge = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 32);
            if (m_fontLarge == Renderer::INVALID_FONT_ID) {
                m_fontLarge = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 32);
            }

            m_fontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 18);
            if (m_fontMedium == Renderer::INVALID_FONT_ID) {
                m_fontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 18);
            }

            m_fontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 14);
            if (m_fontSmall == Renderer::INVALID_FONT_ID) {
                m_fontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 14);
            }

            m_bgTexture = m_renderer->LoadTexture("assets/backgrounds/1.jpg");
            if (m_bgTexture == Renderer::INVALID_TEXTURE_ID) {
                m_bgTexture = m_renderer->LoadTexture("../assets/backgrounds/1.jpg");
            }

            m_roomClient = std::make_unique<network::RoomClient>(m_context.serverIp, m_context.serverPort);

            if (!m_roomClient->isConnected()) {
                m_hasError = true;
                m_errorMessage = "Failed to connect to server";
            } else {
                m_roomClient->onRoomList([this](const std::vector<network::RoomInfo>& rooms) {
                    m_rooms = rooms;
                    createRoomEntities();
                });

                m_roomClient->onRoomCreated([this](uint32_t roomId) {
                    std::cout << "[RoomListState] Room created! Joining room " << roomId << std::endl;
                    m_roomClient->joinRoom(roomId);
                });

                m_roomClient->onRoomJoined([this](network::JoinRoomStatus status) {
                    if (status == network::JoinRoomStatus::SUCCESS) {
                        std::cout << "[RoomListState] Joined room! Transitioning to lobby..." << std::endl;
                        network::TcpSocket socket = m_roomClient->releaseSocket();
                        m_machine.ChangeState(std::make_unique<LobbyState>(m_machine, m_context, std::move(socket)));
                    } else {
                        m_hasError = true;
                        switch (status) {
                        case network::JoinRoomStatus::ROOM_FULL:
                            m_errorMessage = "Room is full";
                            break;
                        case network::JoinRoomStatus::ROOM_NOT_FOUND:
                            m_errorMessage = "Room not found";
                            break;
                        case network::JoinRoomStatus::ROOM_IN_GAME:
                            m_errorMessage = "Game already in progress";
                            break;
                        default:
                            m_errorMessage = "Failed to join room";
                            break;
                        }
                        m_errorTimer = 3.0f;
                    }
                });

                m_roomClient->onRoomUpdate([this](const network::RoomInfo& room) {
                    bool found = false;
                    for (auto& r : m_rooms) {
                        if (r.id == room.id) {
                            r = room;
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        m_rooms.push_back(room);
                    }
                    createRoomEntities();
                });

                m_roomClient->onConnectionError([this](const std::string& error) {
                    m_hasError = true;
                    m_errorMessage = error;
                    m_errorTimer = 3.0f;
                });

                m_roomClient->requestRooms();
            }

            createUI();
        }

        void RoomListState::Cleanup() {
            std::cout << "[RoomListState] Cleaning up..." << std::endl;
            clearRoomEntities();
            m_roomClient.reset();
        }

        void RoomListState::createUI() {
            if (m_bgTexture != Renderer::INVALID_TEXTURE_ID) {
                m_bgEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_bgEntity, Position{0.0f, 0.0f});

                Renderer::SpriteId spriteId = m_renderer->CreateSprite(m_bgTexture, {});
                Drawable drawable(spriteId, -10);

                Renderer::Vector2 texSize = m_renderer->GetTextureSize(m_bgTexture);
                if (texSize.x > 0 && texSize.y > 0) {
                    drawable.scale = {1280.0f / texSize.x, 720.0f / texSize.y};
                }

                m_registry.AddComponent<Drawable>(m_bgEntity, std::move(drawable));
            }

            if (m_fontLarge == Renderer::INVALID_FONT_ID)
                return;

            m_titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_titleEntity, Position{640.0f, 50.0f});
            TextLabel titleLabel("SELECT A ROOM", m_fontLarge, 32);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            titleLabel.centered = true;
            m_registry.AddComponent<TextLabel>(m_titleEntity, std::move(titleLabel));

            m_instructionsEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_instructionsEntity, Position{640.0f, 670.0f});
            TextLabel instrLabel("[ENTER] Join  |  [C] Create Room  |  [ESC] Back", m_fontSmall, 14);
            instrLabel.color = {0.5f, 0.86f, 1.0f, 0.9f};
            instrLabel.centered = true;
            m_registry.AddComponent<TextLabel>(m_instructionsEntity, std::move(instrLabel));
        }

        void RoomListState::clearRoomEntities() {
            for (Entity e : m_roomEntities) {
                if (m_registry.IsEntityAlive(e)) {
                    m_registry.DestroyEntity(e);
                }
            }
            m_roomEntities.clear();

            if (m_noRoomsEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_noRoomsEntity)) {
                m_registry.DestroyEntity(m_noRoomsEntity);
                m_noRoomsEntity = NULL_ENTITY;
            }
        }

        void RoomListState::createRoomEntities() {
            clearRoomEntities();

            if (m_rooms.empty()) {
                m_noRoomsEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_noRoomsEntity, Position{640.0f, 300.0f});
                TextLabel noRoomsLabel("No rooms available\nPress [C] to create one!", m_fontMedium, 18);
                noRoomsLabel.color = {0.7f, 0.7f, 0.7f, 1.0f};
                noRoomsLabel.centered = true;
                m_registry.AddComponent<TextLabel>(m_noRoomsEntity, std::move(noRoomsLabel));
                return;
            }

            float startY = 140.0f;
            float spacing = 60.0f;

            for (size_t i = 0; i < m_rooms.size(); ++i) {
                const auto& room = m_rooms[i];

                Entity roomEntity = m_registry.CreateEntity();
                m_roomEntities.push_back(roomEntity);

                float y = startY + i * spacing;
                m_registry.AddComponent<Position>(roomEntity, Position{640.0f, y});

                std::string prefix = (static_cast<int>(i) == m_selectedIndex) ? "> " : "  ";
                std::string status = room.inGame ? "[IN GAME]" : "[WAITING]";
                std::string playerCount = std::to_string(room.playerCount) + "/" + std::to_string(room.maxPlayers);

                std::string roomName(room.name);
                if (roomName.length() > 16) {
                    roomName = roomName.substr(0, 16);
                }
                while (roomName.length() < 16) {
                    roomName += " ";
                }

                std::string text = prefix + roomName + "  " + playerCount + "  " + status;

                TextLabel roomLabel(text, m_fontMedium, 18);

                if (static_cast<int>(i) == m_selectedIndex) {
                    if (room.inGame) {
                        roomLabel.color = {0.5f, 0.5f, 0.5f, 1.0f};
                    } else {
                        roomLabel.color = {1.0f, 0.08f, 0.58f, 1.0f};
                    }
                } else {
                    if (room.inGame) {
                        roomLabel.color = {0.4f, 0.4f, 0.4f, 0.8f};
                    } else {
                        roomLabel.color = {0.8f, 0.8f, 0.8f, 1.0f};
                    }
                }

                roomLabel.centered = true;
                m_registry.AddComponent<TextLabel>(roomEntity, std::move(roomLabel));
            }
        }

        void RoomListState::handleCreateRoom() {
            std::string roomName = m_context.playerName + "'s Room";
            std::cout << "[RoomListState] Creating room: " << roomName << std::endl;
            m_roomClient->createRoom(roomName);
        }

        void RoomListState::handleJoinRoom() {
            if (m_rooms.empty() || m_selectedIndex < 0 || m_selectedIndex >= static_cast<int>(m_rooms.size())) {
                return;
            }

            const auto& room = m_rooms[m_selectedIndex];

            if (room.inGame) {
                m_hasError = true;
                m_errorMessage = "Cannot join - game in progress";
                m_errorTimer = 2.0f;
                return;
            }

            if (room.playerCount >= room.maxPlayers) {
                m_hasError = true;
                m_errorMessage = "Cannot join - room is full";
                m_errorTimer = 2.0f;
                return;
            }

            std::cout << "[RoomListState] Joining room " << room.id << " (" << room.name << ")" << std::endl;
            m_roomClient->joinRoom(room.id);
        }

        void RoomListState::HandleInput() {
            if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                m_upKeyPressed = true;
                if (!m_rooms.empty()) {
                    m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_rooms.size())) % static_cast<int>(m_rooms.size());
                    createRoomEntities();
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_upKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                m_downKeyPressed = true;
                if (!m_rooms.empty()) {
                    m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_rooms.size());
                    createRoomEntities();
                }
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_downKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_enterKeyPressed) {
                m_enterKeyPressed = true;
                handleJoinRoom();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                m_enterKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::C) && !m_cKeyPressed) {
                m_cKeyPressed = true;
                handleCreateRoom();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::C)) {
                m_cKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;
                std::cout << "[RoomListState] Returning to menu..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }
        }

        void RoomListState::Update(float dt) {
            if (m_roomClient) {
                m_roomClient->update();
            }

            m_refreshTimer += dt;
            if (m_refreshTimer >= REFRESH_INTERVAL) {
                m_refreshTimer = 0.0f;
                if (m_roomClient && m_roomClient->isConnected()) {
                    m_roomClient->requestRooms();
                }
            }

            if (m_hasError) {
                m_errorTimer -= dt;
                if (m_errorTimer <= 0.0f) {
                    m_hasError = false;
                    m_errorMessage.clear();

                    if (m_errorEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_errorEntity)) {
                        m_registry.DestroyEntity(m_errorEntity);
                        m_errorEntity = NULL_ENTITY;
                    }
                }
            }

            if (m_hasError && m_errorEntity == NULL_ENTITY) {
                m_errorEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_errorEntity, Position{640.0f, 620.0f});
                TextLabel errorLabel(m_errorMessage, m_fontSmall, 14);
                errorLabel.color = {1.0f, 0.3f, 0.3f, 1.0f}; // Red
                errorLabel.centered = true;
                m_registry.AddComponent<TextLabel>(m_errorEntity, std::move(errorLabel));
            }
        }

        void RoomListState::Draw() {
            m_renderer->Clear({0.05f, 0.05f, 0.1f, 1.0f});
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);
        }

    }
}
