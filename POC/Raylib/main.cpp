/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** POC Raylib
*/

#include "raylib.h"
#include <iostream>
#include <string>

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib POC - R-Type");
    SetTargetFPS(60);

    std::cout << "POC Test 1: Basic window and rendering" << std::endl;

    std::cout << "POC Test 2: Sprite loading and display" << std::endl;
    Image image = GenImageColor(64, 64, RED);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);

    std::cout << "POC Test 3: Input handling (keyboard/mouse)" << std::endl;

    std::cout << "POC Test 4: Audio playback functionality" << std::endl;
    InitAudioDevice();
    Sound sound = LoadSound("resources/sound.wav");
    if (sound.frameCount == 0) {
        std::cout << "  Note: Audio file not found, skipping audio test" << std::endl;
    }

    Vector2 spritePosition = {(float)screenWidth / 2 - 32, (float)screenHeight / 2 - 32};
    float rotation = 0.0f;

    std::cout << "\nControls:" << std::endl;
    std::cout << "  - Arrow keys: Move sprite" << std::endl;
    std::cout << "  - Mouse: Move sprite to cursor" << std::endl;
    std::cout << "  - Space: Play sound (if available)" << std::endl;
    std::cout << "  - ESC: Exit" << std::endl;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_RIGHT)) spritePosition.x += 2.0f;
        if (IsKeyDown(KEY_LEFT)) spritePosition.x -= 2.0f;
        if (IsKeyDown(KEY_DOWN)) spritePosition.y += 2.0f;
        if (IsKeyDown(KEY_UP)) spritePosition.y -= 2.0f;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            spritePosition.x = GetMouseX() - 32;
            spritePosition.y = GetMouseY() - 32;
        }

        if (IsKeyPressed(KEY_SPACE) && sound.frameCount > 0) {
            PlaySound(sound);
        }

        rotation += 1.0f;
        if (rotation >= 360.0f) rotation = 0.0f;

        BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Raylib POC - R-Type Engine", 10, 10, 20, DARKGRAY);
        DrawText("Arrow keys: Move | Mouse: Click to move | Space: Sound | ESC: Exit", 10, 40, 16, GRAY);

        DrawTextureEx(texture, spritePosition, rotation, 1.0f, WHITE);

        DrawLine(spritePosition.x + 32, spritePosition.y, spritePosition.x + 32, spritePosition.y + 64, BLACK);
        DrawLine(spritePosition.x, spritePosition.y + 32, spritePosition.x + 64, spritePosition.y + 32, BLACK);

        Vector2 mousePos = GetMousePosition();
        DrawText(TextFormat("Mouse: %.0f, %.0f", mousePos.x, mousePos.y), 10, 70, 14, DARKGRAY);
        DrawText(TextFormat("Sprite: %.0f, %.0f", spritePosition.x, spritePosition.y), 10, 90, 14, DARKGRAY);

        EndDrawing();
    }

    UnloadTexture(texture);
    if (sound.frameCount > 0) {
        UnloadSound(sound);
    }
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

