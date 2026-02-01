/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Camera2DSystem implementation
*/

#include "ECS/Camera2DSystem.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Components/Camera2D.hpp"
#include "ECS/Components/Transform.hpp"
#include "Math/Types.hpp"
#include <cmath>
#include <random>
#include <algorithm>

namespace RType {

    namespace ECS {

        Camera2DSystem::Camera2DSystem(Renderer::IRenderer* renderer)
            : m_renderer(renderer) {
            if (m_renderer) {
                Math::Vector2 size = m_renderer->GetWindowSize();
                m_screenSize = size;
            }
        }

        void Camera2DSystem::Update(Registry& registry, float deltaTime) {
            auto cameras = registry.GetEntitiesWithComponent<Camera2D>();

            for (Entity cameraEntity : cameras) {
                auto& camera = registry.GetComponent<Camera2D>(cameraEntity);
                
                if (!camera.active) {
                    continue;
                }

                // Calculate target position
                Math::Vector2 targetPos = camera.position;
                
                if (camera.target != NULL_ENTITY && registry.HasComponent<Position>(camera.target)) {
                    auto& targetTransform = registry.GetComponent<Position>(camera.target);
                    targetPos.x = targetTransform.x + camera.offset.x;
                    targetPos.y = targetTransform.y + camera.offset.y;
                }

                // Smooth interpolation
                if (camera.smoothSpeed > 0.0f) {
                    float t = 1.0f - std::exp(-camera.smoothSpeed * deltaTime);
                    camera.position = Lerp(camera.position, targetPos, t);
                } else {
                    camera.position = targetPos;
                }

                // Apply bounds clamping
                if (camera.HasBounds()) {
                    camera.position = Clamp(camera.position, camera.boundsMin, camera.boundsMax);
                }

                // Update screen shake
                Math::Vector2 shakeOffset{0.0f, 0.0f};
                if (camera.shakeDuration > 0.0f) {
                    shakeOffset = GetShakeOffset(camera.shakeIntensity);
                    camera.shakeDuration -= deltaTime;
                    if (camera.shakeDuration <= 0.0f) {
                        camera.shakeIntensity = 0.0f;
                    }
                }

                // Store final camera position (with shake)
                m_cameraPosition = camera.position;
                m_cameraPosition.x += shakeOffset.x;
                m_cameraPosition.y += shakeOffset.y;
                m_zoom = camera.zoom;

                // Update renderer viewport if available
                if (m_renderer) {
                    m_screenSize = m_renderer->GetWindowSize();
                    
                    Renderer::Camera2D renderCam;
                    renderCam.center = m_cameraPosition;
                    renderCam.size.x = m_screenSize.x / m_zoom;
                    renderCam.size.y = m_screenSize.y / m_zoom;
                    
                    m_renderer->SetCamera(renderCam);
                }

                // Only process first active camera
                break;
            }
        }

        Math::Vector2 Camera2DSystem::ScreenToWorld(const Math::Vector2& screenPos) const {
            Math::Vector2 worldPos;
            worldPos.x = (screenPos.x / m_zoom) + m_cameraPosition.x - (m_screenSize.x / (2.0f * m_zoom));
            worldPos.y = (screenPos.y / m_zoom) + m_cameraPosition.y - (m_screenSize.y / (2.0f * m_zoom));
            return worldPos;
        }

        Math::Vector2 Camera2DSystem::WorldToScreen(const Math::Vector2& worldPos) const {
            Math::Vector2 screenPos;
            screenPos.x = (worldPos.x - m_cameraPosition.x + (m_screenSize.x / (2.0f * m_zoom))) * m_zoom;
            screenPos.y = (worldPos.y - m_cameraPosition.y + (m_screenSize.y / (2.0f * m_zoom))) * m_zoom;
            return screenPos;
        }

        Math::Vector2 Camera2DSystem::Lerp(const Math::Vector2& a, const Math::Vector2& b, float t) const {
            return Math::Vector2{
                a.x + (b.x - a.x) * t,
                a.y + (b.y - a.y) * t
            };
        }

        Math::Vector2 Camera2DSystem::Clamp(const Math::Vector2& value, const Math::Vector2& min, const Math::Vector2& max) const {
            return Math::Vector2{
                std::clamp(value.x, min.x, max.x),
                std::clamp(value.y, min.y, max.y)
            };
        }

        Math::Vector2 Camera2DSystem::GetShakeOffset(float intensity) const {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
            
            return Math::Vector2{
                dist(gen) * intensity,
                dist(gen) * intensity
            };
        }

    }

}
