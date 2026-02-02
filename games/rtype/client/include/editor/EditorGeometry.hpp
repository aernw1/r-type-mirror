/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorGeometry
*/

#pragma once

#include "Math/Types.hpp"

namespace RType {
    namespace Client {
        namespace EditorGeometry {

            inline Math::Rectangle BuildRect(float x, float y, float width, float height) {
                Math::Rectangle rect;
                rect.position = {x, y};
                rect.size = {width, height};
                return rect;
            }

            inline bool PointInRect(const Math::Vector2& point, const Math::Rectangle& rect) {
                return point.x >= rect.position.x &&
                       point.x <= rect.position.x + rect.size.x &&
                       point.y >= rect.position.y &&
                       point.y <= rect.position.y + rect.size.y;
            }

            inline Math::Rectangle GetEntityBounds(float centerX, float centerY, float width, float height) {
                return BuildRect(centerX - width / 2.0f, centerY - height / 2.0f, width, height);
            }

            inline Math::Rectangle GetHandleBounds(float x, float y, float handleSize) {
                return BuildRect(x - handleSize / 2.0f, y - handleSize / 2.0f, handleSize, handleSize);
            }

            inline bool PointInEntityBounds(const Math::Vector2& point,
                                             float centerX, float centerY,
                                             float width, float height) {
                float left = centerX - width / 2.0f;
                float right = centerX + width / 2.0f;
                float top = centerY - height / 2.0f;
                float bottom = centerY + height / 2.0f;

                return point.x >= left && point.x <= right &&
                       point.y >= top && point.y <= bottom;
            }

        }
    }
}
