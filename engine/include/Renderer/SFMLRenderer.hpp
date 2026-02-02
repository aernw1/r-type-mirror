#pragma once

#include "IRenderer.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>

namespace Renderer {

    class SFMLRenderer : public IRenderer {
    public:
        SFMLRenderer();
        ~SFMLRenderer() override;

        const char* GetName() const override;
        RType::Core::ModulePriority GetPriority() const override;
        bool Initialize(RType::Core::Engine* engine) override;
        void Shutdown() override;
        void Update(float deltaTime) override;
        bool ShouldUpdateInRenderThread() const override;

        bool CreateWindow(const WindowConfig& config) override;
        void Destroy() override;
        bool IsWindowOpen() const override;
        Vector2 GetWindowSize() const override;
        void Resize(std::uint32_t width, std::uint32_t height) override;
        void SetWindowTitle(const std::string& title) override;

        void BeginFrame() override;
        void EndFrame() override;
        void Clear(const Color& color) override;

        TextureId LoadTexture(const std::string& path,
                              const TextureConfig& config = TextureConfig{}) override;
        void UnloadTexture(TextureId textureId) override;
        Vector2 GetTextureSize(TextureId textureId) const override;

        SpriteId CreateSprite(TextureId textureId, const Rectangle& region) override;
        void DestroySprite(SpriteId spriteId) override;
        void SetSpriteRegion(SpriteId spriteId, const Rectangle& region) override;

        void DrawSprite(SpriteId spriteId, const Transform2D& transform,
                        const Color& tint = Color{}) override;
        void DrawRectangle(const Rectangle& rectangle, const Color& color) override;

        FontId LoadFont(const std::string& path, std::uint32_t characterSize) override;
        void UnloadFont(FontId fontId) override;
        void DrawText(FontId fontId, const std::string& text,
                      const TextParams& params) override;

        void SetCamera(const Camera2D& camera) override;
        void ResetCamera() override;

        RenderStats GetRenderStats() const override;

        float GetDeltaTime() override;

        bool IsKeyPressed(Key key) const override;
        bool IsMouseButtonPressed(MouseButton button) const override;
        Vector2 GetMousePosition() const override;

        const sf::RenderWindow* GetWindow() const { return m_window.get(); }
        void ProcessEvents();
    private:
        static sf::Color ToSFMLColor(const Color& color);
        static sf::Vector2f ToSFMLVector(const Vector2& vec);
        static sf::IntRect ToSFMLRect(const Rectangle& rect);
        static sf::Keyboard::Key ToSFMLKey(Key key);

        std::unique_ptr<sf::RenderWindow> m_window;
        WindowConfig m_windowConfig;

        struct TextureData {
            std::shared_ptr<sf::Texture> texture;
            TextureConfig config;
        };
        std::unordered_map<TextureId, TextureData> m_textures;
        TextureId m_nextTextureId = 1;

        struct SpriteData {
            sf::Sprite sprite;
            TextureId textureId;
        };
        std::unordered_map<SpriteId, SpriteData> m_sprites;
        SpriteId m_nextSpriteId = 1;

        struct FontData {
            std::shared_ptr<sf::Font> font;
            std::uint32_t characterSize;
        };
        std::unordered_map<FontId, FontData> m_fonts;
        FontId m_nextFontId = 1;

        sf::View m_defaultView;
        sf::View m_currentView;
        bool m_usingCustomCamera = false;

        RType::Core::Engine* m_engine = nullptr;

        RenderStats m_stats;

        sf::Clock m_clock;
        float m_lastDeltaTime = 0.0f;
    };

}
