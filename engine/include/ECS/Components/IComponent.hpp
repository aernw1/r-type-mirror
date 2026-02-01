#pragma once
/**
 * @file IComponent.hpp
 * @brief Base interface for all ECS components.
 */

#include <typeindex>

namespace RType {
namespace ECS {

    using ComponentID = std::type_index;

    /**
     * @brief Base interface for all components.
     * All components should inherit from this.
     */
    struct IComponent {
        virtual ~IComponent() = default;
    };

} // namespace ECS
} // namespace RType
