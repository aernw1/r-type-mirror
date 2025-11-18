#include <iostream>
#include "../include/ECS/Registry.hpp"

using namespace RType::ECS;
int main() {
    std::cout << "R-Type Engine starting..." << std::endl;
    Registry registry;

    std::cout << "Registry created successfully!" << std::endl;
    std::cout << "R-Type Engine running." << std::endl;

    Entity entity1 = registry.CreateEntity();
    Entity entity2 = registry.CreateEntity();

    registry.AddComponent(entity1, Position(100, 100));
    registry.AddComponent(entity2, Position(200, 200));

    std::cout << "Entity 1 has position: " << registry.GetComponent<Position>(entity1).x << ", "
              << registry.GetComponent<Position>(entity1).y << std::endl;
    std::cout << "Entity 2 has position: " << registry.GetComponent<Position>(entity2).x << ", "
              << registry.GetComponent<Position>(entity2).y << std::endl;

    return 0;
}
