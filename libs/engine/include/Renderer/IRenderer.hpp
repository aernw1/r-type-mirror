#pragma once

#define RTYPE_INCLUDE_WINDOWS_H
#include "Core/Platform.hpp"
#include <cstdint>
#include <string>
#include "Core/Module.hpp"
#include "Math/Types.hpp"

namespace Renderer {

    using TextureId = std::uint32_t;
    using SpriteId = std::uint32_t;
    using FontId = std::uint32_t;

    constexpr TextureId INVALID_TEXTURE_ID = 0;
    constexpr SpriteId INVALID_SPRITE_ID = 0;
    constexpr FontId INVALID_FONT_ID = 0;

    using Math::Color;
    using Math::Rectangle;
    using Math::Vector2;

    struct WindowConfig {
        std::string title{"R-Type"};
        std::uint32_t width = 1280;
        std::uint32_t height = 720;
        bool fullscreen = false;
        bool resizable = false;
        std::uint32_t targetFramerate = 60;
    };

    struct TextureConfig {
        bool smooth = true;
        bool repeated = false;
        bool generateMipmaps = false;
    };

    struct Transform2D {
        Vector2 position{0.0f, 0.0f};
        Vector2 scale{1.0f, 1.0f};
        float rotation = 0.0f;
        Vector2 origin{0.0f, 0.0f};
    };

    struct TextParams {
        Vector2 position{0.0f, 0.0f};
        Color color{};
        float rotation = 0.0f;
        float scale = 1.0f;
        float letterSpacing = 0.0f;
        float lineSpacing = 0.0f;
    };

    struct Camera2D {
        Vector2 center{0.0f, 0.0f};
        Vector2 size{1280.0f, 720.0f};
    };

    struct RenderStats {
        std::uint32_t drawCalls = 0;
        std::uint32_t textureSwitches = 0;
    };

    enum class Key {
        Unknown = -1,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Escape,
        Space,
        Enter,
        Backspace,
        Tab,
        Left,
        Right,
        Up,
        Down,
        LShift,
        RShift,
        LControl,
        RControl,
        LAlt,
        RAlt
    };

    class IRenderer : public RType::Core::IModule {
    public:
        ~IRenderer() override = default;

        const char* GetName() const override = 0;
        RType::Core::ModulePriority GetPriority() const override = 0;
        bool Initialize(RType::Core::Engine* engine) override = 0;
        void Shutdown() override = 0;
        void Update(float deltaTime) override = 0;

        virtual bool CreateWindow(const WindowConfig& config) = 0;
        virtual void Destroy() = 0;
        virtual bool IsWindowOpen() const = 0;
        virtual void Resize(std::uint32_t width, std::uint32_t height) = 0;
        virtual void SetWindowTitle(const std::string& title) = 0;

        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Clear(const Color& color) = 0;

        virtual TextureId LoadTexture(const std::string& path,
                                      const TextureConfig& config = TextureConfig{}) = 0;
        virtual void UnloadTexture(TextureId textureId) = 0;

        virtual SpriteId CreateSprite(TextureId textureId, const Rectangle& region) = 0;
        virtual void DestroySprite(SpriteId spriteId) = 0;

        virtual void DrawSprite(SpriteId spriteId, const Transform2D& transform,
                                const Color& tint = Color{}) = 0;
        virtual void DrawRectangle(const Rectangle& rectangle, const Color& color) = 0;

        virtual FontId LoadFont(const std::string& path, std::uint32_t characterSize) = 0;
        virtual void UnloadFont(FontId fontId) = 0;
        virtual void DrawText(FontId fontId, const std::string& text, const TextParams& params) = 0;

        virtual void SetCamera(const Camera2D& camera) = 0;
        virtual void ResetCamera() = 0;

        virtual RenderStats GetRenderStats() const = 0;

        virtual bool IsKeyPressed(Key key) const = 0;
    };

}
