/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** SceneManager - Basic scene lifecycle management
*/

#pragma once

#include "../ECS/Registry.hpp"
#include <string>
#include <functional>
#include <unordered_map>

namespace RType {

    namespace Core {


        class SceneManager {
        public:
            using SceneSetupFn = std::function<void(ECS::Registry&)>;
            using SceneTeardownFn = std::function<void(ECS::Registry&)>;

            explicit SceneManager(ECS::Registry* registry = nullptr);
            ~SceneManager() = default;

            // Non-copyable
            SceneManager(const SceneManager&) = delete;
            SceneManager& operator=(const SceneManager&) = delete;


            void SetRegistry(ECS::Registry* registry) { m_registry = registry; }


            void RegisterScene(const std::string& sceneName, 
                             SceneSetupFn setup,
                             SceneTeardownFn teardown = nullptr);


            bool LoadScene(const std::string& sceneName);


            bool LoadScene(const std::string& sceneName, SceneSetupFn setup);


            void UnloadCurrentScene();


            bool TransitionTo(const std::string& sceneName);


            const std::string& GetCurrentSceneName() const { return m_currentScene; }


            bool HasActiveScene() const { return !m_currentScene.empty(); }


            bool IsSceneRegistered(const std::string& sceneName) const;


            void UnregisterScene(const std::string& sceneName);


            void ClearScenes();

        private:
            struct SceneData {
                SceneSetupFn setup;
                SceneTeardownFn teardown;
            };

            ECS::Registry* m_registry = nullptr;
            std::string m_currentScene;
            SceneTeardownFn m_currentTeardown;
            std::unordered_map<std::string, SceneData> m_scenes;
        };

    }

}
