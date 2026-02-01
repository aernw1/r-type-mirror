/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** SceneManager implementation
*/

#include "Core/SceneManager.hpp"
#include "Core/Logger.hpp"

namespace RType {

    namespace Core {

        SceneManager::SceneManager(ECS::Registry* registry)
            : m_registry(registry) {}

        void SceneManager::RegisterScene(const std::string& sceneName,
                                        SceneSetupFn setup,
                                        SceneTeardownFn teardown) {
            if (!setup) {
                Logger::Warning("SceneManager: Cannot register scene '{}' with null setup function", sceneName);
                return;
            }
            
            m_scenes[sceneName] = SceneData{std::move(setup), std::move(teardown)};
            Logger::Info("SceneManager: Registered scene '{}'", sceneName);
        }

        bool SceneManager::LoadScene(const std::string& sceneName) {
            auto it = m_scenes.find(sceneName);
            if (it == m_scenes.end()) {
                Logger::Error("SceneManager: Scene '{}' not registered", sceneName);
                return false;
            }

            return LoadScene(sceneName, it->second.setup);
        }

        bool SceneManager::LoadScene(const std::string& sceneName, SceneSetupFn setup) {
            if (!m_registry) {
                Logger::Error("SceneManager: No registry set");
                return false;
            }

            if (!setup) {
                Logger::Error("SceneManager: Setup function is null");
                return false;
            }

            // Unload current scene first
            UnloadCurrentScene();

            // Store teardown for later if scene is registered
            auto it = m_scenes.find(sceneName);
            if (it != m_scenes.end()) {
                m_currentTeardown = it->second.teardown;
            } else {
                m_currentTeardown = nullptr;
            }

            // Set current scene
            m_currentScene = sceneName;

            // Call setup function
            Logger::Info("SceneManager: Loading scene '{}'", sceneName);
            setup(*m_registry);

            return true;
        }

        void SceneManager::UnloadCurrentScene() {
            if (m_currentScene.empty() || !m_registry) {
                return;
            }

            Logger::Info("SceneManager: Unloading scene '{}'", m_currentScene);

            // Call teardown if registered
            if (m_currentTeardown) {
                m_currentTeardown(*m_registry);
            }

            // Clear the registry (destroy all entities)
            m_registry->Clear();

            m_currentScene.clear();
            m_currentTeardown = nullptr;
        }

        bool SceneManager::TransitionTo(const std::string& sceneName) {
            return LoadScene(sceneName);
        }

        bool SceneManager::IsSceneRegistered(const std::string& sceneName) const {
            return m_scenes.find(sceneName) != m_scenes.end();
        }

        void SceneManager::UnregisterScene(const std::string& sceneName) {
            m_scenes.erase(sceneName);
        }

        void SceneManager::ClearScenes() {
            m_scenes.clear();
        }

    }

}
