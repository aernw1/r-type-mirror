#include "ECS/Registry.hpp"

namespace RType {

    namespace ECS {

        Registry::Registry()
            : m_nextEntityID(1), m_entityCount(0) {}

        Entity Registry::CreateEntity() {
            Entity newEntity = m_nextEntityID++;
            m_aliveEntities.insert(newEntity);
            m_entityCount++;
            return newEntity;
        }

        void Registry::DestroyEntity(Entity entity) {
            if (entity == NULL_ENTITY) {
                return;
            }
            if (!IsEntityAlive(entity)) {
                return;
            }

            for (auto& [componentID, pool] : m_componentPools) {
                if (pool->Has(entity)) {
                    pool->Remove(entity);
                }
            }
            m_aliveEntities.erase(entity);
            m_entityCount--;
        }

        bool Registry::IsEntityAlive(Entity entity) const {
            return m_aliveEntities.find(entity) != m_aliveEntities.end();
        }

        std::vector<ComponentData> Registry::CollectData() const {
            std::vector<ComponentData> allData;
            for (const auto& [typeId, pool] : m_componentPools) {
                auto it = m_componentIds.find(typeId);
                if (it != m_componentIds.end()) {
                    size_t componentId = it->second;
                    auto poolData = pool->CollectData(componentId);
                    allData.insert(allData.end(), poolData.begin(), poolData.end());
                }
            }
            return allData;
        }

        void Registry::ApplyData(Entity entity, size_t componentId, Engine::Deserializer& buffer) {
            for (const auto& [typeId, id] : m_componentIds) {
                if (id == componentId) {
                    auto it = m_componentPools.find(typeId);
                    if (it != m_componentPools.end()) {
                        it->second->ApplyData(entity, buffer);
                        return;
                    }
                }
            }
        }

    }

}
