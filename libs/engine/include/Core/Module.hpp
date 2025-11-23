#pragma once

#include <string>

namespace RType {

    namespace Core {

        class Engine;

        enum class ModulePriority { Critical = 0,
                                    High = 1,
                                    Normal = 2,
                                    Low = 3 };

        class IModule {
        public:
            virtual ~IModule() = default;
            virtual const char* GetName() const = 0;
            virtual ModulePriority GetPriority() const = 0;
            virtual bool Initialize(Engine* engine) = 0;
            virtual void Shutdown() = 0;
            virtual void Update(float deltaTime) = 0;
            virtual bool ShouldUpdateInRenderThread() const { return false; }
            virtual bool IsOverridable() const { return true; }
        };

        using CreateModuleFunc = IModule* (*)();
        using DestroyModuleFunc = void (*)(IModule*);

    }

}

#ifdef _WIN32
#define RTYPE_MODULE_EXPORT __declspec(dllexport)
#else
#define RTYPE_MODULE_EXPORT __attribute__((visibility("default")))
#endif
