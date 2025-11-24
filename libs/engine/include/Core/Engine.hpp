#pragma once

#include "Module.hpp"
#include "ModuleLoader.hpp"
#include "Logger.hpp"
#include "../ECS/Registry.hpp"
#include "../ECS/ISystem.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <typeindex>

namespace RType {

    namespace Core {

        struct EngineConfig {
            std::string pluginPath = "./plugins";
        };

        class Engine {
        public:
            explicit Engine(const EngineConfig& config = EngineConfig{});
            ~Engine();

            Engine(const Engine&) = delete;
            Engine& operator=(const Engine&) = delete;

            bool Initialize();
            void Shutdown();

            IModule* LoadPlugin(const std::string& pluginPath);
            bool UnloadPlugin(const std::string& pluginName);

            template <typename T>
            void RegisterModule(std::unique_ptr<T> module);

            template <typename T>
            T* GetModule();

            IModule* GetModuleByName(const std::string& name);
            std::vector<IModule*> GetAllModules() const;

            ECS::Registry& GetRegistry() { return m_registry; }
            const ECS::Registry& GetRegistry() const { return m_registry; }

            template <typename T>
            void RegisterSystem(std::unique_ptr<T> system);
            void UpdateSystems(float deltaTime);
        private:
            void SortModulesByPriority();
            bool InitializeModules();
            void ShutdownModules();
            void InitializeSystems();
            void ShutdownSystems();

            EngineConfig m_config;
            ModuleLoader m_moduleLoader;
            std::unordered_map<std::type_index, std::unique_ptr<IModule>> m_builtinModules;
            std::vector<IModule*> m_sortedModules;
            ECS::Registry m_registry;
            std::vector<std::unique_ptr<ECS::ISystem>> m_systems;
            bool m_initialized{false};
        };

        template <typename T>
        void Engine::RegisterModule(std::unique_ptr<T> module) {
            static_assert(std::is_base_of<IModule, T>::value, "T must derive from IModule");

            if (!module) {
                Logger::Error("Cannot register null module");
                return;
            }

            std::type_index typeId = std::type_index(typeid(T));
            Logger::Info("Registering module '{}'", module->GetName());
            m_builtinModules[typeId] = std::move(module);
        }

        template <typename T>
        T* Engine::GetModule() {
            static_assert(std::is_base_of<IModule, T>::value, "T must derive from IModule");

            std::type_index typeId = std::type_index(typeid(T));

            auto it = m_builtinModules.find(typeId);
            if (it != m_builtinModules.end()) {
                return static_cast<T*>(it->second.get());
            }

            for (IModule* module : m_moduleLoader.GetAllPlugins()) {
                T* casted = dynamic_cast<T*>(module);
                if (casted) {
                    return casted;
                }
            }

            return nullptr;
        }

        template <typename T>
        void Engine::RegisterSystem(std::unique_ptr<T> system) {
            static_assert(std::is_base_of<ECS::ISystem, T>::value, "T must derive from ISystem");

            if (!system) {
                Logger::Error("Cannot register null system");
                return;
            }

            Logger::Info("Registering system '{}'", system->GetName());
            m_systems.push_back(std::move(system));
        }

    }

}
