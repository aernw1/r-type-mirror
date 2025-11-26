#include "Renderer/SFMLRenderer.hpp"
#include "Core/Engine.hpp"
#include "Core/Logger.hpp"
#include <cmath>
#include <unordered_map>

namespace Renderer {

    SFMLRenderer::SFMLRenderer() {
        RType::Core::Logger::Debug("SFMLRenderer constructor called");
    }

    SFMLRenderer::~SFMLRenderer() {
        RType::Core::Logger::Debug("SFMLRenderer destructor called");
        Shutdown();
    }

    const char* SFMLRenderer::GetName() const {
        return "SFMLRenderer";
    }

    RType::Core::ModulePriority SFMLRenderer::GetPriority() const {
        return RType::Core::ModulePriority::High;
    }

    bool SFMLRenderer::Initialize(RType::Core::Engine* engine) {
        RType::Core::Logger::Info("Initializing SFMLRenderer module (SFML 2.6)");
        m_engine = engine;
        m_clock.restart();
        return true;
    }

    void SFMLRenderer::Shutdown() {
        RType::Core::Logger::Info("Shutting down SFMLRenderer module");

        m_sprites.clear();
        m_textures.clear();
        m_fonts.clear();

        Destroy();
        m_engine = nullptr;
    }

    void SFMLRenderer::Update(float /* deltaTime */) {
        if (!m_window || !m_window->isOpen()) {
            return;
        }

        ProcessEvents();

        m_stats.drawCalls = 0;
        m_stats.textureSwitches = 0;
    }

    bool SFMLRenderer::ShouldUpdateInRenderThread() const {
        return true;
    }

    bool SFMLRenderer::CreateWindow(const WindowConfig& config) {
        RType::Core::Logger::Info("Creating window: {}x{} - {}", config.width, config.height, config.title);

        m_windowConfig = config;

        sf::Uint32 style = sf::Style::Default;
        if (config.fullscreen) {
            style = sf::Style::Fullscreen;
        } else if (!config.resizable) {
            style = sf::Style::Titlebar | sf::Style::Close;
        }

        m_window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(config.width, config.height),
            config.title,
            style);

        if (!m_window) {
            RType::Core::Logger::Error("Failed to create SFML window");
            return false;
        }

        if (config.targetFramerate > 0) {
            m_window->setFramerateLimit(config.targetFramerate);
        }

        m_defaultView = m_window->getDefaultView();
        m_currentView = m_defaultView;

        RType::Core::Logger::Info("Window created successfully");
        return true;
    }

    void SFMLRenderer::Destroy() {
        if (m_window) {
            RType::Core::Logger::Info("Destroying window");
            m_window->close();
            m_window.reset();
        }
    }

    bool SFMLRenderer::IsWindowOpen() const {
        return m_window && m_window->isOpen();
    }

    void SFMLRenderer::Resize(std::uint32_t width, std::uint32_t height) {
        if (!m_window) {
            RType::Core::Logger::Warning("Cannot resize: window not created");
            return;
        }

        m_windowConfig.width = width;
        m_windowConfig.height = height;
        m_window->setSize(sf::Vector2u(width, height));

        m_defaultView.setSize(static_cast<float>(width), static_cast<float>(height));
        m_defaultView.setCenter(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);

        if (!m_usingCustomCamera) {
            m_currentView = m_defaultView;
            m_window->setView(m_currentView);
        }

        RType::Core::Logger::Info("Window resized to {}x{}", width, height);
    }

    void SFMLRenderer::SetWindowTitle(const std::string& title) {
        if (!m_window) {
            RType::Core::Logger::Warning("Cannot set title: window not created");
            return;
        }

        m_windowConfig.title = title;
        m_window->setTitle(title);
    }

    void SFMLRenderer::BeginFrame() {
        if (!m_window) {
            return;
        }

        m_stats.drawCalls = 0;
        m_stats.textureSwitches = 0;
    }

    void SFMLRenderer::EndFrame() {
        if (!m_window) {
            return;
        }

        m_window->display();
    }

    void SFMLRenderer::Clear(const Color& color) {
        if (!m_window) {
            return;
        }

        m_window->clear(ToSFMLColor(color));
    }

    TextureId SFMLRenderer::LoadTexture(const std::string& path, const TextureConfig& config) {
        auto texture = std::make_shared<sf::Texture>();

        if (!texture->loadFromFile(path)) {
            RType::Core::Logger::Error("Failed to load texture: {}", path);
            return 0;
        }
        texture->setSmooth(config.smooth);
        texture->setRepeated(config.repeated);

        TextureId id = m_nextTextureId++;
        m_textures[id] = {texture, config};

        RType::Core::Logger::Debug("Loaded texture: {} (ID: {})", path, id);
        return id;
    }

    void SFMLRenderer::UnloadTexture(TextureId textureId) {
        auto it = m_textures.find(textureId);
        if (it == m_textures.end()) {
            RType::Core::Logger::Warning("Attempted to unload non-existent texture ID: {}", textureId);
            return;
        }

        for (auto spriteIt = m_sprites.begin(); spriteIt != m_sprites.end();) {
            if (spriteIt->second.textureId == textureId) {
                spriteIt = m_sprites.erase(spriteIt);
            } else {
                ++spriteIt;
            }
        }

        m_textures.erase(it);
        RType::Core::Logger::Debug("Unloaded texture ID: {}", textureId);
    }

    SpriteId SFMLRenderer::CreateSprite(TextureId textureId, const Rectangle& region) {
        auto it = m_textures.find(textureId);
        if (it == m_textures.end()) {
            RType::Core::Logger::Error("Cannot create sprite: texture ID {} not found", textureId);
            return 0;
        }

        SpriteId id = m_nextSpriteId++;

        SpriteData spriteData;
        spriteData.sprite.setTexture(*it->second.texture);
        spriteData.textureId = textureId;

        if (region.size.x > 0 && region.size.y > 0) {
            spriteData.sprite.setTextureRect(sf::IntRect(
                static_cast<int>(region.position.x),
                static_cast<int>(region.position.y),
                static_cast<int>(region.size.x),
                static_cast<int>(region.size.y)));
        }

        m_sprites[id] = spriteData;

        RType::Core::Logger::Debug("Created sprite ID: {} from texture ID: {}", id, textureId);
        return id;
    }

    void SFMLRenderer::DestroySprite(SpriteId spriteId) {
        auto it = m_sprites.find(spriteId);
        if (it == m_sprites.end()) {
            RType::Core::Logger::Warning("Attempted to destroy non-existent sprite ID: {}", spriteId);
            return;
        }

        m_sprites.erase(it);
        RType::Core::Logger::Debug("Destroyed sprite ID: {}", spriteId);
    }

    void SFMLRenderer::DrawSprite(SpriteId spriteId, const Transform2D& transform, const Color& tint) {
        if (!m_window) {
            return;
        }

        auto it = m_sprites.find(spriteId);
        if (it == m_sprites.end()) {
            RType::Core::Logger::Warning("Cannot draw sprite: ID {} not found", spriteId);
            return;
        }

        sf::Sprite& sprite = it->second.sprite;

        sprite.setPosition(ToSFMLVector(transform.position));
        sprite.setScale(ToSFMLVector(transform.scale));
        sprite.setRotation(transform.rotation);
        sprite.setOrigin(ToSFMLVector(transform.origin));

        if (tint.r > 0 || tint.g > 0 || tint.b > 0 || tint.a > 0) {
            sprite.setColor(ToSFMLColor(tint));
        } else {
            sprite.setColor(sf::Color::White);
        }

        m_window->draw(sprite);
        m_stats.drawCalls++;
    }

    void SFMLRenderer::DrawRectangle(const Rectangle& rectangle, const Color& color) {
        if (!m_window) {
            return;
        }

        sf::RectangleShape rect(ToSFMLVector(rectangle.size));
        rect.setPosition(ToSFMLVector(rectangle.position));
        rect.setFillColor(ToSFMLColor(color));

        m_window->draw(rect);
        m_stats.drawCalls++;
    }

    FontId SFMLRenderer::LoadFont(const std::string& path, std::uint32_t characterSize) {
        auto font = std::make_shared<sf::Font>();

        if (!font->loadFromFile(path)) {
            RType::Core::Logger::Error("Failed to load font: {}", path);
            return 0;
        }

        FontId id = m_nextFontId++;
        m_fonts[id] = {font, characterSize};

        RType::Core::Logger::Debug("Loaded font: {} (ID: {}, size: {})", path, id, characterSize);
        return id;
    }

    void SFMLRenderer::UnloadFont(FontId fontId) {
        auto it = m_fonts.find(fontId);
        if (it == m_fonts.end()) {
            RType::Core::Logger::Warning("Attempted to unload non-existent font ID: {}", fontId);
            return;
        }

        m_fonts.erase(it);
        RType::Core::Logger::Debug("Unloaded font ID: {}", fontId);
    }

    void SFMLRenderer::DrawText(FontId fontId, const std::string& text, const TextParams& params) {
        if (!m_window) {
            return;
        }

        auto it = m_fonts.find(fontId);
        if (it == m_fonts.end()) {
            RType::Core::Logger::Warning("Cannot draw text: font ID {} not found", fontId);
            return;
        }

        sf::Text sfText;
        sfText.setFont(*it->second.font);
        sfText.setString(text);
        sfText.setCharacterSize(it->second.characterSize);
        sfText.setRotation(params.rotation);
        sfText.setFillColor(ToSFMLColor(params.color));
        sfText.setPosition(ToSFMLVector(params.position));
        sfText.setScale(sf::Vector2f(params.scale, params.scale));

        m_window->draw(sfText);
        m_stats.drawCalls++;
    }

    void SFMLRenderer::SetCamera(const Camera2D& camera) {
        if (!m_window) {
            return;
        }

        m_currentView.setCenter(ToSFMLVector(camera.center));
        m_currentView.setSize(camera.size.x, camera.size.y);
        m_window->setView(m_currentView);
        m_usingCustomCamera = true;

        RType::Core::Logger::Debug("Camera set to center: ({}, {}), size: ({}, {})",
                                   camera.center.x, camera.center.y,
                                   camera.size.x, camera.size.y);
    }

    void SFMLRenderer::ResetCamera() {
        if (!m_window) {
            return;
        }

        m_currentView = m_defaultView;
        m_window->setView(m_currentView);
        m_usingCustomCamera = false;

        RType::Core::Logger::Debug("Camera reset to default view");
    }

    RenderStats SFMLRenderer::GetRenderStats() const {
        return m_stats;
    }

    void SFMLRenderer::ProcessEvents() {
        if (!m_window) {
            return;
        }

        sf::Event event;
        while (m_window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                m_window->close();
            } else if (event.type == sf::Event::Resized) {
                Resize(event.size.width, event.size.height);
            }
        }
    }

    sf::Color SFMLRenderer::ToSFMLColor(const Color& color) {
        return sf::Color(
            static_cast<std::uint8_t>(color.r * 255.0f),
            static_cast<std::uint8_t>(color.g * 255.0f),
            static_cast<std::uint8_t>(color.b * 255.0f),
            static_cast<std::uint8_t>(color.a * 255.0f));
    }

    sf::Vector2f SFMLRenderer::ToSFMLVector(const Vector2& vec) {
        return sf::Vector2f(vec.x, vec.y);
    }

    sf::IntRect SFMLRenderer::ToSFMLRect(const Rectangle& rect) {
        return sf::IntRect(
            static_cast<int>(rect.position.x),
            static_cast<int>(rect.position.y),
            static_cast<int>(rect.size.x),
            static_cast<int>(rect.size.y));
    }

    sf::Keyboard::Key SFMLRenderer::ToSFMLKey(Key key) {
        static const std::unordered_map<Key, sf::Keyboard::Key> keyMapping = {
            {Key::A, sf::Keyboard::A},
            {Key::B, sf::Keyboard::B},
            {Key::C, sf::Keyboard::C},
            {Key::D, sf::Keyboard::D},
            {Key::E, sf::Keyboard::E},
            {Key::F, sf::Keyboard::F},
            {Key::G, sf::Keyboard::G},
            {Key::H, sf::Keyboard::H},
            {Key::I, sf::Keyboard::I},
            {Key::J, sf::Keyboard::J},
            {Key::K, sf::Keyboard::K},
            {Key::L, sf::Keyboard::L},
            {Key::M, sf::Keyboard::M},
            {Key::N, sf::Keyboard::N},
            {Key::O, sf::Keyboard::O},
            {Key::P, sf::Keyboard::P},
            {Key::Q, sf::Keyboard::Q},
            {Key::R, sf::Keyboard::R},
            {Key::S, sf::Keyboard::S},
            {Key::T, sf::Keyboard::T},
            {Key::U, sf::Keyboard::U},
            {Key::V, sf::Keyboard::V},
            {Key::W, sf::Keyboard::W},
            {Key::X, sf::Keyboard::X},
            {Key::Y, sf::Keyboard::Y},
            {Key::Z, sf::Keyboard::Z},
            {Key::Num0, sf::Keyboard::Num0},
            {Key::Num1, sf::Keyboard::Num1},
            {Key::Num2, sf::Keyboard::Num2},
            {Key::Num3, sf::Keyboard::Num3},
            {Key::Num4, sf::Keyboard::Num4},
            {Key::Num5, sf::Keyboard::Num5},
            {Key::Num6, sf::Keyboard::Num6},
            {Key::Num7, sf::Keyboard::Num7},
            {Key::Num8, sf::Keyboard::Num8},
            {Key::Num9, sf::Keyboard::Num9},
            {Key::Escape, sf::Keyboard::Escape},
            {Key::Space, sf::Keyboard::Space},
            {Key::Enter, sf::Keyboard::Return},
            {Key::Backspace, sf::Keyboard::BackSpace},
            {Key::Tab, sf::Keyboard::Tab},
            {Key::Left, sf::Keyboard::Left},
            {Key::Right, sf::Keyboard::Right},
            {Key::Up, sf::Keyboard::Up},
            {Key::Down, sf::Keyboard::Down},
            {Key::LShift, sf::Keyboard::LShift},
            {Key::RShift, sf::Keyboard::RShift},
            {Key::LControl, sf::Keyboard::LControl},
            {Key::RControl, sf::Keyboard::RControl},
            {Key::LAlt, sf::Keyboard::LAlt},
            {Key::RAlt, sf::Keyboard::RAlt}};

        auto it = keyMapping.find(key);
        return (it != keyMapping.end()) ? it->second : sf::Keyboard::Unknown;
    }

    bool SFMLRenderer::IsKeyPressed(Key key) const {
        return sf::Keyboard::isKeyPressed(ToSFMLKey(key));
    }

}

extern "C" {
#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
RType::Core::IModule*
CreateModule() {
    return new Renderer::SFMLRenderer();
}

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
void DestroyModule(RType::Core::IModule* module) {
    delete module;
}
};
