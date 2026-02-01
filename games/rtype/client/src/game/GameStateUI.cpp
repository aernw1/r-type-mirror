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
                Core::Logger::Error("[GameState] Failed to load HUD font");
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

        }

        void InGameState::updateHUD() {
            if (m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.IsEntityAlive(m_localPlayerEntity) &&
                m_registry.HasComponent<ScoreValue>(m_localPlayerEntity)) {
                const auto& scoreComp = m_registry.GetComponent<ScoreValue>(m_localPlayerEntity);
                m_playerScore = scoreComp.points;
            }

            if (m_hudScoreEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudScoreEntity) && m_registry.HasComponent<TextLabel>(m_hudScoreEntity)) {
                auto& scoreLabel = m_registry.GetComponent<TextLabel>(m_hudScoreEntity);
                std::ostringstream ss;
                ss << std::setw(8) << std::setfill('0') << m_playerScore;
                scoreLabel.text = ss.str();
            }

            if (m_hudLivesEntity != NULL_ENTITY && m_registry.IsEntityAlive(m_hudLivesEntity) && m_registry.HasComponent<TextLabel>(m_hudLivesEntity)) {
                auto& livesLabel = m_registry.GetComponent<TextLabel>(m_hudLivesEntity);

                // Calculate visual lives from real health (0-300)
                // 201-300 HP = 3 lives, 101-200 HP = 2 lives, 1-100 HP = 1 life, 0 HP = 0 lives
                int realHealth = 0;
                if (m_context.playerNumber >= 1 && m_context.playerNumber <= MAX_PLAYERS) {
                    realHealth = m_playersHUD[m_context.playerNumber - 1].health;
                }

                int visualLives = 0;
                if (realHealth > 200) {
                    visualLives = 3;
                } else if (realHealth > 100) {
                    visualLives = 2;
                } else if (realHealth > 0) {
                    visualLives = 1;
                }

                livesLabel.text = "LIVES x" + std::to_string(visualLives);

                if (visualLives <= 1) {
                    livesLabel.color = {1.0f, 0.2f, 0.2f, 1.0f};
                } else if (visualLives <= 2) {
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
                    !m_registry.IsEntityAlive(m_playersHUD[i].scoreEntity) ||
                    !m_registry.HasComponent<TextLabel>(m_playersHUD[i].scoreEntity)) {
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

            updatePowerUpIcons();
        }

        void InGameState::updatePowerUpIcons() {
            if (m_context.playerNumber < 1 || m_context.playerNumber > MAX_PLAYERS) {
                return;
            }

            size_t playerIndex = static_cast<size_t>(m_context.playerNumber - 1);
            if (!m_playersHUD[playerIndex].active) {
                return;
            }

            ECS::Entity playerEntity = m_playersHUD[playerIndex].playerEntity;
            if (playerEntity == ECS::NULL_ENTITY) {
                playerEntity = m_localPlayerEntity;
            }

            bool hasSpreadShot = false;
            bool hasLaserBeam = false;
            bool hasSpeedBoost = false;
            bool hasShield = false;

            if (playerEntity != ECS::NULL_ENTITY && m_registry.IsEntityAlive(playerEntity) &&
                m_registry.HasComponent<ActivePowerUps>(playerEntity)) {
                const auto& powerUps = m_registry.GetComponent<ActivePowerUps>(playerEntity);
                hasSpreadShot = powerUps.hasSpreadShot;
                hasLaserBeam = powerUps.hasLaserBeam;
                hasSpeedBoost = powerUps.speedMultiplier > 1.0f;
                hasShield = powerUps.hasShield;
            }

            const float textSpacing = 25.0f;
            const float columnSpacing = 85.0f;
            const float startX = 800.0f;
            const float startY = 675.0f;
            float currentY = startY;

            updatePowerUpText(m_playersHUD[playerIndex].powerupSpreadEntity, "SPREAD",
                             hasSpreadShot, startX, currentY);
            currentY += textSpacing;

            updatePowerUpText(m_playersHUD[playerIndex].powerupLaserEntity, "LASER",
                             hasLaserBeam, startX, currentY);
            currentY += textSpacing;

            currentY = startY;
            updatePowerUpText(m_playersHUD[playerIndex].powerupSpeedEntity, "SPEED",
                             hasSpeedBoost, startX + columnSpacing, currentY);
            currentY += textSpacing;

            updatePowerUpText(m_playersHUD[playerIndex].powerupShieldEntity, "SHIELD",
                             hasShield, startX + columnSpacing, currentY);
        }

        void InGameState::updatePowerUpText(ECS::Entity& textEntity, const std::string& text,
                                            bool isActive, float x, float y) {
            if (textEntity == ECS::NULL_ENTITY || !m_registry.IsEntityAlive(textEntity)) {
                textEntity = m_registry.CreateEntity();
                m_registry.AddComponent<Position>(textEntity, Position{x, y});
                Renderer::FontId fontId = (m_hudFontSmall != Renderer::INVALID_FONT_ID) ? m_hudFontSmall : m_hudFont;
                TextLabel label(text, fontId, 10);
                label.color = isActive ? Math::Color{1.0f, 1.0f, 1.0f, 1.0f} : Math::Color{0.5f, 0.5f, 0.5f, 0.7f};
                m_registry.AddComponent<TextLabel>(textEntity, std::move(label));
            } else {
                if (m_registry.HasComponent<Position>(textEntity)) {
                    auto& pos = m_registry.GetComponent<Position>(textEntity);
                    pos.x = x;
                    pos.y = y;
                }
                if (m_registry.HasComponent<TextLabel>(textEntity)) {
                    auto& label = m_registry.GetComponent<TextLabel>(textEntity);
                    label.color = isActive ? Math::Color{1.0f, 1.0f, 1.0f, 1.0f} : Math::Color{0.5f, 0.5f, 0.5f, 0.7f};
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
            renderVictoryOverlay();
            renderLevelTransition();
            renderBossWarning();
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

            // Calculate visual health (0-100) from real health (0-300)
            // Each "life" represents 100 HP
            int realHealth = m_playersHUD[playerIndex].health;
            int visualHealth = 0;

            if (m_playersHUD[playerIndex].isDead || realHealth <= 0) {
                visualHealth = 0;
            } else if (realHealth > 200) {
                visualHealth = realHealth - 200;  // 201-300 -> 1-100
            } else if (realHealth > 100) {
                visualHealth = realHealth - 100;  // 101-200 -> 1-100
            } else {
                visualHealth = realHealth;        // 1-100 -> 1-100
            }

            float healthPercent = static_cast<float>(visualHealth) / 100.0f;
            healthPercent = std::max(0.0f, std::min(1.0f, healthPercent));

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

        void InGameState::renderVictoryOverlay() {
            if (!m_levelProgress.allLevelsComplete) {
                return;
            }

            Renderer::Rectangle bgRect;
            bgRect.position = Renderer::Vector2(0.0f, 0.0f);
            bgRect.size = Renderer::Vector2(1280.0f, 720.0f);
            m_renderer->DrawRectangle(bgRect, Renderer::Color(0.0f, 0.0f, 0.0f, 0.45f));
        }

        void InGameState::initializeBossHealthBar() {
            if (m_bossHealthBar.active) {
                return;
            }

            m_bossHealthBar.active = true;

            m_bossHealthBar.titleEntity = m_registry.CreateEntity();
            m_registry.AddComponent<Position>(m_bossHealthBar.titleEntity, Position{640.0f - 80.0f, 10.0f});

            std::string bossTitle = "BOSS - LEVEL " + std::to_string(m_levelProgress.currentLevelNumber);
            m_registry.AddComponent<TextLabel>(m_bossHealthBar.titleEntity,
                TextLabel(bossTitle, m_hudFontSmall != Renderer::INVALID_FONT_ID ? m_hudFontSmall : m_hudFont, 12));

            Core::Logger::Info("[GameState] Boss health bar initialized for level {}", m_levelProgress.currentLevelNumber);
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

        void InGameState::renderLevelTransition() {
            if (m_levelProgress.transitionPhase == TransitionPhase::NONE) {
                return;
            }

            if (m_levelProgress.fadeAlpha > 0.0f) {
                Renderer::Rectangle fadeOverlay;
                fadeOverlay.position = Renderer::Vector2(0.0f, 0.0f);
                fadeOverlay.size = Renderer::Vector2(1280.0f, 720.0f);
                m_renderer->DrawRectangle(fadeOverlay, Renderer::Color(0.0f, 0.0f, 0.0f, m_levelProgress.fadeAlpha));
            }

            if (m_levelProgress.transitionPhase == TransitionPhase::LOADING) {
                Renderer::Rectangle fullScreen;
                fullScreen.position = Renderer::Vector2(0.0f, 0.0f);
                fullScreen.size = Renderer::Vector2(1280.0f, 720.0f);
                m_renderer->DrawRectangle(fullScreen, Renderer::Color(0.0f, 0.0f, 0.0f, 1.0f));
            }
        }

        void InGameState::renderBossWarning() {
            if (!m_bossWarningActive) {
                return;
            }

            if (!m_bossWarningFlashState) {
                return;
            }

            if (m_gameOverFontLarge != Renderer::INVALID_FONT_ID) {
                Renderer::TextParams textParams;
                textParams.position = Renderer::Vector2(440.0f, 300.0f);
                textParams.color = Renderer::Color(1.0f, 0.0f, 0.0f, 1.0f);
                textParams.scale = 1.0f;
                m_renderer->DrawText(m_gameOverFontLarge, "WARNING !!", textParams);
            } else {
                 Core::Logger::Error("[GameState] Cannot render boss warning - Invalid Font ID");
            }
        }

    }
}
