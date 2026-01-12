/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - Update and input handling functions
*/

#include "../../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include "ResultsState.hpp"
#include "RoomListState.hpp"
#include <cmath>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        void InGameState::HandleInput() {
            if (m_isGameOver) {
                if (m_renderer->IsKeyPressed(Renderer::Key::Enter) && !m_gameOverEnterPressed) {
                    m_gameOverEnterPressed = true;
                    enterResultsScreen();
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Enter)) {
                    m_gameOverEnterPressed = false;
                }

                if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_gameOverEscapePressed) {
                    m_gameOverEscapePressed = true;
                    if (m_context.networkClient) {
                        m_context.networkClient->Stop();
                        m_context.networkClient.reset();
                    }
                    m_machine.ChangeState(std::make_unique<RoomListState>(m_machine, m_context));
                } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                    m_gameOverEscapePressed = false;
                }
                return;
            }

            m_currentInputs = 0;

            if (m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_currentInputs |= network::InputFlags::UP;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_currentInputs |= network::InputFlags::DOWN;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                m_currentInputs |= network::InputFlags::LEFT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                m_currentInputs |= network::InputFlags::RIGHT;
            }
            if (m_renderer->IsKeyPressed(Renderer::Key::Space)) {
                if (!m_isCharging) {
                    m_isCharging = true;
                    m_chargeTime = 0.0f;
                }
            } else if (m_isCharging) {
                m_currentInputs |= network::InputFlags::SHOOT;
                m_isCharging = false;
                m_chargeTime = 0.0f;
            }

            if (!m_isNetworkSession &&
                m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.HasComponent<ShootCommand>(m_localPlayerEntity)) {
                auto& shootCmd = m_registry.GetComponent<ShootCommand>(m_localPlayerEntity);
                shootCmd.wantsToShoot = (m_currentInputs & network::InputFlags::SHOOT);
            }
            if (m_context.networkClient && m_currentInputs != m_previousInputs) {
                auto now = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastInputTime).count();

                if (ms >= 30) {
                    m_context.networkClient->SendInput(static_cast<uint8_t>(m_currentInputs));
                    m_lastInputTime = now;

                    static int inputLog = 0;
                    if (inputLog++ % 10 == 0) {
                        auto epoch = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
                        std::cout << "[CLIENT SEND INPUT] t=" << epoch << " inputs=" << m_currentInputs << std::endl;
                    }
                }
                m_previousInputs = m_currentInputs;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Escape) && !m_escapeKeyPressed) {
                m_escapeKeyPressed = true;
                std::cout << "[GameState] Returning to lobby..." << std::endl;
                m_machine.PopState();
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Escape)) {
                m_escapeKeyPressed = false;
            }
        }

        void InGameState::Update(float dt) {
            if (m_context.networkClient) {
                m_context.networkClient->ReceivePackets();
            } else {
                m_scoreAccumulator += dt;
                if (m_scoreAccumulator >= 10.0f) {
                    uint32_t intervals = static_cast<uint32_t>(m_scoreAccumulator / 10.0f);
                    m_playerScore += intervals * 10;
                    m_scoreAccumulator -= static_cast<float>(intervals) * 10.0f;
                }
            }

            if (m_isCharging) {
                m_chargeTime += dt;
                if (m_chargeTime > MAX_CHARGE_TIME) {
                    m_chargeTime = MAX_CHARGE_TIME;
                }
            }

            triggerGameOverIfNeeded();
            if (m_isGameOver) {
                m_gameOverElapsed += dt;
                updateHUD();
                return;
            }
            if (m_inputSystem) {
                m_inputSystem->Update(m_registry, dt);
            }
            if (m_movementSystem) {
                m_movementSystem->Update(m_registry, dt);
            }

            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto entity : entities) {
                if (m_registry.HasComponent<Velocity>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    if (entity == m_localPlayerEntity) {
                        pos.x = std::max(0.0f, std::min(pos.x, 1280.0f - 66.0f));
                        pos.y = std::max(0.0f, std::min(pos.y, 720.0f - 32.0f));
                    }
                    UpdatePlayerNameLabelPosition(entity, pos.x, pos.y);
                }
            }

            if (m_collisionDetectionSystem) {
                m_collisionDetectionSystem->Update(m_registry, dt);
            }
            if (m_bulletResponseSystem) {
                m_bulletResponseSystem->Update(m_registry, dt);
            }
            if (m_playerResponseSystem) {
                m_playerResponseSystem->Update(m_registry, dt);
            }
            if (m_obstacleResponseSystem) {
                m_obstacleResponseSystem->Update(m_registry, dt);
            }
            if (m_scoreSystem) {
                m_scoreSystem->Update(m_registry, dt);
            }
            if (m_healthSystem) {
                m_healthSystem->Update(m_registry, dt);
            }

            if (m_scrollingSystem) {
                m_scrollingSystem->Update(m_registry, dt);
            }
            m_localScrollOffset += -150.0f * dt;

            if (m_shieldSystem) {
                m_shieldSystem->Update(m_registry, dt);
            }
            if (m_forcePodSystem) {
                m_forcePodSystem->Update(m_registry, dt);
            }

            if (m_shootingSystem) {
                if (m_context.networkClient) {
                    m_shootingSystem->Update(m_registry, dt);
                } else {
                    m_shootingSystem->Update(m_registry, dt);
                }
            }

            for (auto& bg : m_backgroundEntities) {
                if (!m_registry.HasComponent<Position>(bg))
                    continue;
                auto& pos = m_registry.GetComponent<Position>(bg);
                if (pos.x <= -1280.0f) {
                    pos.x = pos.x + 3 * 1280.0f;
                }
            }

            updateHUD();
        }

        void InGameState::triggerGameOverIfNeeded() {
            if (m_isGameOver) {
                return;
            }

            bool dead = false;
            if (m_isNetworkSession) {
                if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                    size_t idx = static_cast<size_t>(m_context.playerNumber - 1);
                    dead = m_playersHUD[idx].active && m_playersHUD[idx].isDead;
                }
            } else {
                if (m_localPlayerEntity == ECS::NULL_ENTITY || !m_registry.IsEntityAlive(m_localPlayerEntity)) {
                    dead = true;
                } else if (m_registry.HasComponent<Health>(m_localPlayerEntity)) {
                    const auto& h = m_registry.GetComponent<Health>(m_localPlayerEntity);
                    dead = (h.current <= 0);
                }
            }

            if (!dead) {
                return;
            }

            m_isGameOver = true;
            m_gameOverElapsed = 0.0f;
            m_isCharging = false;
            m_chargeTime = 0.0f;

            if (m_gameOverTitleEntity == NULL_ENTITY && m_gameOverFontLarge != Renderer::INVALID_FONT_ID) {
                m_gameOverTitleEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_gameOverTitleEntity, Position{640.0f, 260.0f});
                TextLabel title("GAME OVER", m_gameOverFontLarge, 56);
                title.centered = true;
                title.color = {1.0f, 0.08f, 0.58f, 1.0f};
                m_registry.AddComponent<TextLabel>(m_gameOverTitleEntity, std::move(title));
            }

            if (m_gameOverScoreEntity == NULL_ENTITY && m_gameOverFontMedium != Renderer::INVALID_FONT_ID) {
                m_gameOverScoreEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_gameOverScoreEntity, Position{640.0f, 360.0f});
                TextLabel score("", m_gameOverFontMedium, 22);
                score.centered = true;
                score.color = {0.5f, 0.86f, 1.0f, 0.95f};
                m_registry.AddComponent<TextLabel>(m_gameOverScoreEntity, std::move(score));
            }

            if (m_gameOverHintEntity == NULL_ENTITY && m_hudFontSmall != Renderer::INVALID_FONT_ID) {
                m_gameOverHintEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(m_gameOverHintEntity, Position{640.0f, 430.0f});
                TextLabel hint("Press ENTER to view results  |  ESC to return to lobby", m_hudFontSmall, 14);
                hint.centered = true;
                hint.color = {0.5f, 0.86f, 1.0f, 0.85f};
                m_registry.AddComponent<TextLabel>(m_gameOverHintEntity, std::move(hint));
            }

            if (m_gameOverScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_gameOverScoreEntity)) {
                auto& label = m_registry.GetComponent<TextLabel>(m_gameOverScoreEntity);
                label.text = "SCORE " + std::to_string(m_playerScore);
            }
        }

        void InGameState::Cleanup() {
            std::cout << "[GameState] Cleaning up game state..." << std::endl;

            for (auto& bg : m_backgroundEntities) {
                if (m_registry.IsEntityAlive(bg)) {
                    m_registry.DestroyEntity(bg);
                }
            }
            m_backgroundEntities.clear();

            for (auto& obstacle : m_obstacleSpriteEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleSpriteEntities.clear();

            for (auto& obstacle : m_obstacleColliderEntities) {
                if (m_registry.IsEntityAlive(obstacle)) {
                    m_registry.DestroyEntity(obstacle);
                }
            }
            m_obstacleColliderEntities.clear();
            m_obstacleIdToCollider.clear();

            if (m_hudPlayerEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudPlayerEntity)) {
                m_registry.DestroyEntity(m_hudPlayerEntity);
            }
            if (m_hudScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreEntity)) {
                m_registry.DestroyEntity(m_hudScoreEntity);
            }
            if (m_hudLivesEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudLivesEntity)) {
                m_registry.DestroyEntity(m_hudLivesEntity);
            }
            if (m_hudScoreboardTitle != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreboardTitle)) {
                m_registry.DestroyEntity(m_hudScoreboardTitle);
            }
            for (auto& playerHUD : m_playersHUD) {
                if (playerHUD.scoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(playerHUD.scoreEntity)) {
                    m_registry.DestroyEntity(playerHUD.scoreEntity);
                }
            }

            for (auto& [playerEntity, labelEntity] : m_playerNameLabels) {
                if (m_registry.IsEntityAlive(labelEntity)) {
                    m_registry.DestroyEntity(labelEntity);
                }
            }
            m_playerNameLabels.clear();
        }

        void InGameState::enterResultsScreen() {
            std::vector<std::pair<std::string, uint32_t>> scores;
            scores.reserve(MAX_PLAYERS);

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                if (!m_playersHUD[i].active) {
                    continue;
                }
                uint8_t playerNum = static_cast<uint8_t>(i + 1);
                std::string name = "P" + std::to_string(playerNum);
                for (const auto& p : m_context.allPlayers) {
                    if (p.number == playerNum && p.name[0] != '\0') {
                        name = std::string(p.name);
                        break;
                    }
                }
                scores.push_back({name, m_playersHUD[i].score});
            }

            if (!m_isNetworkSession && scores.empty()) {
                std::string name = m_context.playerName.empty() ? "P1" : m_context.playerName;
                scores.push_back({name, m_playerScore});
            }

            if (m_context.networkClient) {
                m_context.networkClient->Stop();
                m_context.networkClient.reset();
            }

            m_machine.ChangeState(std::make_unique<ResultsState>(m_machine, m_context, std::move(scores)));
        }

    }
}
