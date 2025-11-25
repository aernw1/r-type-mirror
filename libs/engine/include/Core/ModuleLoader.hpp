#pragma once

#define RTYPE_INCLUDE_WINDOWS_H
#include "Platform.hpp"
#include "Module.hpp"
#include "Logger.hpp"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

#ifdef _WIN32
using LibraryHandle = HMODULE;
#else
#include <dlfcn.h>
using LibraryHandle = void*;
#endif

namespace RType {

    namespace Core {

        struct PluginInfo {
            std::string name;
            std::string path;
            LibraryHandle handle;
            IModule* module;
            DestroyModuleFunc destroyFunc;
        };

        class ModuleLoader {
        public:
            ModuleLoader() = default;
            ~ModuleLoader();

            ModuleLoader(const ModuleLoader&) = delete;
            ModuleLoader& operator=(const ModuleLoader&) = delete;

            ModuleLoader(ModuleLoader&&) = default;
            ModuleLoader& operator=(ModuleLoader&&) = default;

            IModule* LoadPlugin(const std::string& pluginPath);
            bool UnloadPlugin(const std::string& pluginName);
            void UnloadAllPlugins();
            IModule* GetPlugin(const std::string& pluginName);
            std::vector<IModule*> GetAllPlugins() const;
            bool IsPluginLoaded(const std::string& pluginName) const;
            size_t GetPluginCount() const { return m_loadedPlugins.size(); }
        private:
            LibraryHandle LoadLibraryFromPath(const std::string& path);
            void FreeLibraryHandle(LibraryHandle handle);
            void* GetFunction(LibraryHandle handle, const std::string& name);
            std::string GetLastErrorMessage();
            std::string ExtractPluginName(const std::string& path);

            std::unordered_map<std::string, PluginInfo> m_loadedPlugins;
        };

    }

}
