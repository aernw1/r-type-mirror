/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** GameState
*/

#include "../include/GameState.hpp"
#include <iostream>

namespace RType {
    namespace Client {

        GameState::GameState(GameStateMachine& machine, GameContext& context, uint32_t seed)
            : m_machine(machine),
              m_context(context),
              m_gameSeed(seed)
        {
            m_renderer = context.renderer;
        }

    }
}
