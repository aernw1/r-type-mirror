/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Camera2DSystem - Updates camera position based on target following
*/

#pragma once

#include "ISystem.hpp"
#include "../Renderer/IRenderer.hpp"
#include "../Math/Types.hpp"

namespace RType {

    namespace ECS {

        /**
         * @brief System that updates Camera2D components
         * 
         * This system handles:
         * - Smooth target following with configurable speed
         * - World bounds clamping
         * - Screen shake effects
         * - Viewport updates on renderer
         * 
         * The system should be registered early to ensure camera position
         * is updated before rendering systems use it.
         */
        class Camera2DSystem : public ISystem {
        public:
            explicit Camera2DSystem(Renderer::IRenderer* renderer = nullptr);
            ~Camera2DSystem() override = default;

            void Update(Registry& registry, float deltaTime) override;
            const char* GetName() const override { return "Camera2DSystem"; }

            /**
             * @brief Set the renderer for viewport updates
             * @param renderer The renderer to update
             */
            void SetRenderer(Renderer::IRenderer* renderer) { m_renderer = renderer; }

            /**
             * @brief Get the current active camera position
             * @return Camera position in world coordinates
             */
            Math::Vector2 GetCameraPosition() const { return m_cameraPosition; }

            /**
             * @brief Convert screen coordinates to world coordinates
             * @param screenPos Position in screen space
             * @return Position in world space
             */
            Math::Vector2 ScreenToWorld(const Math::Vector2& screenPos) const;

            /**
             * @brief Convert world coordinates to screen coordinates
             * @param worldPos Position in world space
             * @return Position in screen space
             */
            Math::Vector2 WorldToScreen(const Math::Vector2& worldPos) const;

            /**
             * @brief Get the current camera zoom
             */
            float GetZoom() const { return m_zoom; }

        private:
            Renderer::IRenderer* m_renderer = nullptr;
            Math::Vector2 m_cameraPosition{0.0f, 0.0f};
            Math::Vector2 m_screenSize{1920.0f, 1080.0f};
            float m_zoom = 1.0f;

            Math::Vector2 Lerp(const Math::Vector2& a, const Math::Vector2& b, float t) const;
            Math::Vector2 Clamp(const Math::Vector2& value, const Math::Vector2& min, const Math::Vector2& max) const;
            Math::Vector2 GetShakeOffset(float intensity) const;
        };

    }

}
