/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorCanvasManager
*/

#pragma once

#include "EditorTypes.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <memory>

namespace RType {
    namespace Client {

        class EditorCanvasManager {
        public:
            explicit EditorCanvasManager(Renderer::IRenderer* renderer);
            ~EditorCanvasManager() = default;

            void HandleCameraInput();
            void ApplyCamera();
            void DrawGrid();

            Math::Vector2 ScreenToWorld(Math::Vector2 screenPos) const;
            Math::Vector2 WorldToScreen(Math::Vector2 worldPos) const;

            const EditorCamera& GetCamera() const { return m_camera; }
            const EditorGrid& GetGrid() const { return m_grid; }

            void ToggleGridSnap() { m_grid.snapToGrid = !m_grid.snapToGrid; }
            float SnapToGrid(float value) const;

        private:
            Renderer::IRenderer* m_renderer;
            EditorCamera m_camera;
            EditorGrid m_grid;
        };

    }
}
