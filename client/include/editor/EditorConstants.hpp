/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** EditorConstants
*/

#pragma once

#include "Math/Types.hpp"

namespace RType {
    namespace Client {
        namespace EditorConstants {

            namespace UI {
                constexpr float PALETTE_PANEL_LEFT = 10.0f;
                constexpr float PALETTE_PANEL_WIDTH = 220.0f;
                constexpr float PALETTE_BUTTON_HEIGHT = 22.0f;
                constexpr float PALETTE_START_Y = 80.0f;

                constexpr float PROPERTY_PANEL_X = 920.0f;
                constexpr float PROPERTY_PANEL_WIDTH = 340.0f;
                constexpr float PROPERTY_PANEL_START_Y = 90.0f;
                constexpr float PROPERTY_ROW_HEIGHT = 28.0f;
                constexpr float PROPERTY_VALUE_OFFSET_X = 100.0f;

                constexpr float STATUS_BAR_X = 10.0f;
                constexpr float STATUS_BAR_Y = 10.0f;
                constexpr float STATUS_BAR_LINE_HEIGHT = 14.0f;
            }

            namespace Colors {
                inline const Math::Color BACKGROUND{0.1f, 0.1f, 0.15f, 1.0f};
                inline const Math::Color GRID{0.3f, 0.3f, 0.3f, 0.5f};
                inline const Math::Color SELECTION_OUTLINE{1.0f, 1.0f, 0.0f, 1.0f};
                constexpr float PREVIEW_ALPHA = 0.35f;

                inline const Math::Color UI_HEADER{0.4f, 0.86f, 0.9f, 1.0f};
                inline const Math::Color UI_TEXT{0.75f, 0.75f, 0.8f, 1.0f};
                inline const Math::Color UI_HINT{0.5f, 0.86f, 1.0f, 0.7f};
                inline const Math::Color UI_ACTIVE{1.0f, 0.7f, 0.0f, 1.0f};
                inline const Math::Color UI_HOVER{0.9f, 0.9f, 0.95f, 1.0f};
            }

            namespace Camera {
                constexpr float PAN_SPEED = 500.0f;
                constexpr float ZOOM_SPEED = 0.1f;
                constexpr float INITIAL_X = 640.0f;
                constexpr float INITIAL_Y = 360.0f;
                constexpr float INITIAL_ZOOM = 1.0f;

                constexpr float MIN_X = -1000.0f;
                constexpr float MAX_X = 15000.0f;
                constexpr float MIN_Y = -500.0f;
                constexpr float MAX_Y = 1200.0f;
                constexpr float MIN_ZOOM = 0.25f;
                constexpr float MAX_ZOOM = 2.0f;
            }

            namespace Grid {
                constexpr float CELL_SIZE = 50.0f;
                constexpr float LINE_THICKNESS = 1.0f;
            }

            namespace PropertySteps {
                constexpr float POSITION_STEP = 25.0f;
                constexpr float SCALE_STEP = 10.0f;
                constexpr float LAYER_STEP = 1.0f;
                constexpr float SCROLL_SPEED_STEP = 5.0f;
                constexpr float MIN_SCALE = 10.0f;
            }

            namespace Collider {
                constexpr float HANDLE_SIZE = 8.0f;
                constexpr float MIN_COLLIDER_SIZE = 10.0f;
                constexpr float COLLIDER_LINE_THICKNESS = 2.0f;
                constexpr float SNAP_DISTANCE = 5.0f;

                inline const Math::Color COLLIDER_NORMAL{0.0f, 1.0f, 0.0f, 0.6f};
                inline const Math::Color COLLIDER_SELECTED{1.0f, 0.5f, 0.0f, 0.8f};
                inline const Math::Color COLLIDER_HANDLE{1.0f, 1.0f, 0.0f, 1.0f};
            }

            namespace Input {
                constexpr size_t MAX_INPUT_BUFFER_SIZE = 8;
                constexpr int NUMBER_KEY_COUNT = 10;
            }

            namespace Selection {
                constexpr float OUTLINE_THICKNESS = 3.0f;
                inline const Math::Color OUTLINE_COLOR{1.0f, 0.85f, 0.35f, 1.0f};
            }

        }
    }
}
