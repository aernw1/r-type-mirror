#include "Menu/MainMenu.hpp"
#include "ECS/Components/TextLabel.hpp"
#include "ECS/Components/Clickable.hpp"
#include "ECS/Component.hpp"
#include <iostream>

namespace RType {
    namespace Client {

        enum MenuAction {
            Action_None = 0,
            Action_Play,
            Action_Exit
        };

        MainMenu::MainMenu(GameStateMachine& machine, GameContext& context)
            : m_machine(machine), m_context(context) {
            
            m_renderingSystem = std::make_unique<ECS::RenderingSystem>(context.renderer.get());
            m_textRenderingSystem = std::make_unique<ECS::TextRenderingSystem>(context.renderer.get());
            m_menuSystem = std::make_unique<ECS::MenuSystem>(context.renderer.get());
            
            m_menuSystem->SetActionCallback([this](int actionId) {
                switch (actionId) {
                    case Action_Play:
                        std::cout << "Play Action Triggered!" << std::endl;
                        m_context.serverIp = "127.0.0.1";
                        m_context.serverPort = 4242;
                        m_context.playerName = "Player1";
                        break;
                    case Action_Exit:
                         exit(0);
                        break;
                }
            });
        }

        void MainMenu::Init() {
            auto renderer = m_context.renderer;
            auto registry = m_context.registry;

            m_fontId = renderer->LoadFont("assets/fonts/PressStart2P-Regular.ttf", 24);
            if (m_fontId == 0) m_fontId = renderer->LoadFont("../assets/fonts/PressStart2P-Regular.ttf", 24);
            
            m_bgTextureId = renderer->LoadTexture("assets/backgrounds/1.jpg");
            if (m_bgTextureId == 0) m_bgTextureId = renderer->LoadTexture("../assets/backgrounds/1.jpg");

            if (m_bgTextureId != 0) {
                ECS::Entity bgEntity = registry->CreateEntity();
                m_entities.push_back(bgEntity);
                
                registry->AddComponent(bgEntity, ECS::Position{0.0f, 0.0f});
                
                m_bgSpriteId = renderer->CreateSprite(m_bgTextureId, {});
                ECS::Drawable drawable(m_bgSpriteId, -1);
                
                Renderer::Vector2 texSize = renderer->GetTextureSize(m_bgTextureId);
                if (texSize.x > 0 && texSize.y > 0) {
                    drawable.scale = {1280.0f / texSize.x, 720.0f / texSize.y};
                }

                registry->AddComponent(bgEntity, std::move(drawable));
            }

            // 3. Create Title
            ECS::Entity titleEntity = registry->CreateEntity();
            m_entities.push_back(titleEntity);
            
            registry->AddComponent(titleEntity, ECS::Position{1280.0f / 2.0f - 150.0f, 100.0f});
            ECS::TextLabel titleLabel("R-TYPE", m_fontId, 60);
            titleLabel.color = {0.0f, 1.0f, 1.0f, 1.0f};
            registry->AddComponent(titleEntity, std::move(titleLabel));

            // 4. Create Play Button
            ECS::Entity playBtn = registry->CreateEntity();
            m_entities.push_back(playBtn);
            
            float btnX = 1280.0f / 2.0f - 100.0f; // 200px width button, so start at center - 100
            float btnY = 300.0f;
            registry->AddComponent(playBtn, ECS::Position{btnX, btnY});
            
            ECS::TextLabel playLabel("PLAY", m_fontId, 30);
            playLabel.color = {1.0f, 1.0f, 1.0f, 1.0f};
            playLabel.offsetX = 50.0f;
            playLabel.offsetY = 10.0f;
            registry->AddComponent(playBtn, std::move(playLabel));
            
            registry->AddComponent(playBtn, ECS::Clickable(200.0f, 50.0f, Action_Play));
            
            registry->AddComponent(playBtn, ECS::Drawable(Renderer::INVALID_SPRITE_ID));

            ECS::Entity exitBtn = registry->CreateEntity();
            m_entities.push_back(exitBtn);
            
            float exitY = 400.0f;
            registry->AddComponent(exitBtn, ECS::Position{btnX, exitY});
            
            ECS::TextLabel exitLabel("EXIT", m_fontId, 30);
            exitLabel.color = {1.0f, 1.0f, 1.0f, 1.0f};
            exitLabel.offsetX = 50.0f;
            exitLabel.offsetY = 10.0f;
            registry->AddComponent(exitBtn, std::move(exitLabel));
            
            registry->AddComponent(exitBtn, ECS::Clickable(200.0f, 50.0f, Action_Exit));
            registry->AddComponent(exitBtn, ECS::Drawable(Renderer::INVALID_SPRITE_ID));
        }

        void MainMenu::Cleanup() {
            auto registry = m_context.registry;
            for (ECS::Entity entity : m_entities) {
                registry->DestroyEntity(entity);
            }
            m_entities.clear();
        }

        void MainMenu::HandleInput() {
        }

        void MainMenu::Update(float dt) {
            m_menuSystem->Update(*m_context.registry, dt);
        }

        void MainMenu::Draw() {
            m_renderingSystem->Update(*m_context.registry, 0.0f);
            m_textRenderingSystem->Update(*m_context.registry, 0.0f);
        }

    }
}
