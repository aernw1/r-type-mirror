#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"

namespace RType {

    namespace ECS {
        using ComponentID = std::type_index;

        struct IComponent {
            virtual ~IComponent() = default;
        };

        struct Position : public IComponent {
            float x = 0.0f;
            float y = 0.0f;

            Position() = default;
            Position(float x, float y)
                : x(x), y(y) {}
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy)
                : dx(dx), dy(dy) {}
        };

        struct Drawable : public IComponent {
            Renderer::SpriteId spriteId = Renderer::INVALID_SPRITE_ID;
            Math::Vector2 scale{1.0f, 1.0f};
            float rotation = 0.0f;
            Math::Vector2 origin{0.0f, 0.0f};
            Math::Color tint{1.0f, 1.0f, 1.0f, 1.0f};
            int layer = 0;

            Drawable() = default;
            Drawable(Renderer::SpriteId sprite, int renderLayer = 0)
                : spriteId(sprite), layer(renderLayer) {}
        };

        struct NetworkPlayer : public IComponent {
            uint8_t playerNumber = 0;
            uint64_t playerHash = 0;
            char name[32] = {};
            bool ready = false;

            NetworkPlayer() = default;
            NetworkPlayer(uint8_t num, uint64_t hash, const char* playerName, bool isReady = false)
                : playerNumber(num), playerHash(hash), ready(isReady) {
                if (playerName) {
                    std::strncpy(name, playerName, 31);
                    name[31] = '\0';
                }
            }
        };

        struct TextLabel : public IComponent {
            std::string text;
            Renderer::FontId fontId = Renderer::INVALID_FONT_ID;
            Math::Color color{1.0f, 1.0f, 1.0f, 1.0f};
            float scale = 1.0f;

            TextLabel() = default;
            TextLabel(const std::string& txt, Renderer::FontId font = Renderer::INVALID_FONT_ID)
                : text(txt), fontId(font) {}
        };
    }

}