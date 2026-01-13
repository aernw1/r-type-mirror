/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState - UI and HUD rendering functions
*/

#include "../../include/GameState.hpp"

#include "ECS/Components/TextLabel.hpp"
#include "ECS/Component.hpp"
#include "Core/Logger.hpp"
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace RType::ECS;

namespace RType {
    namespace Client {

        void InGameState::initializeUI() {
            m_hudFont = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 16);
            if (m_hudFont == Renderer::INVALID_FONT_ID) {
                m_hudFont = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 16);
            }

            m_hudFontSmall = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 12);
            if (m_hudFontSmall == Renderer::INVALID_FONT_ID) {
                m_hudFontSmall = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 12);
            }

            m_gameOverFontLarge = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 48);
            if (m_gameOverFontLarge == Renderer::INVALID_FONT_ID) {
                m_gameOverFontLarge = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 48);
            }

            m_gameOverFontMedium = m_renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 20);
            if (m_gameOverFontMedium == Renderer::INVALID_FONT_ID) {
                m_gameOverFontMedium = m_renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 20);
            }

            if (m_hudFont == Renderer::INVALID_FONT_ID) {
                std::cerr << "[GameState] Error: Failed to load HUD font!" << std::endl;
                return;
            }

            m_hudPlayerEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudPlayerEntity, Position{20.0f, 680.0f});
            std::string playerText = "P" + std::to_string(m_context.playerNumber);
            TextLabel playerLabel(playerText, m_hudFont, 16);
            playerLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudPlayerEntity, std::move(playerLabel));

            m_hudLivesEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudLivesEntity, Position{350.0f, 680.0f});
            TextLabel livesLabel("LIVES x3", m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
            livesLabel.color = {1.0f, 0.8f, 0.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudLivesEntity, std::move(livesLabel));

            m_hudScoreboardTitle = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_hudScoreboardTitle, Position{1050.0f, 620.0f});
            TextLabel titleLabel("SCORES", m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            m_registry.AddComponent<TextLabel>(m_hudScoreboardTitle, std::move(titleLabel));

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                m_playersHUD[i].active = false;
                m_playersHUD[i].score = 0;
                m_playersHUD[i].lives = 3;
                m_playersHUD[i].health = 100;
                m_playersHUD[i].maxHealth = 100;
                m_playersHUD[i].playerEntity = NULL_ENTITY;

                m_playersHUD[i].scoreEntity = m_registry.CreateEntity();
                float yPos = 645.0f + (i * 20.0f);
                m_registry.AddComponent<Position>(m_playersHUD[i].scoreEntity, Position{1050.0f, yPos});

                std::string scoreText = "P" + std::to_string(i + 1) + " --------";
                TextLabel label(scoreText, m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12);
                label.color = {0.4f, 0.4f, 0.4f, 0.6f};
                m_registry.AddComponent<TextLabel>(m_playersHUD[i].scoreEntity, std::move(label));
            }

            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                m_playersHUD[m_context.playerNumber - 1].active = true;
            }

            std::cout << "[GameState] HUD initialized for P" << (int)m_context.playerNumber << std::endl;
        }

        void InGameState::updateHUD() {
            if (m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.IsEntityAlive(m_localPlayerEntity) &&
                m_registry.HasComponent<ScoreValue>(m_localPlayerEntity)) {
                const auto& scoreComp = m_registry.GetComponent<ScoreValue>(m_localPlayerEntity);
                m_playerScore = scoreComp.points;
            }

            if (m_hudScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreEntity)) {
                auto& scoreLabel = m_registry.GetComponent<TextLabel>(m_hudScoreEntity);
                std::ostringstream ss;
                ss << std::setw(8) << std::setfill('0') << m_playerScore;
                scoreLabel.text = ss.str();
            }

            if (m_hudLivesEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudLivesEntity)) {
                auto& livesLabel = m_registry.GetComponent<TextLabel>(m_hudLivesEntity);
                livesLabel.text = "LIVES x" + std::to_string(m_playerLives);

                if (m_playerLives <= 1) {
                    livesLabel.color = {1.0f, 0.2f, 0.2f, 1.0f};
                } else if (m_playerLives <= 2) {
                    livesLabel.color = {1.0f, 0.6f, 0.0f, 1.0f};
                } else {
                    livesLabel.color = {1.0f, 0.8f, 0.0f, 1.0f};
                }
            }

            if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                m_playersHUD[m_context.playerNumber - 1].score = m_playerScore;
                m_playersHUD[m_context.playerNumber - 1].lives = m_playerLives;
            }

            const Math::Color playerColors[MAX_PLAYERS] = {
                {0.2f, 1.0f, 0.2f, 1.0f}, // P1 - green
                {0.2f, 0.6f, 1.0f, 1.0f}, // P2 - blue
                {1.0f, 0.3f, 0.3f, 1.0f}, // P3 - Red
                {1.0f, 1.0f, 0.2f, 1.0f}  // P4 - yellow
            };

            for (size_t i = 0; i < MAX_PLAYERS; i++) {
                if (m_playersHUD[i].scoreEntity == NULL_ENTITY ||
                    !m_registry.IsEntityAlive(m_playersHUD[i].scoreEntity)) {
                    continue;
                }

                auto& label = m_registry.GetComponent<TextLabel>(m_playersHUD[i].scoreEntity);

                if (m_playersHUD[i].active) {
                    if (m_playersHUD[i].isDead) {
                        m_playersHUD[i].health = 0;
                    } else {
                        bool entityExists = false;
                        if (m_playersHUD[i].playerEntity != NULL_ENTITY &&
                            m_registry.IsEntityAlive(m_playersHUD[i].playerEntity) &&
                            m_registry.HasComponent<Health>(m_playersHUD[i].playerEntity)) {
                            const auto& health = m_registry.GetComponent<Health>(m_playersHUD[i].playerEntity);
                            if (health.current > 0) {
                                m_playersHUD[i].health = health.current;
                                m_playersHUD[i].maxHealth = health.max;
                            } else {
                                m_playersHUD[i].isDead = true;
                                m_playersHUD[i].health = 0;
                            }
                            if (m_registry.HasComponent<ScoreValue>(m_playersHUD[i].playerEntity)) {
                                const auto& scoreComp = m_registry.GetComponent<ScoreValue>(m_playersHUD[i].playerEntity);
                                m_playersHUD[i].score = scoreComp.points;
                            }
                            entityExists = true;
                        } else if (i == static_cast<size_t>(m_context.playerNumber - 1) && m_localPlayerEntity != ECS::NULL_ENTITY && m_registry.IsEntityAlive(m_localPlayerEntity) && m_registry.HasComponent<Health>(m_localPlayerEntity)) {
                            const auto& health = m_registry.GetComponent<Health>(m_localPlayerEntity);
                            if (health.current > 0) {
                                m_playersHUD[i].health = health.current;
                                m_playersHUD[i].maxHealth = health.max;
                            } else {
                                m_playersHUD[i].isDead = true;
                                m_playersHUD[i].health = 0;
                            }
                            if (m_registry.HasComponent<ScoreValue>(m_localPlayerEntity)) {
                                const auto& scoreComp = m_registry.GetComponent<ScoreValue>(m_localPlayerEntity);
                                m_playersHUD[i].score = scoreComp.points;
                            }
                            entityExists = true;

                            if (m_playersHUD[i].playerEntity == NULL_ENTITY) {
                                m_playersHUD[i].playerEntity = m_localPlayerEntity;
                            }
                        }

                        if (!entityExists) {
                            if (!m_isNetworkSession) {
                                m_playersHUD[i].isDead = true;
                                m_playersHUD[i].health = 0;
                            }
                        }
                    }

                    std::string playerDisplayName = "P" + std::to_string(i + 1);
                    uint8_t playerNum = static_cast<uint8_t>(i + 1);

                    for (const auto& p : m_context.allPlayers) {
                        if (p.number == playerNum && p.name[0] != '\0') {
                            playerDisplayName = std::string(p.name);
                            break;
                        }
                    }

                    if (playerDisplayName == "P" + std::to_string(i + 1)) {
                        auto nameIt = m_playerNameMap.find(static_cast<uint64_t>(playerNum));
                        if (nameIt != m_playerNameMap.end() && !nameIt->second.empty()) {
                            playerDisplayName = nameIt->second;
                        }
                    }

                    size_t originalLength = playerDisplayName.length();
                    bool nameTooLong = originalLength > 16;

                    if (nameTooLong) {
                        playerDisplayName = playerDisplayName.substr(0, 16);
                    }

                    if (nameTooLong && m_registry.HasComponent<Position>(m_playersHUD[i].scoreEntity)) {
                        auto& pos = m_registry.GetComponent<Position>(m_playersHUD[i].scoreEntity);
                        float shiftAmount = static_cast<float>(originalLength - 16) * 38.0f;
                        pos.x = 1050.0f - shiftAmount;
                    } else if (m_registry.HasComponent<Position>(m_playersHUD[i].scoreEntity)) {
                        auto& pos = m_registry.GetComponent<Position>(m_playersHUD[i].scoreEntity);
                        pos.x = 1050.0f;
                    }

                    std::ostringstream ss;
                    ss << playerDisplayName << " " << std::setw(8) << std::setfill('0') << m_playersHUD[i].score;
                    label.text = ss.str();
                    label.color = playerColors[i];

                    if (i == static_cast<size_t>(m_context.playerNumber - 1)) {
                        label.color.a = 1.0f;
                    } else {
                        label.color.a = 0.85f;
                    }
                } else {
                    label.text = "P" + std::to_string(i + 1) + " --------";
                    label.color = {0.4f, 0.4f, 0.4f, 0.5f};
                }
            }
        }

        void InGameState::Draw() {
            m_renderingSystem->Update(m_registry, 0.0f);
            m_textSystem->Update(m_registry, 0.0f);

            renderChargeBar();
            renderHealthBars();
            renderBossHealthBar();
            renderGameOverOverlay();

            // DEBUG: Visualize obstacle colliders
            renderDebugColliders();
        }

        void InGameState::renderDebugColliders() {
            static bool loggedVisualization = false;
            static int frameCount = 0;
            frameCount++;

            if (!loggedVisualization) {
                std::cout << "[DEBUG VIZ] m_obstacleSpriteEntities.size()=" << m_obstacleSpriteEntities.size() << std::endl;
                std::cout << "[DEBUG VIZ] m_obstacleColliderEntities.size()=" << m_obstacleColliderEntities.size() << std::endl;

                // Log all obstacle entities found in registry
                auto allObstacles = m_registry.GetEntitiesWithComponent<Obstacle>();
                std::cout << "[DEBUG VIZ] Total Obstacle entities in registry: " << allObstacles.size() << std::endl;

                int logged = 0;
                for (auto entity : allObstacles) {
                    if (!m_registry.IsEntityAlive(entity)) continue;
                    if (!m_registry.HasComponent<Position>(entity) || !m_registry.HasComponent<BoxCollider>(entity)) continue;

                    const auto& pos = m_registry.GetComponent<Position>(entity);
                    const auto& box = m_registry.GetComponent<BoxCollider>(entity);

                    bool isTracked = false;
                    for (auto& tracked : m_obstacleColliderEntities) {
                        if (tracked == entity) { isTracked = true; break; }
                    }

                    std::cout << "  Obstacle " << entity << ": pos=(" << pos.x << "," << pos.y
                              << ") size=(" << box.width << "," << box.height << ")"
                              << " tracked=" << (isTracked ? "YES" : "NO") << std::endl;

                    if (++logged >= 10) {
                        std::cout << "  ... (showing first 10 only)" << std::endl;
                        break;
                    }
                }

                loggedVisualization = true;
            }

            // Draw obstacle visual sprite positions (green dots)
            for (auto& visual : m_obstacleSpriteEntities) {
                if (!m_registry.IsEntityAlive(visual))
                    continue;
                if (!m_registry.HasComponent<Position>(visual))
                    continue;

                const auto& pos = m_registry.GetComponent<Position>(visual);

                // Draw a small green circle at the visual entity's position
                Renderer::Rectangle marker;
                marker.position = Renderer::Vector2(pos.x - 3, pos.y - 3);
                marker.size = Renderer::Vector2(6, 6);
                m_renderer->DrawRectangle(marker, Renderer::Color(0.0f, 1.0f, 0.0f, 0.9f));
            }

            // Draw ALL entities with Obstacle component and BoxCollider
            // This will catch any colliders not in m_obstacleColliderEntities
            auto allObstacles = m_registry.GetEntitiesWithComponent<Obstacle>();
            for (auto entity : allObstacles) {
                if (!m_registry.IsEntityAlive(entity))
                    continue;
                if (!m_registry.HasComponent<Position>(entity) ||
                    !m_registry.HasComponent<BoxCollider>(entity))
                    continue;

                const auto& pos = m_registry.GetComponent<Position>(entity);
                const auto& box = m_registry.GetComponent<BoxCollider>(entity);

                // Check if this entity is in our tracked list
                bool isTracked = false;
                for (auto& tracked : m_obstacleColliderEntities) {
                    if (tracked == entity) {
                        isTracked = true;
                        break;
                    }
                }

                // Use different colors for tracked vs untracked colliders
                Renderer::Color borderColor;
                Renderer::Color fillColor;
                if (isTracked) {
                    // Tracked colliders: yellow border, red fill
                    borderColor = Renderer::Color(1.0f, 1.0f, 0.0f, 0.6f);
                    fillColor = Renderer::Color(1.0f, 0.0f, 0.0f, 0.4f);
                } else {
                    // UNTRACKED colliders: magenta border, blue fill (THESE ARE THE INVISIBLE ONES!)
                    borderColor = Renderer::Color(1.0f, 0.0f, 1.0f, 0.9f);
                    fillColor = Renderer::Color(0.0f, 0.0f, 1.0f, 0.6f);
                }

                // Draw border
                Renderer::Rectangle borderRect;
                borderRect.position = Renderer::Vector2(pos.x - 2, pos.y - 2);
                borderRect.size = Renderer::Vector2(box.width + 4, box.height + 4);
                m_renderer->DrawRectangle(borderRect, borderColor);

                // Draw collider box
                Renderer::Rectangle rect;
                rect.position = Renderer::Vector2(pos.x, pos.y);
                rect.size = Renderer::Vector2(box.width, box.height);
                m_renderer->DrawRectangle(rect, fillColor);

                // If this collider has metadata, draw a cyan dot
                if (m_registry.HasComponent<ObstacleMetadata>(entity)) {
                    const auto& metadata = m_registry.GetComponent<ObstacleMetadata>(entity);

                    if (metadata.visualEntity != ECS::NULL_ENTITY &&
                        m_registry.IsEntityAlive(metadata.visualEntity) &&
                        m_registry.HasComponent<Position>(metadata.visualEntity)) {

                        float colliderCenterX = pos.x + box.width / 2;
                        float colliderCenterY = pos.y + box.height / 2;

                        // Draw a cyan dot at collider center to show the link
                        Renderer::Rectangle linkMarker;
                        linkMarker.position = Renderer::Vector2(colliderCenterX - 2, colliderCenterY - 2);
                        linkMarker.size = Renderer::Vector2(4, 4);
                        m_renderer->DrawRectangle(linkMarker, Renderer::Color(0.0f, 1.0f, 1.0f, 0.8f));
                    }
                }
            }
        }

        void InGameState::renderChargeBar() {
            const float barX = 500.0f;
            const float barY = 675.0f;
            const float barWidth = 200.0f;
            const float barHeight = 20.0f;

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(barX - 2, barY - 2);
            bgRect.size = Renderer::Vector2(barWidth + 4, barHeight + 4);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.2f, 0.2f, 0.2f, 0.8f));

            Renderer::Rectangle innerBgRect;
            innerBgRect.position = Renderer::Vector2(barX, barY);
            innerBgRect.size = Renderer::Vector2(barWidth, barHeight);
            m_renderer->DrawRectangle(innerBgRect, Renderer::Color(0.1f, 0.1f, 0.1f, 0.9f));

            float chargePercent = m_chargeTime / MAX_CHARGE_TIME;
            float filledWidth = barWidth * chargePercent;

            Renderer::Color barColor;
            if (chargePercent < 0.5f) {
                float blend = chargePercent * 2.0f;
                barColor = Renderer::Color(blend, 1.0f, 1.0f - blend, 1.0f);
            } else {
                float blend = (chargePercent - 0.5f) * 2.0f;
                barColor = Renderer::Color(1.0f, 1.0f - blend * 0.5f, 0.0f, 1.0f);
            }

            if (filledWidth > 0) {
                Renderer::Rectangle fillRect;
                fillRect.position = Renderer::Vector2(barX, barY);
                fillRect.size = Renderer::Vector2(filledWidth, barHeight);
                m_renderer->DrawRectangle(fillRect, barColor);
            }

            if (m_isCharging && m_hudFontSmall != Renderer::INVALID_FONT_ID) {
                Renderer::TextParams textParams;
                textParams.position = Renderer::Vector2(barX + barWidth + 10, barY + 2);
                textParams.color = Renderer::Color(0.0f, 1.0f, 1.0f, 1.0f);
                textParams.scale = 1.0f;

                if (m_chargeTime >= MAX_CHARGE_TIME) {
                    float pulse = 0.7f + 0.3f * std::sin(m_chargeTime * 10.0f);
                    textParams.color = Renderer::Color(1.0f, pulse, 0.0f, 1.0f);
                    m_renderer->DrawText(m_hudFontSmall, "MAX!", textParams);
                } else {
                    m_renderer->DrawText(m_hudFontSmall, "BEAM", textParams);
                }
            }
        }

        void InGameState::renderHealthBars() {
            if (m_context.playerNumber < 1 || m_context.playerNumber > MAX_PLAYERS) {
                return;
            }

            size_t playerIndex = static_cast<size_t>(m_context.playerNumber - 1);
            if (!m_playersHUD[playerIndex].active) {
                return;
            }

            const float barWidth = 150.0f;
            const float barHeight = 12.0f;
            const float barX = 180.0f;
            const float barY = 680.0f;

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(barX - 2, barY - 2);
            bgRect.size = Renderer::Vector2(barWidth + 4, barHeight + 4);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.1f, 0.1f, 0.1f, 0.9f));

            Renderer::Rectangle innerBgRect;
            innerBgRect.position = Renderer::Vector2(barX, barY);
            innerBgRect.size = Renderer::Vector2(barWidth, barHeight);
            m_renderer->DrawRectangle(innerBgRect, Renderer::Color(0.3f, 0.1f, 0.1f, 0.8f));

            Renderer::TextParams textParams;
            textParams.position = Renderer::Vector2(barX, barY - barHeight - 2);
            textParams.color = Renderer::Color(0.0f, 1.0f, 1.0f, 1.0f);
            textParams.scale = 1.0f;
            m_renderer->DrawText(m_hudFontSmall, "HEALTH", textParams);

            float healthPercent = 0.0f;
            if (m_playersHUD[playerIndex].isDead) {
                healthPercent = 0.0f;
            } else if (m_playersHUD[playerIndex].maxHealth > 0) {
                healthPercent = static_cast<float>(m_playersHUD[playerIndex].health) / static_cast<float>(m_playersHUD[playerIndex].maxHealth);
                healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));
            }

            if (m_playersHUD[playerIndex].isDead || m_playersHUD[playerIndex].health <= 0) {
                healthPercent = 0.0f;
            }

            float filledWidth = barWidth * healthPercent;

            Renderer::Color healthColor;
            if (healthPercent <= 0.3f) {
                healthColor = Renderer::Color(1.0f, 0.2f, 0.2f, 1.0f);
            } else if (healthPercent <= 0.6f) {
                float blend = (healthPercent - 0.3f) / 0.3f;
                healthColor = Renderer::Color(1.0f, 0.2f + blend * 0.8f, 0.2f, 1.0f);
            } else {
                healthColor = Renderer::Color(0.2f, 1.0f, 0.2f, 1.0f);
            }

            if (filledWidth > 0) {
                Renderer::Rectangle fillRect;
                fillRect.position = Renderer::Vector2(barX, barY);
                fillRect.size = Renderer::Vector2(filledWidth, barHeight);
                m_renderer->DrawRectangle(fillRect, healthColor);
            }
        }

        void InGameState::renderGameOverOverlay() {
            if (!m_isGameOver) {
                return;
            }

            Renderer::Rectangle rect;
            rect.position = Renderer::Vector2(0.0f, 0.0f);
            rect.size = Renderer::Vector2(1280.0f, 720.0f);
            m_renderer->DrawRectangle(rect, Renderer::Color(0.0f, 0.0f, 0.0f, 0.45f));
        }

        void InGameState::initializeBossHealthBar() {
            if (m_bossHealthBar.active) {
                return;
            }

            m_bossHealthBar.active = true;

            m_bossHealthBar.titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_bossHealthBar.titleEntity, Position{640.0f - 80.0f, 10.0f});
            m_registry.AddComponent<TextLabel>(m_bossHealthBar.titleEntity,
                TextLabel("BOSS - LEVEL 1", m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12));

            Core::Logger::Info("[GameState] Boss health bar initialized");
        }

        void InGameState::destroyBossHealthBar() {
            if (!m_bossHealthBar.active) {
                return;
            }

            if (m_bossHealthBar.titleEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_bossHealthBar.titleEntity)) {
                m_registry.DestroyEntity(m_bossHealthBar.titleEntity);
            }

            m_bossHealthBar.active = false;
            m_bossHealthBar.titleEntity = NULL_ENTITY;
            m_bossHealthBar.bossNetworkId = 0;

            Core::Logger::Info("[GameState] Boss health bar destroyed");
        }

        void InGameState::updateBossHealthBar() {
            if (!m_bossHealthBar.active) {
                return;
            }

            float healthPercent = static_cast<float>(m_bossHealthBar.currentHealth) /
                                 static_cast<float>(m_bossHealthBar.maxHealth);
            if (healthPercent < 0.0f) healthPercent = 0.0f;
            if (healthPercent > 1.0f) healthPercent = 1.0f;

            if (m_bossHealthBar.currentHealth <= 0) {
                destroyBossHealthBar();
            }
        }

        void InGameState::renderBossHealthBar() {
            if (!m_bossHealthBar.active) {
                return;
            }

            const float barWidth = 400.0f;
            const float barHeight = 15.0f;
            const float barX = 440.0f;
            const float barY = 35.0f;

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(barX, barY);
            bgRect.size = Renderer::Vector2(barWidth, barHeight);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.3f, 0.3f, 0.3f, 1.0f));

            float healthPercent = static_cast<float>(m_bossHealthBar.currentHealth) /
                                 static_cast<float>(m_bossHealthBar.maxHealth);
            if (healthPercent < 0.0f) healthPercent = 0.0f;
            if (healthPercent > 1.0f) healthPercent = 1.0f;

            Renderer::Rectangle fgRect;
            fgRect.position = Renderer::Vector2(barX, barY);
            fgRect.size = Renderer::Vector2(barWidth * healthPercent, barHeight);

            float red = 1.0f - (healthPercent * 0.5f);
            float green = healthPercent * 0.8f;
            m_renderer->DrawRectangle(fgRect, Renderer::Color(red, green, 0.0f, 1.0f));
        }

    }
}
