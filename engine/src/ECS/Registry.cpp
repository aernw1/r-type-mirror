#include "ECS/Registry.hpp"
#include <iostream>

namespace RType {

    namespace ECS {

        Registry::Registry()
            : m_nextEntityID(1), m_entityCount(0) {}

        Entity Registry::CreateEntity() {
            Entity newEntity;

            if (!m_freeEntityIds.empty()) {
                newEntity = m_freeEntityIds.back();
                m_freeEntityIds.pop_back();

#ifdef DEBUG
                // CRITICAL: Verify entity has no components before reuse
                // If this fires, DestroyEntity didn't clean up properly
                for (const auto& [componentID, pool] : m_componentPools) {
                    if (pool->Has(newEntity)) {
                        std::cerr << "[CRITICAL] Entity " << newEntity
                                  << " being reused but still has components!" << std::endl;
                        pool->Remove(newEntity);  // Emergency cleanup
                    }
                }
#endif
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

            // Remove all components from this entity
            for (auto& [componentID, pool] : m_componentPools) {
                if (pool->Has(entity)) {
                    pool->Remove(entity);
                }
            }

#ifdef DEBUG
            // Debug validation: Verify all components were actually removed
            // This helps catch component persistence bugs that cause entity type confusion
            size_t remainingComponents = 0;
            for (const auto& [componentID, pool] : m_componentPools) {
                if (pool->Has(entity)) {
                    remainingComponents++;
                }
            }
            if (remainingComponents > 0) {
                std::cerr << "[DEBUG] WARNING: Entity " << entity
                          << " still has " << remainingComponents
                          << " component(s) after DestroyEntity cleanup!" << std::endl;
            }
#endif

            m_aliveEntities.erase(entity);
            m_entityCount--;
            m_freeEntityIds.push_back(entity);
        }

        bool Registry::IsEntityAlive(Entity entity) const {
            return m_aliveEntities.find(entity) != m_aliveEntities.end();
        }

        void Registry::Clear() {
            for (auto& [id, pool] : m_componentPools) {
                pool->Clear();
            }
            m_aliveEntities.clear();
            m_freeEntityIds.clear();
            m_nextEntityID = 1;
            m_entityCount = 0;
        }

    }

}
