#pragma once
/**
 * @file Rendering.hpp
 * @brief Generic rendering components for any 2D game engine.
 */

#include <cstring>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include "ECS/Components/IComponent.hpp"

namespace RType {
namespace ECS {

    /**
     * @brief Sprite rendering component.
     */
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

    /**
     * @brief Visual damage flash effect.
     */
    struct DamageFlash : public IComponent {
        float duration = 0.1f;
        float timeRemaining = 0.0f;
        bool isActive = false;

        DamageFlash() = default;
        DamageFlash(float flashDuration) : duration(flashDuration) {}

        void Trigger() {
            isActive = true;
            timeRemaining = duration;
        }
    };

    /**
     * @brief Floating text that animates upward and fades.
     */
    struct FloatingText : public IComponent {
        char text[32] = {};
        float lifetime = 0.0f;
        float maxLifetime = 1.5f;
        float velocityY = -50.0f;
        float fadeStartTime = 0.5f;
        Math::Color color{1.0f, 1.0f, 1.0f, 1.0f};

        FloatingText() = default;
        FloatingText(const char* txt, float duration, const Math::Color& col)
            : maxLifetime(duration), color(col) {
            if (txt) {
                std::strncpy(text, txt, 31);
                text[31] = '\0';
            }
        }
    };

} // namespace ECS
} // namespace RType
