/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Camera2D - 2D camera component for target following
*/

#pragma once

#include "IComponent.hpp"
#include "../Entity.hpp"
#include "../../Math/Types.hpp"

namespace RType {

    namespace ECS {


        struct Camera2D : public IComponent {
            /// Entity to follow (NULL_ENTITY for static camera)
            Entity target = NULL_ENTITY;
            
            /// Offset from target position
            Math::Vector2 offset{0.0f, 0.0f};
            
            /// Current camera position (updated by Camera2DSystem)
            Math::Vector2 position{0.0f, 0.0f};
            
            /// World bounds for camera clamping (0 = no bounds)
            Math::Vector2 boundsMin{0.0f, 0.0f};
            Math::Vector2 boundsMax{0.0f, 0.0f};
            
            /// Smooth interpolation speed (higher = snappier, 0 = instant)
            float smoothSpeed = 5.0f;
            
            /// Camera zoom level (1.0 = default, 2.0 = 2x zoom in)
            float zoom = 1.0f;
            
            /// Whether this camera is active
            bool active = true;
            
            /// Screen shake intensity (0 = no shake)
            float shakeIntensity = 0.0f;
            
            /// Screen shake duration remaining
            float shakeDuration = 0.0f;

            Camera2D() = default;


            void Shake(float intensity, float duration) {
                shakeIntensity = intensity;
                shakeDuration = duration;
            }


            bool HasBounds() const {
                return boundsMax.x > boundsMin.x || boundsMax.y > boundsMin.y;
            }
        };

    }

}
