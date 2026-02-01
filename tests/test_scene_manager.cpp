/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test SceneManager
*/

#include "Core/SceneManager.hpp"
#include "ECS/Registry.hpp"
#include <iostream>
#include <cassert>

int main() {
    RType::ECS::Registry registry;
    RType::Core::SceneManager sceneManager(&registry);

    bool scene1Loaded = false;
    bool scene2Loaded = false;

    // Test LoadScene
    sceneManager.LoadScene("Scene1", [&](RType::ECS::Registry& r) {
        scene1Loaded = true;
        auto e = r.CreateEntity(); // Create one entity
    });

    assert(scene1Loaded && "Scene1 setup should be called");
    assert(sceneManager.GetCurrentSceneName() == "Scene1");
    assert(registry.GetEntityCount() == 1 && "Registry should have 1 entity");

    // Test Transition (should clear previous scene)
    sceneManager.RegisterScene("Scene2", [&](RType::ECS::Registry& r) {
        scene2Loaded = true;
        // Verify registry was cleared (Entity count reset? ID reset?)
        // Note: Registry::Clear resets ID to 1.
        auto e = r.CreateEntity();
    });
    sceneManager.TransitionTo("Scene2");

    assert(scene2Loaded && "Scene2 setup should be called");
    assert(sceneManager.GetCurrentSceneName() == "Scene2");
    // Implementation detail: SceneManager::TransitionTo calls UnloadCurrentScene (which clears registry) then LoadScene.
    // So entity count should be 1 (from Scene2), not 2.
    assert(registry.GetEntityCount() == 1 && "Registry should be cleared and have 1 entity from Scene2");

    std::cout << "SceneManager tests passed!" << std::endl;
    return 0;
}
