/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorCanvasManager
*/

#include "editor/EditorCanvasManager.hpp"
#include <cmath>
#include <algorithm>

namespace RType {
    namespace Client {

        EditorCanvasManager::EditorCanvasManager(Renderer::IRenderer* renderer)
            : m_renderer(renderer)
        {
        }

        void EditorCanvasManager::HandleCameraInput() {
            const float panSpeed = 500.0f;
            const float zoomSpeed = 0.1f;

            if (m_renderer->IsKeyPressed(Renderer::Key::Up) && !m_upKeyPressed) {
                m_upKeyPressed = true;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Up)) {
                m_upKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Down) && !m_downKeyPressed) {
                m_downKeyPressed = true;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Down)) {
                m_downKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Left) && !m_leftKeyPressed) {
                m_leftKeyPressed = true;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Left)) {
                m_leftKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Right) && !m_rightKeyPressed) {
                m_rightKeyPressed = true;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Right)) {
                m_rightKeyPressed = false;
            }

            if (m_upKeyPressed) {
                m_camera.y -= panSpeed * (1.0f / 60.0f) / m_camera.zoom;
            }
            if (m_downKeyPressed) {
                m_camera.y += panSpeed * (1.0f / 60.0f) / m_camera.zoom;
            }
            if (m_leftKeyPressed) {
                m_camera.x -= panSpeed * (1.0f / 60.0f) / m_camera.zoom;
            }
            if (m_rightKeyPressed) {
                m_camera.x += panSpeed * (1.0f / 60.0f) / m_camera.zoom;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Num0) && !m_plusKeyPressed) {
                m_plusKeyPressed = true;
                m_camera.zoom += zoomSpeed;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Num0)) {
                m_plusKeyPressed = false;
            }

            if (m_renderer->IsKeyPressed(Renderer::Key::Num9) && !m_minusKeyPressed) {
                m_minusKeyPressed = true;
                m_camera.zoom -= zoomSpeed;
            } else if (!m_renderer->IsKeyPressed(Renderer::Key::Num9)) {
                m_minusKeyPressed = false;
            }

            m_camera.x = std::clamp(m_camera.x, m_camera.minX, m_camera.maxX);
            m_camera.y = std::clamp(m_camera.y, m_camera.minY, m_camera.maxY);
            m_camera.zoom = std::clamp(m_camera.zoom, m_camera.minZoom, m_camera.maxZoom);
        }

        void EditorCanvasManager::ApplyCamera() {
            Renderer::Camera2D cam;
            cam.center = {m_camera.x, m_camera.y};
            cam.size = {1280.0f / m_camera.zoom, 720.0f / m_camera.zoom};
            m_renderer->SetCamera(cam);
        }

        void EditorCanvasManager::DrawGrid() {
            if (!m_grid.enabled) {
                return;
            }

            Math::Vector2 topLeft = ScreenToWorld({0.0f, 0.0f});
            Math::Vector2 bottomRight = ScreenToWorld({1280.0f, 720.0f});

            float startX = std::floor(topLeft.x / m_grid.cellSize) * m_grid.cellSize;
            float startY = std::floor(topLeft.y / m_grid.cellSize) * m_grid.cellSize;
            float endX = std::ceil(bottomRight.x / m_grid.cellSize) * m_grid.cellSize;
            float endY = std::ceil(bottomRight.y / m_grid.cellSize) * m_grid.cellSize;

            for (float x = startX; x <= endX; x += m_grid.cellSize) {
                Math::Rectangle rect = {{x, startY}, {1.0f, endY - startY}};
                m_renderer->DrawRectangle(rect, m_grid.gridColor);
            }

            for (float y = startY; y <= endY; y += m_grid.cellSize) {
                Math::Rectangle rect = {{startX, y}, {endX - startX, 1.0f}};
                m_renderer->DrawRectangle(rect, m_grid.gridColor);
            }
        }

        Math::Vector2 EditorCanvasManager::ScreenToWorld(Math::Vector2 screenPos) const {
            float worldX = (screenPos.x - 640.0f) / m_camera.zoom + m_camera.x;
            float worldY = (screenPos.y - 360.0f) / m_camera.zoom + m_camera.y;
            return {worldX, worldY};
        }

        Math::Vector2 EditorCanvasManager::WorldToScreen(Math::Vector2 worldPos) const {
            float screenX = (worldPos.x - m_camera.x) * m_camera.zoom + 640.0f;
            float screenY = (worldPos.y - m_camera.y) * m_camera.zoom + 360.0f;
            return {screenX, screenY};
        }

        float EditorCanvasManager::SnapToGrid(float value) const {
            if (!m_grid.snapToGrid) {
                return value;
            }
            return std::round(value / m_grid.cellSize) * m_grid.cellSize;
        }

    }
}
