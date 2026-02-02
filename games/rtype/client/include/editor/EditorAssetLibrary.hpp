#pragma once

#include "editor/EditorTypes.hpp"
#include "Renderer/IRenderer.hpp"
#include "Math/Types.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace RType {
    namespace Client {

        struct EditorAssetDefinition {
            std::string id;
            std::string displayName;
            EditorEntityType type = EditorEntityType::OBSTACLE;
            std::string texturePath;
            Math::Vector2 defaultSize{0.0f, 0.0f};
            float defaultScrollSpeed = -50.0f;
            int defaultLayer = 1;
            std::string enemyType;
            std::string powerUpType;
        };

        struct EditorAssetResource {
            EditorAssetDefinition definition;
            Renderer::TextureId textureId = Renderer::INVALID_TEXTURE_ID;
            Renderer::SpriteId spriteId = Renderer::INVALID_SPRITE_ID;
            Math::Vector2 textureSize{128.0f, 128.0f};
        };

        class EditorAssetLibrary {
        public:
            explicit EditorAssetLibrary(Renderer::IRenderer* renderer);

            bool Initialize();

            const EditorAssetResource* GetResource(const std::string& id) const;
            const std::vector<const EditorAssetResource*>& GetResources(EditorEntityType type) const;

        private:
            Renderer::IRenderer* m_renderer;
            std::unordered_map<std::string, EditorAssetResource> m_resourcesById;
            std::unordered_map<int, std::vector<const EditorAssetResource*>> m_resourcesByType;

            std::vector<EditorAssetDefinition> buildDefaultDefinitions() const;
            bool loadResource(EditorAssetResource& resource);
        };

    }
}

