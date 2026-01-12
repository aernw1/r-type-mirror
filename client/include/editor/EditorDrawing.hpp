/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorDrawing
*/

#pragma once

#include "Math/Types.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/LevelLoader.hpp"

namespace RType {
    namespace Client {
        namespace EditorDrawing {

            void DrawRectangleOutline(Renderer::IRenderer* renderer,
                                      const Math::Rectangle& rect,
                                      const Math::Color& color,
                                      float thickness = 3.0f);

            void DrawCollider(Renderer::IRenderer* renderer,
                              const ECS::ColliderDef& collider,
                              const Math::Color& color,
                              float lineThickness = 2.0f);

            void DrawHandle(Renderer::IRenderer* renderer,
                            const Math::Vector2& position,
                            const Math::Color& color,
                            float size = 8.0f);

        }
    }
}
