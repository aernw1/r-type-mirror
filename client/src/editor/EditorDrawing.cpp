/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorDrawing
*/

#include "editor/EditorDrawing.hpp"
#include "editor/EditorGeometry.hpp"

namespace RType {
    namespace Client {
        namespace EditorDrawing {

            void DrawRectangleOutline(Renderer::IRenderer* renderer,
                                      const Math::Rectangle& rect,
                                      const Math::Color& color,
                                      float thickness) {
                if (!renderer) {
                    return;
                }

                // Draw four rectangles for the border (top, bottom, left, right)
                Math::Rectangle top = EditorGeometry::BuildRect(
                    rect.position.x, rect.position.y, rect.size.x, thickness);
                Math::Rectangle bottom = EditorGeometry::BuildRect(
                    rect.position.x, rect.position.y + rect.size.y - thickness, rect.size.x, thickness);
                Math::Rectangle left = EditorGeometry::BuildRect(
                    rect.position.x, rect.position.y, thickness, rect.size.y);
                Math::Rectangle right = EditorGeometry::BuildRect(
                    rect.position.x + rect.size.x - thickness, rect.position.y, thickness, rect.size.y);

                renderer->DrawRectangle(top, color);
                renderer->DrawRectangle(bottom, color);
                renderer->DrawRectangle(left, color);
                renderer->DrawRectangle(right, color);
            }

            void DrawCollider(Renderer::IRenderer* renderer,
                              const ECS::ColliderDef& collider,
                              const Math::Color& color,
                              float lineThickness) {
                if (!renderer) {
                    return;
                }

                // Draw collider as an outline (same pattern as DrawRectangleOutline)
                Math::Rectangle top = EditorGeometry::BuildRect(
                    collider.x, collider.y, collider.width, lineThickness);
                Math::Rectangle bottom = EditorGeometry::BuildRect(
                    collider.x, collider.y + collider.height - lineThickness, collider.width, lineThickness);
                Math::Rectangle left = EditorGeometry::BuildRect(
                    collider.x, collider.y, lineThickness, collider.height);
                Math::Rectangle right = EditorGeometry::BuildRect(
                    collider.x + collider.width - lineThickness, collider.y, lineThickness, collider.height);

                renderer->DrawRectangle(top, color);
                renderer->DrawRectangle(bottom, color);
                renderer->DrawRectangle(left, color);
                renderer->DrawRectangle(right, color);
            }

            void DrawHandle(Renderer::IRenderer* renderer,
                            const Math::Vector2& position,
                            const Math::Color& color,
                            float size) {
                if (!renderer) {
                    return;
                }

                const float halfSize = size / 2.0f;
                Math::Rectangle handle = EditorGeometry::BuildRect(
                    position.x - halfSize, position.y - halfSize, size, size);
                renderer->DrawRectangle(handle, color);
            }

        }
    }
}
