#include "../../include/Core/Engine.hpp"
#include <algorithm>

namespace RType {

    namespace Core {

        Engine::Engine(const EngineConfig& config)
            : m_config(config) {
            Logger::Info("Creating R-Type Engine");
        }

        Engine::~Engine() {
            if (m_initialized) {
                Shutdown();
            }
        }

        bool Engine::Initialize() {
            if (m_initialized) {
                Logger::Warning("Engine already initialized");
                return true;
            }

            Logger::Info("Initializing R-Type Engine...");

            SortModulesByPriority();

            if (!InitializeModules()) {
                Logger::Error("Failed to initialize modules");
                return false;
            }

            InitializeSystems();

            m_initialized = true;
            Logger::Info("R-Type Engine initialized successfully");
            return true;
        }

        void Engine::Shutdown() {
            if (!m_initialized) {
                return;
            }

            Logger::Info("Shutting down R-Type Engine...");

            ShutdownSystems();
            ShutdownModules();
            m_moduleLoader.UnloadAllPlugins();

            m_initialized = false;
            Logger::Info("R-Type Engine shutdown complete");
        }

        IModule* Engine::LoadPlugin(const std::string& pluginPath) {
            IModule* module = m_moduleLoader.LoadPlugin(pluginPath);

            if (module && m_initialized) {
                Logger::Info("Initializing late-loaded plugin '{}'", module->GetName());
                if (!module->Initialize(this)) {
                    Logger::Error("Failed to initialize plugin '{}'", module->GetName());
                    m_moduleLoader.UnloadPlugin(module->GetName());
                    return nullptr;
                }
                SortModulesByPriority();
            }

            return module;
        }

        bool Engine::UnloadPlugin(const std::string& pluginName) {
            IModule* module = m_moduleLoader.GetPlugin(pluginName);

            if (module && m_initialized) {
                Logger::Info("Shutting down plugin '{}'", pluginName);
                module->Shutdown();
            }

            bool result = m_moduleLoader.UnloadPlugin(pluginName);

            if (result) {
                SortModulesByPriority();
            }

            return result;
        }

        IModule* Engine::GetModuleByName(const std::string& name) {
            for (const auto& [_, module] : m_builtinModules) {
                if (std::string(module->GetName()) == name) {
                    return module.get();
                }
            }

            return m_moduleLoader.GetPlugin(name);
        }

        std::vector<IModule*> Engine::GetAllModules() const { return m_sortedModules; }

        void Engine::SortModulesByPriority() {
            m_sortedModules.clear();

            for (const auto& [_, module] : m_builtinModules) {
                m_sortedModules.push_back(module.get());
            }

            for (IModule* plugin : m_moduleLoader.GetAllPlugins()) {
                m_sortedModules.push_back(plugin);
            }

            std::sort(m_sortedModules.begin(), m_sortedModules.end(), [](IModule* a, IModule* b) {
                return static_cast<int>(a->GetPriority()) < static_cast<int>(b->GetPriority());
            });
        }

        bool Engine::InitializeModules() {
            Logger::Info("Initializing {} modules...", m_sortedModules.size());

            for (IModule* module : m_sortedModules) {
                Logger::Info("Initializing module '{}'", module->GetName());

                if (!module->Initialize(this)) {
                    Logger::Error("Failed to initialize module '{}'", module->GetName());
                    return false;
                }
            }

            return true;
        }

        void Engine::ShutdownModules() {
            Logger::Info("Shutting down {} modules...", m_sortedModules.size());

            for (auto it = m_sortedModules.rbegin(); it != m_sortedModules.rend(); ++it) {
                IModule* module = *it;
                Logger::Info("Shutting down module '{}'", module->GetName());
                module->Shutdown();
            }
        }

        void Engine::UpdateSystems(float deltaTime) {
            for (auto& system : m_systems) {
                system->Update(m_registry, deltaTime);
            }
        }

        void Engine::InitializeSystems() {
            Logger::Info("Initializing {} systems...", m_systems.size());

            for (auto& system : m_systems) {
                Logger::Info("Initializing system '{}'", system->GetName());
                system->Initialize(m_registry);
            }
        }

        void Engine::ShutdownSystems() {
            Logger::Info("Shutting down {} systems...", m_systems.size());

            for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
                Logger::Info("Shutting down system '{}'", (*it)->GetName());
                (*it)->Shutdown();
            }
        }

    }

}
