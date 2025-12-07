#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include <typeindex>
#include <type_traits>
#include <vector>
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include "Entity.hpp"
#include "Serialization/Serializer.hpp"
#include "Serialization/Deserializer.hpp"

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

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(x);
                ser.serializeTrivial(y);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(x);
                deser.deserializeTrivial(y);
            }
        };

        struct Velocity : public IComponent {
            float dx = 0.0f;
            float dy = 0.0f;

            Velocity() = default;
            Velocity(float dx, float dy)
                : dx(dx), dy(dy) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(dx);
                ser.serializeTrivial(dy);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(dx);
                deser.deserializeTrivial(dy);
            }
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

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(spriteId);
                ser.serializeTrivial(scale.x);
                ser.serializeTrivial(scale.y);
                ser.serializeTrivial(rotation);
                ser.serializeTrivial(origin.x);
                ser.serializeTrivial(origin.y);
                ser.serializeTrivial(tint.r);
                ser.serializeTrivial(tint.g);
                ser.serializeTrivial(tint.b);
                ser.serializeTrivial(tint.a);
                ser.serializeTrivial(layer);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(spriteId);
                deser.deserializeTrivial(scale.x);
                deser.deserializeTrivial(scale.y);
                deser.deserializeTrivial(rotation);
                deser.deserializeTrivial(origin.x);
                deser.deserializeTrivial(origin.y);
                deser.deserializeTrivial(tint.r);
                deser.deserializeTrivial(tint.g);
                deser.deserializeTrivial(tint.b);
                deser.deserializeTrivial(tint.a);
                deser.deserializeTrivial(layer);
            }
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

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(playerNumber);
                ser.serializeTrivial(playerHash);
                for (int i = 0; i < 32; ++i) ser.serializeTrivial(name[i]);
                ser.serializeTrivial(ready);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(playerNumber);
                deser.deserializeTrivial(playerHash);
                for (int i = 0; i < 32; ++i) deser.deserializeTrivial(name[i]);
                deser.deserializeTrivial(ready);
            }
        };

        struct BoxCollider : public IComponent {
            float width = 0.0f;
            float height = 0.0f;

            BoxCollider() = default;
            BoxCollider(float width, float height)
                : width(width), height(height) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(width);
                ser.serializeTrivial(height);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(width);
                deser.deserializeTrivial(height);
            }
        };

        struct Controllable : public IComponent {
            float speed = 200.0f;

            Controllable() = default;
            Controllable(float moveSpeed)
                : speed(moveSpeed) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(speed);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(speed);
            }
        };

        struct Player : public IComponent {
            uint8_t playerNumber = 0;
            uint64_t playerHash = 0;
            bool isLocalPlayer = false;

            Player() = default;
            Player(uint8_t number, uint64_t hash, bool local = false)
                : playerNumber(number), playerHash(hash), isLocalPlayer(local) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(playerNumber);
                ser.serializeTrivial(playerHash);
                ser.serializeTrivial(isLocalPlayer);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(playerNumber);
                deser.deserializeTrivial(playerHash);
                deser.deserializeTrivial(isLocalPlayer);
            }
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

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(type);
                ser.serializeTrivial(id);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(type);
                deser.deserializeTrivial(id);
            }
        };

        struct Health : public IComponent {
            int current = 100;
            int max = 100;

            Health() = default;
            Health(int maxHealth)
                : current(maxHealth), max(maxHealth) {}
            Health(int currentHealth, int maxHealth)
                : current(currentHealth), max(maxHealth) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(current);
                ser.serializeTrivial(max);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(current);
                deser.deserializeTrivial(max);
            }
        };

        struct ScoreValue : public IComponent {
            uint32_t points = 100;

            ScoreValue() = default;
            ScoreValue(uint32_t scorePoints)
                : points(scorePoints) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(points);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(points);
            }
        };

        struct Damage : public IComponent {
            int amount = 10;

            Damage() = default;
            Damage(int damageAmount)
                : amount(damageAmount) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(amount);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(amount);
            }
        };

        struct EnemyKilled : public IComponent {
            uint32_t enemyId = 0;
            Entity killedBy = NULL_ENTITY;

            EnemyKilled() = default;
            EnemyKilled(uint32_t id, Entity killer = NULL_ENTITY)
                : enemyId(id), killedBy(killer) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(enemyId);
                ser.serializeTrivial(killedBy);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(enemyId);
                deser.deserializeTrivial(killedBy);
            }
        };

        struct Bullet : public IComponent {
            Entity owner = NULL_ENTITY;

            Bullet() = default;
            Bullet(Entity shooter)
                : owner(shooter) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(owner);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(owner);
            }
        };

        struct Shooter : public IComponent {
            float fireRate = 0.2f;
            float cooldown = 0.0f;
            float offsetX = 50.0f;
            float offsetY = 20.0f;

            Shooter() = default;
            Shooter(float rate, float oX = 50.0f, float oY = 20.0f) : fireRate(rate), offsetX(oX), offsetY(oY) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(fireRate);
                ser.serializeTrivial(cooldown);
                ser.serializeTrivial(offsetX);
                ser.serializeTrivial(offsetY);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(fireRate);
                deser.deserializeTrivial(cooldown);
                deser.deserializeTrivial(offsetX);
                deser.deserializeTrivial(offsetY);
            }
        };

        struct ShootCommand : public IComponent {
            bool wantsToShoot = false;

            ShootCommand() = default;
            ShootCommand(bool shoot) : wantsToShoot(shoot) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(wantsToShoot);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(wantsToShoot);
            }
        };

        struct Scrollable : public IComponent {
            float speed = -100.0f;

            Scrollable() = default;
            Scrollable(float scrollSpeed) : speed(scrollSpeed) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(speed);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(speed);
            }
        };

        struct Obstacle : public IComponent {
            bool blocking = true;

            Obstacle() = default;
            Obstacle(bool isBlocking) : blocking(isBlocking) {}

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(blocking);
            }

            void deserialize(Engine::Deserializer& deser) {
                deser.deserializeTrivial(blocking);
            }
        };

        struct ColliderBox {
            float x = 0.0f;
            float y = 0.0f;
            float width = 0.0f;
            float height = 0.0f;

            ColliderBox() = default;
            ColliderBox(float offsetX, float offsetY, float w, float h)
                : x(offsetX), y(offsetY), width(w), height(h) {}
        };

        struct MultiBoxCollider : public IComponent {
            std::vector<ColliderBox> boxes;

            MultiBoxCollider() = default;
            MultiBoxCollider(const std::vector<ColliderBox>& colliderBoxes)
                : boxes(colliderBoxes) {}

            void AddBox(float offsetX, float offsetY, float width, float height) {
                boxes.emplace_back(offsetX, offsetY, width, height);
            }

            void serialize(Engine::Serializer& ser) const {
                ser.serializeTrivial(static_cast<uint32_t>(boxes.size()));
                for (const auto& box : boxes) {
                    ser.serializeTrivial(box.x);
                    ser.serializeTrivial(box.y);
                    ser.serializeTrivial(box.width);
                    ser.serializeTrivial(box.height);
                }
            }

            void deserialize(Engine::Deserializer& deser) {
                uint32_t count = 0;
                deser.deserializeTrivial(count);
                boxes.clear();
                boxes.reserve(count);
                for (uint32_t i = 0; i < count; ++i) {
                    ColliderBox box;
                    deser.deserializeTrivial(box.x);
                    deser.deserializeTrivial(box.y);
                    deser.deserializeTrivial(box.width);
                    deser.deserializeTrivial(box.height);
                    boxes.push_back(box);
                }
            }
        };
    }

}
