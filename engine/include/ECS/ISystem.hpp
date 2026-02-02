#pragma once

#include "Registry.hpp"
#include <string>

namespace RType {

    namespace ECS {

        class ISystem {
        public:
            virtual ~ISystem() = default;

            virtual void Update(Registry& registry, float deltaTime) = 0;
            virtual const char* GetName() const = 0;
            virtual bool Initialize(Registry& /* registry */) { return true; }
            virtual void Shutdown() {}
        };

    }

}
