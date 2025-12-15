#include "ECS/Registry.hpp"

namespace RType {

    namespace ECS {

        Registry::Registry()
            : m_nextEntityID(1), m_entityCount(0) {}

        Entity Registry::CreateEntity() {
            Entity newEntity;

            if (!m_freeEntityIds.empty()) {
                newEntity = m_freeEntityIds.back();
                m_freeEntityIds.pop_back();
            } else {
                newEntity = m_nextEntityID++;
            }

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
            m_freeEntityIds.push_back(entity);
        }

        bool Registry::IsEntityAlive(Entity entity) const {
            return m_aliveEntities.find(entity) != m_aliveEntities.end();
        }

    }

}
