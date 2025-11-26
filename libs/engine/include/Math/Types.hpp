#pragma once

namespace Math {

    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;
        Vector2(float x, float y)
            : x(x), y(y) {}
    };

    struct Color {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        Color() = default;
        Color(float r, float g, float b, float a = 1.0f)
            : r(r), g(g), b(b), a(a) {}
    };

    struct Rectangle {
        Vector2 position{0.0f, 0.0f};
        Vector2 size{0.0f, 0.0f};
    };

}
