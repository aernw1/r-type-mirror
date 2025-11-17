#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <stdexcept>
#include <typeindex>
#include <sstream>
#include <string>

namespace RType {

    namespace ECS {

        class IComponentPool {
        public:
            virtual ~IComponentPool() = default;
            virtual bool Has(Entity entity) const = 0;
            virtual void Remove(Entity entity) = 0;
        };

        template <typename T> class ComponentPool : public IComponentPool {
        public:
            T& Add(Entity entity, T&& component = T{});
            T& Get(Entity entity);
            const T& Get(Entity entity) const;
            bool Has(Entity entity) const override;
            void Remove(Entity entity) override;
            std::vector<Entity> GetEntities() const;
        private:
            std::unordered_map<Entity, T> m_components;
        };

        class Registry {
        public:
            Registry();
            ~Registry() = default;

            Entity CreateEntity();
            void DestroyEntity(Entity entity);
            bool IsEntityAlive(Entity entity) const;

            template <typename T> T& AddComponent(Entity entity, T&& component = T{});

            template <typename T> T& GetComponent(Entity entity);

            template <typename T> const T& GetComponent(Entity entity) const;

            template <typename T> bool HasComponent(Entity entity) const;

            template <typename T> void RemoveComponent(Entity entity);

            template <typename T> std::vector<Entity> GetEntitiesWithComponent() const;
            size_t GetEntityCount() const { return m_entityCount; }
        private:
            template <typename T> ComponentPool<T>* GetOrCreatePool();
            template <typename T> ComponentPool<T>* GetPool();
            template <typename T> const ComponentPool<T>* GetPool() const;

            Entity m_nextEntityID;
            size_t m_entityCount;
            std::unordered_set<Entity> m_aliveEntities;
            std::unordered_map<ComponentID, std::unique_ptr<IComponentPool>> m_componentPools;
        };

        template <typename T> T& ComponentPool<T>::Add(Entity entity, T&& component) {
            m_components[entity] = std::move(component);
            return m_components[entity];
        }

        template <typename T> T& ComponentPool<T>::Get(Entity entity) {
            auto it = m_components.find(entity);
            if (it == m_components.end()) {
                std::ostringstream oss;
                oss << "Component '" << typeid(T).name() << "' not found for entity " << entity;
                throw std::runtime_error(oss.str());
            }
            return it->second;
        }

        template <typename T> const T& ComponentPool<T>::Get(Entity entity) const {
            auto it = m_components.find(entity);
            if (it == m_components.end()) {
                std::ostringstream oss;
                oss << "Component '" << typeid(T).name() << "' not found for entity " << entity;
                throw std::runtime_error(oss.str());
            }
            return it->second;
        }

        template <typename T> bool ComponentPool<T>::Has(Entity entity) const {
            return m_components.find(entity) != m_components.end();
        }

        template <typename T> void ComponentPool<T>::Remove(Entity entity) {
            auto it = m_components.find(entity);
            if (it == m_components.end()) {
                std::ostringstream oss;
                oss << "Component '" << typeid(T).name() << "' not found for entity " << entity;
                throw std::runtime_error(oss.str());
            }
            m_components.erase(it);
        }

        template <typename T> std::vector<Entity> ComponentPool<T>::GetEntities() const {
            std::vector<Entity> entities;
            entities.reserve(m_components.size());
            for (const auto& [entity, _] : m_components) {
                entities.push_back(entity);
            }
            return entities;
        }

        template <typename T> T& Registry::AddComponent(Entity entity, T&& component) {
            if (entity == NULL_ENTITY) {
                throw std::invalid_argument("Cannot add component to NULL_ENTITY");
            }
            if (!IsEntityAlive(entity)) {
                std::ostringstream oss;
                oss << "Cannot add component '" << typeid(T).name() << "' to non-existent entity "
                    << entity;
                throw std::runtime_error(oss.str());
            }
            ComponentPool<T>* pool = GetOrCreatePool<T>();
            return pool->Add(entity, std::forward<T>(component));
        }

        template <typename T> T& Registry::GetComponent(Entity entity) {
            ComponentPool<T>* pool = GetPool<T>();
            if (!pool) {
                std::ostringstream oss;
                oss << "Component type '" << typeid(T).name() << "' not registered";
                throw std::runtime_error(oss.str());
            }
            return pool->Get(entity);
        }

        template <typename T> const T& Registry::GetComponent(Entity entity) const {
            const ComponentPool<T>* pool = GetPool<T>();
            if (!pool) {
                std::ostringstream oss;
                oss << "Component type '" << typeid(T).name() << "' not registered";
                throw std::runtime_error(oss.str());
            }
            return pool->Get(entity);
        }

        template <typename T> bool Registry::HasComponent(Entity entity) const {
            const ComponentPool<T>* pool = GetPool<T>();
            if (!pool) {
                return false;
            }
            return pool->Has(entity);
        }

        template <typename T> void Registry::RemoveComponent(Entity entity) {
            ComponentPool<T>* pool = GetPool<T>();
            if (pool) {
                pool->Remove(entity);
            }
        }

        template <typename T> std::vector<Entity> Registry::GetEntitiesWithComponent() const {
            const ComponentPool<T>* pool = GetPool<T>();
            if (!pool) {
                return std::vector<Entity>();
            }
            return pool->GetEntities();
        }

        template <typename T> ComponentPool<T>* Registry::GetOrCreatePool() {
            ComponentID typeID = std::type_index(typeid(T));
            auto it = m_componentPools.find(typeID);
            if (it == m_componentPools.end()) {
                auto pool = std::make_unique<ComponentPool<T>>();
                ComponentPool<T>* poolPtr = pool.get();
                m_componentPools[typeID] = std::move(pool);
                return poolPtr;
            }
            return static_cast<ComponentPool<T>*>(it->second.get());
        }

        template <typename T> ComponentPool<T>* Registry::GetPool() {
            ComponentID typeID = std::type_index(typeid(T));
            auto it = m_componentPools.find(typeID);
            if (it == m_componentPools.end()) {
                return nullptr;
            }
            return static_cast<ComponentPool<T>*>(it->second.get());
        }

        template <typename T> const ComponentPool<T>* Registry::GetPool() const {
            ComponentID typeID = std::type_index(typeid(T));
            auto it = m_componentPools.find(typeID);
            if (it == m_componentPools.end()) {
                return nullptr;
            }
            return static_cast<const ComponentPool<T>*>(it->second.get());
        }

    }

}