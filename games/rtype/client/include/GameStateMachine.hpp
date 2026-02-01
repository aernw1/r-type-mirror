/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameStateMachine
*/

#pragma once

#include <memory>
#include <optional>
#include <stack>
#include <vector>
#include <iostream>
#include "Renderer/IRenderer.hpp"
#include "ECS/Registry.hpp"
#include "Audio/IAudio.hpp"
#include "../../libs/network/include/GameClient.hpp"
#include "../../libs/network/include/Protocol.hpp"
#include "../../libs/network/include/INetworkModule.hpp"
#include "Core/Logger.hpp"

namespace RType {
    namespace Client {

        enum class GameState {
            None,
            Menu,
            Lobby,
            Game,
            Exit
        };

        struct GameContext {
            std::shared_ptr<Renderer::IRenderer> renderer;
            std::shared_ptr<ECS::Registry> registry;
            std::shared_ptr<network::GameClient> networkClient;
            std::shared_ptr<Network::INetworkModule> networkModule;
            std::shared_ptr<Audio::IAudio> audio;

            std::string playerName;
            std::string serverIp;
            uint16_t serverPort;
            uint64_t playerHash;
            uint8_t playerNumber;
            std::vector<network::PlayerInfo> allPlayers;

            uint32_t roomId = 0;
            std::string roomName;
        };

        class IState {
        public:
            virtual ~IState() = default;

            virtual void Init() = 0;
            virtual void Cleanup() {}
            virtual void HandleInput() = 0;
            virtual void Update(float dt) = 0;
            virtual void Draw() = 0;
        };

        class GameStateMachine {
        public:
            GameStateMachine() = default;
            ~GameStateMachine() {
                while (!m_states.empty()) {
                    PopStateImmediate();
                }
            }

            void PushState(std::unique_ptr<IState> state) {
                if (m_isDispatching) {
                    m_pending = PendingOp{OpType::Push, std::move(state)};
                    return;
                }
                PushStateImmediate(std::move(state));
            }

            void PopState() {
                if (m_isDispatching) {
                    m_pending = PendingOp{OpType::Pop, nullptr};
                    return;
                }
                PopStateImmediate();
            }

            void ChangeState(std::unique_ptr<IState> state) {
                if (m_isDispatching) {
                    m_pending = PendingOp{OpType::Change, std::move(state)};
                    return;
                }
                ChangeStateImmediate(std::move(state));
            }

            IState* GetCurrentState() {
                return m_states.empty() ? nullptr : m_states.top().get();
            }

            void Update(float dt) {
                if (GetCurrentState()) {
                    m_isDispatching = true;
                    GetCurrentState()->Update(dt);
                    m_isDispatching = false;
                    ApplyPending();
                    if (m_states.empty()) {
                        Core::Logger::Warning("[GameStateMachine] State stack is empty after Update!");
                    }
                }
            }

            void Draw() {
                if (GetCurrentState()) {
                    m_isDispatching = true;
                    GetCurrentState()->Draw();
                    m_isDispatching = false;
                    ApplyPending();
                    if (m_states.empty()) {
                        Core::Logger::Warning("[GameStateMachine] State stack is empty after Draw!");
                    }
                }
            }

            void HandleInput() {
                if (GetCurrentState()) {
                    m_isDispatching = true;
                    GetCurrentState()->HandleInput();
                    m_isDispatching = false;
                    ApplyPending();
                    
                    if (m_states.empty()) {
                        Core::Logger::Warning("[GameStateMachine] State stack is empty after HandleInput!");
                    }
                }
            }

            bool IsRunning() const { return !m_states.empty(); }
            size_t GetStateCount() const { return m_states.size(); }
        private:
            enum class OpType {
                Push,
                Pop,
                Change
            };

            struct PendingOp {
                OpType type;
                std::unique_ptr<IState> state;
            };

            void ApplyPending() {
                if (!m_pending.has_value()) {
                    return;
                }
                PendingOp op = std::move(*m_pending);
                m_pending.reset();

                switch (op.type) {
                case OpType::Push:
                    PushStateImmediate(std::move(op.state));
                    break;
                case OpType::Pop:
                    PopStateImmediate();
                    break;
                case OpType::Change:
                    ChangeStateImmediate(std::move(op.state));
                    break;
                }
            }

            void PushStateImmediate(std::unique_ptr<IState> state) {
                m_states.push(std::move(state));
                m_states.top()->Init();
            }

            void PopStateImmediate() {
                if (!m_states.empty()) {
                    m_states.top()->Cleanup();
                    m_states.pop();
                } else {
                    Core::Logger::Warning("[GameStateMachine] Attempted to pop state from empty stack!");
                }
            }

            void ChangeStateImmediate(std::unique_ptr<IState> state) {
                if (!m_states.empty()) {
                    m_states.top()->Cleanup();
                    m_states.pop();
                }
                PushStateImmediate(std::move(state));
            }

            std::stack<std::unique_ptr<IState>> m_states;
            bool m_isDispatching = false;
            std::optional<PendingOp> m_pending;
        };

    }
}
