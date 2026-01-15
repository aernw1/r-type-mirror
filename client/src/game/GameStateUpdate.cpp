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
            static bool spacePressedLastFrame = false;
            bool spacePressed = m_renderer->IsKeyPressed(Renderer::Key::Space);

            if (spacePressed) {
                m_isCharging = true;
                m_chargeTime += 0.016f; 
                if (m_chargeTime > 2.0f) {
                    m_chargeTime = 2.0f;
                }
                
            } else {
                if (spacePressedLastFrame) {
                    m_currentInputs |= network::InputFlags::SHOOT;
                    if (m_isNetworkSession) {
                        if (m_shootMusic != Audio::INVALID_MUSIC_ID) {
                             std::cout << "[DEBUG] Direct PlayMusic Call (Shoot ID: " << m_shootMusic << ")" << std::endl;
                             Audio::PlaybackOptions opts;
                             opts.volume = 1.0f;
                             opts.loop = false;
                             m_context.audio->StopMusic(m_shootMusic); 
                             m_context.audio->PlayMusic(m_shootMusic, opts);
                        } else if (m_playerShootSound != Audio::INVALID_SOUND_ID) {
                            std::cout << "[DEBUG] Direct PlaySound Call (ID: " << m_playerShootSound << ")" << std::endl;
                            Audio::PlaybackOptions opts;
                            opts.volume = 1.0f;
                            m_context.audio->PlaySound(m_playerShootSound, opts);
                        }
                    }
                }
                m_isCharging = false;
                m_chargeTime = 0.0f;
            }
            spacePressedLastFrame = spacePressed;
            
            

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

            if (m_shootSfxCooldown > 0.0f) {
                m_shootSfxCooldown -= dt;
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

            // IMPORTANT: Sync obstacle colliders BEFORE player prediction
            // so collision checks use current frame positions
            static bool loggedMode = false;
            if (!loggedMode) {
                std::cout << "[UPDATE MODE] m_isNetworkSession=" << m_isNetworkSession << std::endl;
                loggedMode = true;
            }

            if (m_isNetworkSession) {
                static bool loggedNetworkPath = false;
                if (!loggedNetworkPath) {
                    std::cout << "[UPDATE] Taking NETWORK session path" << std::endl;
                    loggedNetworkPath = true;
                }
                // Scroll backgrounds
                for (auto& bg : m_backgroundEntities) {
                    if (!m_registry.IsEntityAlive(bg))
                        continue;
                    if (!m_registry.HasComponent<Position>(bg) ||
                        !m_registry.HasComponent<Scrollable>(bg))
                        continue;
                    auto& pos = m_registry.GetComponent<Position>(bg);
                    const auto& scrollable = m_registry.GetComponent<Scrollable>(bg);
                    pos.x += scrollable.speed * dt;
                }
                // Scroll obstacle visuals - ARCHITECTURAL FIX
                // Instead of maintaining static list, query obstacle colliders and scroll their visuals
                // This fixes entity ID recycling bug where bullets reused obstacle IDs
                auto obstacleColliders = m_registry.GetEntitiesWithComponent<ECS::ObstacleMetadata>();
                for (auto collider : obstacleColliders) {
                    if (!m_registry.HasComponent<ECS::ObstacleMetadata>(collider)) {
                        continue;
                    }
                    const auto& metadata = m_registry.GetComponent<ECS::ObstacleMetadata>(collider);
                    auto visual = metadata.visualEntity;

                    // Only scroll if visual entity exists and has required components
                    if (visual == ECS::NULL_ENTITY ||
                        !m_registry.IsEntityAlive(visual) ||
                        !m_registry.HasComponent<Position>(visual) ||
                        !m_registry.HasComponent<Scrollable>(visual)) {
                        continue;
                    }

                    auto& pos = m_registry.GetComponent<Position>(visual);
                    const auto& scrollable = m_registry.GetComponent<Scrollable>(visual);
                    pos.x += scrollable.speed * dt;
                }
                // Synchronize collider positions to visual entities using ObstacleMetadata
                // ONLY for obstacles that still have Scrollable (not yet synced via network)
                // Use component query instead of static list
                static int syncDebugCounter = 0;
                auto obstacleCollidersForSync = m_registry.GetEntitiesWithComponent<ECS::ObstacleMetadata>();
                for (auto collider : obstacleCollidersForSync) {
                    if (!m_registry.IsEntityAlive(collider) ||
                        !m_registry.HasComponent<ObstacleMetadata>(collider) ||
                        !m_registry.HasComponent<Position>(collider))
                        continue;

                    // Skip obstacles that are synced via network (no Scrollable component)
                    if (!m_registry.HasComponent<Scrollable>(collider))
                        continue;

                    const auto& metadata = m_registry.GetComponent<ObstacleMetadata>(collider);

                    // If visual entity exists, sync collider to visual position + offset
                    if (metadata.visualEntity != ECS::NULL_ENTITY &&
                        m_registry.IsEntityAlive(metadata.visualEntity) &&
                        m_registry.HasComponent<Position>(metadata.visualEntity)) {

                        const auto& visualPos = m_registry.GetComponent<Position>(metadata.visualEntity);
                        auto& colliderPos = m_registry.GetComponent<Position>(collider);

                        // Debug: Log first few syncs to verify it's running
                        if (syncDebugCounter < 5) {
                            std::cout << "[RUNTIME SYNC] Collider " << collider
                                      << ": visual=(" << visualPos.x << "," << visualPos.y << ")"
                                      << " offset=(" << metadata.offsetX << "," << metadata.offsetY << ")"
                                      << " -> collider=(" << visualPos.x + metadata.offsetX << ","
                                      << visualPos.y + metadata.offsetY << ")" << std::endl;
                            syncDebugCounter++;
                        }

                        // Collider position = visual position + stored offset
                        colliderPos.x = visualPos.x + metadata.offsetX;
                        colliderPos.y = visualPos.y + metadata.offsetY;
                    }
                    // Fallback: if no visual (missing texture), scroll collider independently
                    else if (m_registry.HasComponent<Scrollable>(collider)) {
                        auto& pos = m_registry.GetComponent<Position>(collider);
                        const auto& scrollable = m_registry.GetComponent<Scrollable>(collider);
                        pos.x += scrollable.speed * dt;

                        if (syncDebugCounter < 5) {
                            std::cout << "[RUNTIME SYNC FALLBACK] Collider " << collider
                                      << " scrolling independently at (" << pos.x << "," << pos.y << ")" << std::endl;
                            syncDebugCounter++;
                        }
                    }
                }
            } else if (m_scrollingSystem) {
                static bool loggedScrollingPath = false;
                if (!loggedScrollingPath) {
                    std::cout << "[UPDATE] Taking ScrollingSystem path (non-network)" << std::endl;
                    loggedScrollingPath = true;
                }
                // Non-network mode: use ScrollingSystem which also handles sync
                m_scrollingSystem->Update(m_registry, dt);
            }

            if (m_isNetworkSession && m_localPlayerEntity != ECS::NULL_ENTITY &&
                m_registry.IsEntityAlive(m_localPlayerEntity) &&
                m_registry.HasComponent<Position>(m_localPlayerEntity)) {
                static bool loggedPrediction = false;
                if (!loggedPrediction) {
                    std::cout << "[UPDATE] Player prediction code is running" << std::endl;
                    std::cout << "[UPDATE] m_obstacleColliderEntities.size()=" << m_obstacleColliderEntities.size() << std::endl;
                    loggedPrediction = true;
                }
                auto& pos = m_registry.GetComponent<Position>(m_localPlayerEntity);

                float newX = pos.x;
                float newY = pos.y;
                if (m_currentInputs & network::InputFlags::UP) {
                    newY -= PREDICTION_SPEED * dt;
                }
                if (m_currentInputs & network::InputFlags::DOWN) {
                    newY += PREDICTION_SPEED * dt;
                }
                if (m_currentInputs & network::InputFlags::LEFT) {
                    newX -= PREDICTION_SPEED * dt;
                }
                if (m_currentInputs & network::InputFlags::RIGHT) {
                    newX += PREDICTION_SPEED * dt;
                }

                newX = std::max(0.0f, std::min(newX, 1280.0f - 66.0f));
                newY = std::max(0.0f, std::min(newY, 720.0f - 32.0f));

                float playerW = 25.0f, playerH = 25.0f;
                if (m_registry.HasComponent<BoxCollider>(m_localPlayerEntity)) {
                    const auto& box = m_registry.GetComponent<BoxCollider>(m_localPlayerEntity);
                    playerW = box.width;
                    playerH = box.height;
                }

                bool blocked = false;
                static int collisionDebugCounter = 0;
                for (auto& collider : m_obstacleColliderEntities) {
                    if (!m_registry.IsEntityAlive(collider) ||
                        !m_registry.HasComponent<Position>(collider) ||
                        !m_registry.HasComponent<BoxCollider>(collider))
                        continue;
                    const auto& obstPos = m_registry.GetComponent<Position>(collider);
                    const auto& obstBox = m_registry.GetComponent<BoxCollider>(collider);

                    bool wouldCollide =
                        newX < obstPos.x + obstBox.width &&
                        newX + playerW > obstPos.x &&
                        newY < obstPos.y + obstBox.height &&
                        newY + playerH > obstPos.y;

                    if (wouldCollide) {
                        if (collisionDebugCounter < 3) {
                            std::cout << "[COLLISION DETECTED] Player at (" << newX << "," << newY
                                      << ") size=(" << playerW << "," << playerH << ")"
                                      << " vs Obstacle ENTITY=" << collider
                                      << " at (" << obstPos.x << "," << obstPos.y << ")"
                                      << " size=(" << obstBox.width << "," << obstBox.height << ")" << std::endl;

                            // Check if this entity has ObstacleMetadata
                            if (m_registry.HasComponent<ObstacleMetadata>(collider)) {
                                const auto& meta = m_registry.GetComponent<ObstacleMetadata>(collider);
                                std::cout << "  -> Has metadata: visualEntity=" << meta.visualEntity
                                          << " offset=(" << meta.offsetX << "," << meta.offsetY << ")" << std::endl;

                                // Check if visual entity actually exists and has drawable
                                if (meta.visualEntity != ECS::NULL_ENTITY) {
                                    bool alive = m_registry.IsEntityAlive(meta.visualEntity);
                                    bool hasPos = m_registry.HasComponent<Position>(meta.visualEntity);
                                    bool hasDrawable = m_registry.HasComponent<Drawable>(meta.visualEntity);
                                    std::cout << "  -> Visual entity: alive=" << alive
                                              << " hasPos=" << hasPos
                                              << " hasDrawable=" << hasDrawable << std::endl;
                                    if (hasPos) {
                                        const auto& vPos = m_registry.GetComponent<Position>(meta.visualEntity);
                                        std::cout << "  -> Visual position: (" << vPos.x << "," << vPos.y << ")" << std::endl;
                                    }
                                }
                            } else {
                                std::cout << "  -> NO METADATA!" << std::endl;
                            }
                            collisionDebugCounter++;
                        }
                        blocked = true;
                        float overlapLeft = (newX + playerW) - obstPos.x;
                        float overlapRight = (obstPos.x + obstBox.width) - newX;
                        float overlapTop = (newY + playerH) - obstPos.y;
                        float overlapBottom = (obstPos.y + obstBox.height) - newY;

                        float minOverlapX = std::min(overlapLeft, overlapRight);
                        float minOverlapY = std::min(overlapTop, overlapBottom);

                        if (minOverlapX < minOverlapY) {
                            if (overlapLeft < overlapRight) {
                                newX = obstPos.x - playerW - 0.5f;
                            } else {
                                newX = obstPos.x + obstBox.width + 0.5f;
                            }
                        } else {
                            if (overlapTop < overlapBottom) {
                                newY = obstPos.y - playerH - 0.5f;
                            } else {
                                newY = obstPos.y + obstBox.height + 0.5f;
                            }
                        }
                        break;
                    }
                }

                newX = std::max(0.0f, std::min(newX, 1280.0f - 66.0f));
                newY = std::max(0.0f, std::min(newY, 720.0f - 32.0f));

                pos.x = newX;
                pos.y = newY;

                PredictedInput prediction;
                prediction.sequence = m_inputSequence;
                prediction.inputs = m_currentInputs;
                prediction.predictedX = newX;
                prediction.predictedY = newY;
                prediction.deltaTime = dt;
                m_inputHistory.push_back(prediction);

                while (m_inputHistory.size() > MAX_INPUT_HISTORY) {
                    m_inputHistory.pop_front();
                }

                m_predictedX = newX;
                m_predictedY = newY;

                (void)blocked;
            }

            auto entities = m_registry.GetEntitiesWithComponent<Position>();
            for (auto entity : entities) {
                if (m_registry.HasComponent<Velocity>(entity)) {
                    auto& pos = m_registry.GetComponent<Position>(entity);
                    if (entity == m_localPlayerEntity && !m_isNetworkSession) {
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

            // Scrolling already handled earlier (before player prediction)
            m_localScrollOffset += -150.0f * dt;

            if (m_shieldSystem) {
                m_shieldSystem->Update(m_registry, dt);
            }
            if (m_forcePodSystem) {
                m_forcePodSystem->Update(m_registry, dt);
            }

            if (m_shootingSystem) {
                m_shootingSystem->Update(m_registry, dt);
            }

            if (m_audioSystem) {
                if (!m_isGameOver && m_gameMusic != Audio::INVALID_MUSIC_ID && !m_gameMusicPlaying) {
                    auto cmd = m_registry.CreateEntity();
                    auto& me = m_registry.AddComponent<MusicEffect>(cmd, MusicEffect(m_gameMusic));
                    me.play = true;
                    me.stop = false;
                    me.loop = true;
                    me.volume = 0.35f;
                    me.pitch = 1.0f;
                    m_gameMusicPlaying = true;
                }
                m_audioSystem->Update(m_registry, dt);
            }

            for (auto& bg : m_backgroundEntities) {
                if (!m_registry.HasComponent<Position>(bg))
                    continue;
                auto& pos = m_registry.GetComponent<Position>(bg);
                if (pos.x <= -1280.0f) {
                    pos.x = pos.x + 3 * 1280.0f;
                }
            }

            if (m_isNetworkSession) {
                std::vector<uint32_t> toRemove;
                for (auto& [networkId, interpState] : m_interpolationStates) {
                    auto entityIt = m_networkEntityMap.find(networkId);
                    if (entityIt == m_networkEntityMap.end()) {
                        toRemove.push_back(networkId);
                        continue;
                    }

                    ECS::Entity entity = entityIt->second;
                    if (!m_registry.IsEntityAlive(entity)) {
                        toRemove.push_back(networkId);
                        continue;
                    }

                    if (!m_registry.HasComponent<Position>(entity)) {
                        continue;
                    }

                    interpState.interpTime += dt;
                    float t = interpState.interpTime / interpState.interpDuration;
                    if (t > 1.0f) {
                        t = 1.0f;
                    }

                    auto& pos = m_registry.GetComponent<Position>(entity);
                    pos.x = interpState.prevX + (interpState.targetX - interpState.prevX) * t;
                    pos.y = interpState.prevY + (interpState.targetY - interpState.prevY) * t;

                    if (m_registry.HasComponent<Player>(entity)) {
                        UpdatePlayerNameLabelPosition(entity, pos.x, pos.y);
                    }
                }
                for (uint32_t id : toRemove) {
                    m_interpolationStates.erase(id);
                }

                for (auto& [networkId, ecsEntity] : m_networkEntityMap) {
                    if (!m_registry.IsEntityAlive(ecsEntity)) continue;
                    if (!m_registry.HasComponent<Position>(ecsEntity)) continue;
                    if (!m_registry.HasComponent<Velocity>(ecsEntity)) continue;
                    if (m_interpolationStates.count(networkId) > 0) continue;
                    if (ecsEntity == m_localPlayerEntity) continue;

                    auto& pos = m_registry.GetComponent<Position>(ecsEntity);
                    const auto& vel = m_registry.GetComponent<Velocity>(ecsEntity);
                    pos.x += vel.dx * dt;
                    pos.y += vel.dy * dt;
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

            if (m_context.audio) {
                std::cout << "[DEBUG] Game Over " << std::endl;
                if (m_gameMusic != Audio::INVALID_MUSIC_ID && m_gameMusicPlaying) {
                    m_context.audio->StopMusic(m_gameMusic);
                    m_gameMusicPlaying = false;
                }
                
                if (m_gameOverMusic != Audio::INVALID_MUSIC_ID && !m_gameOverMusicPlaying) {
                    Audio::PlaybackOptions opts;
                    opts.loop = true;
                    opts.volume = 0.5f;
                    m_context.audio->PlayMusic(m_gameOverMusic, opts);
                    m_gameOverMusicPlaying = true;
                }
            }

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

            if (m_context.audio && m_shootMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_shootMusic);
                m_context.audio->UnloadMusic(m_shootMusic);
            }

            if (m_context.audio && m_gameMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_gameMusic);
                m_context.audio->UnloadMusic(m_gameMusic);
                m_gameMusic = Audio::INVALID_MUSIC_ID;
                m_gameMusicPlaying = false;
            }

            if (m_context.audio && m_gameOverMusic != Audio::INVALID_MUSIC_ID) {
                m_context.audio->StopMusic(m_gameOverMusic);
                m_context.audio->UnloadMusic(m_gameOverMusic);
                m_gameOverMusic = Audio::INVALID_MUSIC_ID;
                m_gameOverMusicPlaying = false;
            }

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
