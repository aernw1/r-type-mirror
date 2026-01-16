/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Test for hit animation effect
*/

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include "ECS/Registry.hpp"
#include "ECS/EffectFactory.hpp"
#include "ECS/Component.hpp"
#include "ECS/AnimationSystem.hpp"
#include "ECS/RenderingSystem.hpp"
#include "Animation/AnimationModule.hpp"
#include "Renderer/SFMLRenderer.hpp"
#include "Core/Logger.hpp"
#include "Core/Engine.hpp"
#include "Core/Platform.hpp"

using namespace RType;
using namespace RType::ECS;
using namespace Animation;
using namespace Renderer;

int main() {
    std::cout << "=== Hit Animation Test ===" << std::endl;

    Core::Logger::SetLogLevel(Core::LogLevel::Info);

    // Initialize engine and load renderer plugin
    auto engine = std::make_unique<Core::Engine>();
    Core::IModule* module = nullptr;

    module = engine->LoadPlugin("lib/" + Core::Platform::GetPluginPath("SFMLRenderer"));
    if (!module) {
        module = engine->LoadPlugin(Core::Platform::GetPluginPathFromBin("SFMLRenderer"));
    }

    if (!module) {
        std::cerr << "ERROR: Failed to load SFMLRenderer plugin" << std::endl;
        std::cerr << "Make sure to run from build/ directory: ./bin/test_hit_animation" << std::endl;
        return 1;
    }

    auto* rendererPtr = dynamic_cast<Renderer::IRenderer*>(module);
    if (!rendererPtr) {
        std::cerr << "ERROR: Failed to cast module to IRenderer" << std::endl;
        return 1;
    }

    if (!engine->Initialize()) {
        std::cerr << "ERROR: Failed to initialize engine" << std::endl;
        return 1;
    }

    auto renderer = std::shared_ptr<Renderer::IRenderer>(rendererPtr, [](Renderer::IRenderer*) {});

    WindowConfig config;
    config.title = "Hit Animation Test";
    config.width = 1280;
    config.height = 720;
    config.resizable = false;
    config.fullscreen = false;
    config.targetFramerate = 60;

    if (!renderer->CreateWindow(config)) {
        std::cerr << "ERROR: Failed to create window" << std::endl;
        return 1;
    }

    // Initialize animation module
    auto animationModule = std::make_unique<AnimationModule>();

    // Load hit texture
    TextureId hitTexture = renderer->LoadTexture("assets/SFX/hit.png");
    if (hitTexture == INVALID_TEXTURE_ID) {
        hitTexture = renderer->LoadTexture("../assets/SFX/hit.png");
    }

    if (hitTexture == INVALID_TEXTURE_ID) {
        std::cerr << "ERROR: Failed to load hit texture" << std::endl;
        return 1;
    }

    std::cout << "✓ Hit texture loaded (ID: " << hitTexture << ")" << std::endl;

    // Create sprite
    Vector2 textureSize = renderer->GetTextureSize(hitTexture);
    SpriteId hitSprite = renderer->CreateSprite(hitTexture, Rectangle{{0.0f, 0.0f}, {textureSize.x, textureSize.y}});
    
    if (hitSprite == INVALID_SPRITE_ID) {
        std::cerr << "ERROR: Failed to create hit sprite" << std::endl;
        return 1;
    }

    std::cout << "✓ Hit sprite created (ID: " << hitSprite << ", texture size: " << textureSize.x << "x" << textureSize.y << ")" << std::endl;

    // Create animation clip
    const int frameCount = 5;
    float singleFrameWidth = textureSize.x / static_cast<float>(frameCount);
    float frameHeight = textureSize.y;

    AnimationClipConfig hitConfig;
    hitConfig.name = "hit_animation";
    hitConfig.texturePath = "assets/SFX/hit.png";
    hitConfig.looping = false;
    hitConfig.playbackSpeed = 1.0f;

    for (int i = 0; i < frameCount; ++i) {
        FrameDef frame;
        frame.region.position.x = static_cast<float>(i) * singleFrameWidth;
        frame.region.position.y = 0.0f;
        frame.region.size.x = singleFrameWidth;
        frame.region.size.y = frameHeight;
        frame.duration = 0.05f;
        hitConfig.frames.push_back(frame);
    }

    AnimationClipId hitClipId = animationModule->CreateClip(hitConfig);
    if (hitClipId == INVALID_CLIP_ID) {
        std::cerr << "ERROR: Failed to create hit animation clip" << std::endl;
        return 1;
    }

    std::cout << "✓ Hit animation clip created (ID: " << hitClipId << ", " << frameCount << " frames, frame size: " << singleFrameWidth << "x" << frameHeight << ")" << std::endl;

    // Get first frame region
    auto firstFrame = animationModule->GetFrameAtTime(hitClipId, 0.0f, false);
    std::cout << "✓ First frame region: pos(" << firstFrame.region.position.x << ", " << firstFrame.region.position.y 
              << ") size(" << firstFrame.region.size.x << ", " << firstFrame.region.size.y << ")" << std::endl;

    // Setup EffectConfig
    EffectConfig effectConfig;
    effectConfig.hitAnimation = hitClipId;
    effectConfig.hitTexture = hitTexture;
    effectConfig.hitSprite = hitSprite;
    effectConfig.hitFirstFrameRegion = firstFrame.region;

    // Create EffectFactory
    auto effectFactory = std::make_unique<EffectFactory>(effectConfig);
    std::cout << "✓ EffectFactory created" << std::endl;

    // Create registry
    Registry registry;

    // Create hit effect at center of screen
    float testX = 640.0f;
    float testY = 360.0f;
    Entity hitEntity = effectFactory->CreateHitEffect(registry, testX, testY);

    std::cout << "✓ Hit effect entity created (ID: " << hitEntity << ")" << std::endl;

    // Verify components
    bool hasPosition = registry.HasComponent<Position>(hitEntity);
    bool hasDrawable = registry.HasComponent<Drawable>(hitEntity);
    bool hasSpriteAnimation = registry.HasComponent<SpriteAnimation>(hitEntity);
    bool hasAnimatedSprite = registry.HasComponent<AnimatedSprite>(hitEntity);
    bool hasVisualEffect = registry.HasComponent<VisualEffect>(hitEntity);

    std::cout << "\nComponent check:" << std::endl;
    std::cout << "  Position: " << (hasPosition ? "✓" : "✗") << std::endl;
    std::cout << "  Drawable: " << (hasDrawable ? "✓" : "✗") << std::endl;
    std::cout << "  SpriteAnimation: " << (hasSpriteAnimation ? "✓" : "✗") << std::endl;
    std::cout << "  AnimatedSprite: " << (hasAnimatedSprite ? "✓" : "✗") << std::endl;
    std::cout << "  VisualEffect: " << (hasVisualEffect ? "✓" : "✗") << std::endl;

    if (hasPosition) {
        const auto& pos = registry.GetComponent<Position>(hitEntity);
        std::cout << "  Position: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }

    if (hasDrawable) {
        const auto& drawable = registry.GetComponent<Drawable>(hitEntity);
        std::cout << "  Drawable: spriteId=" << drawable.spriteId << ", layer=" << drawable.layer 
                  << ", scale=(" << drawable.scale.x << ", " << drawable.scale.y << ")" << std::endl;
    }

    if (hasSpriteAnimation) {
        const auto& anim = registry.GetComponent<SpriteAnimation>(hitEntity);
        std::cout << "  SpriteAnimation: clipId=" << anim.clipId << ", looping=" << anim.looping 
                  << ", destroyOnComplete=" << anim.destroyOnComplete << ", currentFrame=" << anim.currentFrameIndex << std::endl;
        std::cout << "  Current region: pos(" << anim.currentRegion.position.x << ", " << anim.currentRegion.position.y 
                  << ") size(" << anim.currentRegion.size.x << ", " << anim.currentRegion.size.y << ")" << std::endl;
    }

    if (hasAnimatedSprite) {
        const auto& animSprite = registry.GetComponent<AnimatedSprite>(hitEntity);
        std::cout << "  AnimatedSprite: needsUpdate=" << animSprite.needsUpdate << std::endl;
    }

    // Create animation and rendering systems
    AnimationSystem animationSystem(animationModule.get());
    RenderingSystem renderingSystem(renderer.get());

    std::cout << "\n=== Rendering animation (press ESC to exit) ===" << std::endl;
    std::cout << "You should see a hit animation at the center of the screen" << std::endl;

    auto startTime = std::chrono::steady_clock::now();
    int loopFrame = 0;
    bool entityDestroyed = false;

    while (renderer->IsWindowOpen()) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        float dt = std::min(1.0f / 60.0f, static_cast<float>(elapsed) / 1000.0f);
        startTime = currentTime;

        // Handle window events
        renderer->Update(dt);
        if (renderer->IsKeyPressed(Key::Escape)) {
            break;
        }

        // Update animation
        animationSystem.Update(registry, dt);

        // Check if entity still exists
        if (!entityDestroyed && !registry.IsEntityAlive(hitEntity)) {
            std::cout << "  Hit animation completed and entity destroyed (loop frame " << loopFrame << ")" << std::endl;
            entityDestroyed = true;
        }

        // Render
        renderer->BeginFrame();
        renderer->Clear({0.1f, 0.1f, 0.1f, 1.0f});
        renderingSystem.Update(registry, dt);
        renderer->EndFrame();

        loopFrame++;
        
        // Limit to 60fps
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (!entityDestroyed && registry.IsEntityAlive(hitEntity)) {
        std::cout << "  Entity still alive after closing window" << std::endl;
    }

    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}
