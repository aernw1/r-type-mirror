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

        struct BoxCollider : public IComponent {
            float width = 0.0f;
            float height = 0.0f;

            BoxCollider() = default;
            BoxCollider(float width, float height)
                : width(width), height(height) {}
        };

        struct Controllable : public IComponent {
            float speed = 200.0f;  // Pixels per second

            Controllable() = default;
            Controllable(float moveSpeed)
                : speed(moveSpeed) {}
        };

        struct Player : public IComponent {
            uint8_t playerNumber = 0;
            uint64_t playerHash = 0;
            bool isLocalPlayer = false;

            Player() = default;
            Player(uint8_t number, uint64_t hash, bool local = false)
                : playerNumber(number), playerHash(hash), isLocalPlayer(local) {}
        };

         enum class EnemyType : uint8_t {
            BASIC = 0,
            FAST = 1,
            TANK = 2,
            BOSS = 3,
            FORMATION = 4
        };

        struct Enemy : public IComponent {
            EnemyType type = EnemyType::BASIC;
            uint32_t id = 0;

            Enemy() = default;
            Enemy(EnemyType enemyType, uint32_t enemyId = 0)
                : type(enemyType), id(enemyId) {}
        };

        struct Health : public IComponent {
            int current = 100;
            int max = 100;

            Health() = default;
            Health(int maxHealth)
                : current(maxHealth), max(maxHealth) {}
            Health(int currentHealth, int maxHealth)
                : current(currentHealth), max(maxHealth) {}
        };

        struct ScoreValue : public IComponent {
            uint32_t points = 100;

            ScoreValue() = default;
            ScoreValue(uint32_t scorePoints)
                : points(scorePoints) {}
        };

        struct Damage : public IComponent {
            int amount = 10;

            Damage() = default;
            Damage(int damageAmount)
                : amount(damageAmount) {}
        };

        struct EnemyKilled : public IComponent {
            uint32_t enemyId = 0;
            Entity killedBy = NULL_ENTITY;

            EnemyKilled() = default;
            EnemyKilled(uint32_t id, Entity killer = NULL_ENTITY)
                : enemyId(id), killedBy(killer) {}
        };

        struct Bullet : public IComponent {
            Entity owner = NULL_ENTITY;

            Bullet() = default;
            Bullet(Entity shooter)
                : owner(shooter) {}
        };
    }

}
