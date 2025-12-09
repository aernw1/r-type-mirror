/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** MapResources - Définitions des obstacles et ressources de map
*/

#pragma once

#include <vector>
#include <string>

namespace RType {
    namespace Client {
        namespace MapResources {

            struct ObstacleDefinition {
                const char* texturePath;
                float x;
                float y;
                float colliderWidth;
                float colliderHeight;
                float scaleX;
                float scaleY;
            };

            inline const std::vector<ObstacleDefinition> LEVEL_1_OBSTACLES = {
                // Obstacle 1: violet_square
                {"assets/backgrounds/obstacles/violet_square.png", 500.0f, 100.0f, 64.0f, 64.0f, 1.5f, 1.5f},

                // Obstacle 2: obstacle_yellow
                {"assets/backgrounds/obstacles/obstacle_yellow.png", 1000.0f, 300.0f, 120.0f, 80.0f, 1.0f, 1.0f},

                // Obstacle 3: little_obstacle
                {"assets/backgrounds/obstacles/little_obstacle.png", 1200.0f, 150.0f, 100.0f, 60.0f, 2.0f, 2.0f},

                // Obstacle 4: middle_obstacle
                {"assets/backgrounds/obstacles/middle_obstacle.png", 1300.0f, 400.0f, 150.0f, 100.0f, 1.5f, 1.5f},

                // Obstacle 5: obstacle_bas
                {"assets/backgrounds/obstacles/obstacle_bas.png", 1800.0f, 600.0f, 140.0f, 90.0f, 3.0f, 3.0f},

                // Obstacle 6: violet_rectangle
                {"assets/backgrounds/obstacles/violet_rectangle.png", 2200.0f, 200.0f, 80.0f, 120.0f, 2.0f, 2.0f},

                // Obstacle 7: obstacle_square_green
                {"assets/backgrounds/obstacles/obstacle_square_green.png", 2800.0f, 350.0f, 64.0f, 64.0f, 1.0f, 1.0f},

                // Obstacle 8: ostacle_triangle_green
                {"assets/backgrounds/obstacles/ostacle_triangle_green.png", 3400.0f, 450.0f, 130.0f, 85.0f, 1.5f, 1.5f},

                // Obstacle 9: obstacle_square_yellow
                {"assets/backgrounds/obstacles/obstacle_square_yellow.png", 4000.0f, 100.0f, 64.0f, 64.0f, 1.0f, 1.0f},

                // Obstacle 10: ostacle_bas_two
                {"assets/backgrounds/obstacles/ostacle_bas_two.png", 4600.0f, 550.0f, 145.0f, 95.0f, 2.5f, 2.5f},
            };

            constexpr float LEVEL_1_MAP_WIDTH = 10000.0f;
            constexpr float LEVEL_1_MAP_HEIGHT = 720.0f;
            constexpr const char* LEVEL_1_BACKGROUND = "assets/backgrounds/Cave_one.png";

        }
    }
}
