/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameStateMachine
*/

#pragma once

#include <memory>
#include <stack>
#include <vector>
#include "Renderer/IRenderer.hpp"
#include "ECS/Registry.hpp"
#include "../../libs/network/include/GameClient.hpp"
#include "../../libs/network/include/Protocol.hpp"

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

            std::string playerName;
            std::string serverIp;
            uint16_t serverPort;
            uint64_t playerHash;
            uint8_t playerNumber;
            std::vector<network::PlayerInfo> allPlayers;
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
                    m_states.top()->Cleanup();
                    m_states.pop();
                }
            }

            void PushState(std::unique_ptr<IState> state) {
                m_states.push(std::move(state));
                m_states.top()->Init();
            }

            void PopState() {
                if (!m_states.empty()) {
                    m_states.top()->Cleanup();
                    m_states.pop();
                }
            }

            void ChangeState(std::unique_ptr<IState> state) {
                if (!m_states.empty()) {
                    m_states.top()->Cleanup();
                    m_states.pop();
                }
                PushState(std::move(state));
            }

            IState* GetCurrentState() {
                return m_states.empty() ? nullptr : m_states.top().get();
            }

            void Update(float dt) {
                if (GetCurrentState())
                    GetCurrentState()->Update(dt);
            }

            void Draw() {
                if (GetCurrentState())
                    GetCurrentState()->Draw();
            }

            void HandleInput() {
                if (GetCurrentState())
                    GetCurrentState()->HandleInput();
            }

            bool IsRunning() const { return !m_states.empty(); }
        private:
            std::stack<std::unique_ptr<IState>> m_states;
        };

    }
}
