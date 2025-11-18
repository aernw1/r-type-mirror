#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include "ECS/Entity.hpp"

using namespace RType::ECS;

int main() {
    Registry registry;

    Entity e1 = registry.CreateEntity();
    registry.AddComponent(e1, Position(0.0f, 0.0f));
    registry.AddComponent(e1, Velocity(1.0f, 1.0f));

    Entity e2 = registry.CreateEntity();
    registry.AddComponent(e2, Position(10.0f, 10.0f));

    return 0;
}
