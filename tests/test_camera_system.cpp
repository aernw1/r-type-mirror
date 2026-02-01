/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test Camera2DSystem
*/

#include "ECS/Components/Camera2D.hpp"
#include "ECS/Camera2DSystem.hpp"
#include "Renderer/IRenderer.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Component.hpp"
#include <iostream>
#include <cassert>

class MockRenderer : public Renderer::IRenderer {
public:
    Renderer::Vector2 GetWindowSize() const override {
        return {1280.0f, 720.0f};
    }
    
    // Minimal override stubs for abstract class (returning default values)
    const char* GetName() const override { return "Mock"; }
    RType::Core::ModulePriority GetPriority() const override { return RType::Core::ModulePriority::Normal; }
    bool Initialize(RType::Core::Engine*) override { return true; }
    void Shutdown() override {}
    void Update(float) override {}
    bool ShouldUpdateInRenderThread() const override { return false; }
    bool CreateWindow(const Renderer::WindowConfig&) override { return true; }
    void Destroy() override {}
    bool IsWindowOpen() const override { return true; }
    void Resize(std::uint32_t, std::uint32_t) override {}
    void SetWindowTitle(const std::string&) override {}
    void BeginFrame() override {}
    void EndFrame() override {}
    void Clear(const Renderer::Color&) override {}
    Renderer::TextureId LoadTexture(const std::string&, const Renderer::TextureConfig&) override { return 0; }
    void UnloadTexture(Renderer::TextureId) override {}
    Renderer::Vector2 GetTextureSize(Renderer::TextureId) const override { return {0,0}; }
    Renderer::SpriteId CreateSprite(Renderer::TextureId, const Renderer::Rectangle&) override { return 0; }
    void DestroySprite(Renderer::SpriteId) override {}
    void SetSpriteRegion(Renderer::SpriteId, const Renderer::Rectangle&) override {}
    void DrawSprite(Renderer::SpriteId, const Renderer::Transform2D&, const Renderer::Color&) override {}
    void DrawRectangle(const Renderer::Rectangle&, const Renderer::Color&) override {}
    Renderer::FontId LoadFont(const std::string&, std::uint32_t) override { return 0; }
    void UnloadFont(Renderer::FontId) override {}
    void DrawText(Renderer::FontId, const std::string&, const Renderer::TextParams&) override {}
    
    void SetCamera(const Renderer::Camera2D& cam) override {
        lastCamera = cam;
        cameraSet = true;
    }
    void ResetCamera() override {
        cameraSet = false;
    }
    Renderer::RenderStats GetRenderStats() const override { return {}; }
    float GetDeltaTime() override { return 0.016f; }
    bool IsKeyPressed(Renderer::Key) const override { return false; }
    bool IsMouseButtonPressed(Renderer::MouseButton) const override { return false; }
    Renderer::Vector2 GetMousePosition() const override { return {0,0}; }
    
    Renderer::Camera2D lastCamera;
    bool cameraSet = false;
};

int main() {
    MockRenderer renderer;
    RType::ECS::Camera2DSystem cameraSystem(&renderer);
    RType::ECS::Registry registry;
    
    // Create camera entity
    auto camEntity = registry.CreateEntity();
    auto& cam = registry.AddComponent<RType::ECS::Camera2D>(camEntity);
    cam.active = true;
    cam.zoom = 1.0f;
    cam.smoothSpeed = 100.0f; // High speed for instant snap test or multiple updates
    
    // Create target entity
    auto targetEntity = registry.CreateEntity();
    registry.AddComponent<RType::ECS::Position>(targetEntity, RType::ECS::Position{100.0f, 200.0f});
    
    // Set target
    cam.target = targetEntity;
    
    // Initial update
    cameraSystem.Update(registry, 1.0f);
    
    // Implementation detail: Camera2DSystem centers camera
    // Window size (1280, 720) -> Center is offset (640, 360)
    // If target is at (100, 200), Camera should center on it.
    // SFML View center should be (100, 200).
    // Let's verify what SetCamera received.
    
    assert(renderer.cameraSet && "SetCamera should be called");
    // Depending on implementation, Camera2DSystem logic might vary.
    // Typically: view.setCenter(targetPos + offset)
    assert(renderer.lastCamera.center.x == 100.0f);
    assert(renderer.lastCamera.center.y == 200.0f);
    
    // Test screen to world
    Math::Vector2 screenPos{640.0f, 360.0f}; // Center of screen
    Math::Vector2 worldPos = cameraSystem.ScreenToWorld(screenPos);
    // Should be approx 100, 200
    std::cout << "Screen(640,360) -> World(" << worldPos.x << "," << worldPos.y << ")" << std::endl;
    
    std::cout << "Camera2DSystem tests passed!" << std::endl;
    return 0;
}
