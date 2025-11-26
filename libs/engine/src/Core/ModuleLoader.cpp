#include "../../include/Core/ModuleLoader.hpp"
#include <algorithm>
#include <filesystem>

namespace RType {

    namespace Core {

        ModuleLoader::~ModuleLoader() { UnloadAllPlugins(); }

        IModule* ModuleLoader::LoadPlugin(const std::string& pluginPath) {
            // Check if file exists
            if (!std::filesystem::exists(pluginPath)) {
                Logger::Error("Plugin file not found: {}", pluginPath);
                return nullptr;
            }

            // Extract plugin name from path
            std::string pluginName = ExtractPluginName(pluginPath);

            // Check if already loaded
            if (IsPluginLoaded(pluginName)) {
                Logger::Warning("Plugin '{}' is already loaded", pluginName);
                return m_loadedPlugins[pluginName].module;
            }

            // Load the shared library
            LibraryHandle handle = LoadLibraryFromPath(pluginPath);
            if (!handle) {
                Logger::Error("Failed to load library '{}': {}", pluginPath, GetLastErrorMessage());
                return nullptr;
            }

            // Get the CreateModule function
            auto createFunc =
                reinterpret_cast<CreateModuleFunc>(GetFunction(handle, "CreateModule"));
            if (!createFunc) {
                Logger::Error("Failed to find CreateModule function in '{}': {}", pluginPath,
                              GetLastErrorMessage());
                FreeLibraryHandle(handle);
                return nullptr;
            }

            // Get the DestroyModule function
            auto destroyFunc =
                reinterpret_cast<DestroyModuleFunc>(GetFunction(handle, "DestroyModule"));
            if (!destroyFunc) {
                Logger::Error("Failed to find DestroyModule function in '{}': {}", pluginPath,
                              GetLastErrorMessage());
                FreeLibraryHandle(handle);
                return nullptr;
            }

            // Create the module instance
            IModule* module = createFunc();
            if (!module) {
                Logger::Error("CreateModule returned nullptr for '{}'", pluginPath);
                FreeLibraryHandle(handle);
                return nullptr;
            }

            // Store plugin info
            PluginInfo info;
            info.name = module->GetName();
            info.path = pluginPath;
            info.handle = handle;
            info.module = module;
            info.destroyFunc = destroyFunc;

            m_loadedPlugins[info.name] = info;

            Logger::Info("Successfully loaded plugin '{}' from '{}'", info.name, pluginPath);
            return module;
        }

        bool ModuleLoader::UnloadPlugin(const std::string& pluginName) {
            auto it = m_loadedPlugins.find(pluginName);
            if (it == m_loadedPlugins.end()) {
                Logger::Warning("Plugin '{}' not found for unloading", pluginName);
                return false;
            }

            PluginInfo& info = it->second;

            // Destroy the module
            if (info.destroyFunc && info.module) {
                info.destroyFunc(info.module);
                info.module = nullptr;
            }

            // Unload the library
            if (info.handle) {
                FreeLibraryHandle(info.handle);
                info.handle = nullptr;
            }

            // Remove from map
            m_loadedPlugins.erase(it);

            Logger::Info("Successfully unloaded plugin '{}'", pluginName);
            return true;
        }

        void ModuleLoader::UnloadAllPlugins() {
            // Get all plugin names first (to avoid modifying map while iterating)
            std::vector<std::string> pluginNames;
            pluginNames.reserve(m_loadedPlugins.size());
            for (const auto& [name, _] : m_loadedPlugins) {
                pluginNames.push_back(name);
            }

            // Unload in reverse order (LIFO)
            for (auto it = pluginNames.rbegin(); it != pluginNames.rend(); ++it) {
                UnloadPlugin(*it);
            }
        }

        IModule* ModuleLoader::GetPlugin(const std::string& pluginName) {
            auto it = m_loadedPlugins.find(pluginName);
            if (it == m_loadedPlugins.end()) {
                return nullptr;
            }
            return it->second.module;
        }

        std::vector<IModule*> ModuleLoader::GetAllPlugins() const {
            std::vector<IModule*> plugins;
            plugins.reserve(m_loadedPlugins.size());
            for (const auto& [_, info] : m_loadedPlugins) {
                plugins.push_back(info.module);
            }
            return plugins;
        }

        bool ModuleLoader::IsPluginLoaded(const std::string& pluginName) const {
            return m_loadedPlugins.find(pluginName) != m_loadedPlugins.end();
        }

        LibraryHandle ModuleLoader::LoadLibraryFromPath(const std::string& path) {
#ifdef _WIN32
            return ::LoadLibraryA(path.c_str());
#else
            return dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
        }

        void ModuleLoader::FreeLibraryHandle(LibraryHandle handle) {
            if (!handle)
                return;
#ifdef _WIN32
            ::FreeLibrary(handle);
#else
            dlclose(handle);
#endif
        }

        void* ModuleLoader::GetFunction(LibraryHandle handle, const std::string& name) {
#ifdef _WIN32
            return reinterpret_cast<void*>(::GetProcAddress(handle, name.c_str()));
#else
            return dlsym(handle, name.c_str());
#endif
        }

        std::string ModuleLoader::GetLastErrorMessage() {
#ifdef _WIN32
            DWORD error = GetLastError();
            if (error == 0) {
                return "No error";
            }
            LPSTR messageBuffer = nullptr;
            size_t size =
                FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                   FORMAT_MESSAGE_IGNORE_INSERTS,
                               NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (LPSTR)&messageBuffer, 0, NULL);
            std::string message(messageBuffer, size);
            LocalFree(messageBuffer);
            return message;
#else
            const char* error = dlerror();
            return error ? error : "No error";
#endif
        }

        std::string ModuleLoader::ExtractPluginName(const std::string& path) {
            std::filesystem::path p(path);
            std::string filename = p.stem().string();

            // Remove common prefixes like "lib"
            if (filename.substr(0, 3) == "lib") {
                filename = filename.substr(3);
            }

            return filename;
        }

    } // namespace Core

} // namespace RType
