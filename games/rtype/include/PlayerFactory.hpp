#pragma once

#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <string>

namespace RType {

    namespace ECS {

        class PlayerFactory {
        public:
            static Entity CreatePlayer(Registry& registry, uint8_t playerNumber, uint64_t playerHash,
                                       float startX, float startY, Renderer::IRenderer* renderer);
        private:
            static Math::Color GetPlayerColor(uint8_t playerNumber);
            static std::string GetPlayerSpritePath(uint8_t playerNumber);
        };

    }

}
