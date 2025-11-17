#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <stdexcept>
#include <typeindex>
#include <iostream>

namespace RType {

    namespace ECS {

        class IComponentPool {
            public:
                virtual ~IComponentPool() = default;
                virtual bool Has(Entity entity) const = 0;
                virtual void Remove(Entity entity) = 0;
        };

        template<typename T>
        class ComponentPool : public IComponentPool {
            public:
                T& Add(Entity entity, T&& component = T{});
                T& Get(Entity entity);
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

                template<typename T>
                T& AddComponent(Entity entity, T&& component = T{});

                template<typename T>
                T& GetComponent(Entity entity);

                template<typename T>
                bool HasComponent(Entity entity) const;

                template<typename T>
                void RemoveComponent(Entity entity);

                template<typename T>
                std::vector<Entity> GetEntitiesWithComponent() const;
                size_t GetEntityCount() const { return m_entityCount; }

            private:
                template<typename T>
                ComponentPool<T>* GetOrCreatePool();
                template<typename T>
                ComponentPool<T>* GetPool() const;

                Entity m_nextEntityID;
                size_t m_entityCount;
                std::unordered_set<Entity> m_aliveEntities;
                std::unordered_map<ComponentID, std::unique_ptr<IComponentPool>> m_componentPools;
        };

        template<typename T>
        T& ComponentPool<T>::Add(Entity entity, T&& component) {
            m_components[entity] = std::move(component);
            return m_components[entity];
        }

        template<typename T>
        T& ComponentPool<T>::Get(Entity entity) {
            auto it = m_components.find(entity);
            if (it == m_components.end()) {
                throw std::runtime_error("Component not found for entity");
            }
            return it->second;
        }

        template<typename T>
        bool ComponentPool<T>::Has(Entity entity) const {
            return m_components.find(entity) != m_components.end();
        }

        template<typename T>
        void ComponentPool<T>::Remove(Entity entity) {
            m_components.erase(entity);
        }

        template<typename T>
        std::vector<Entity> ComponentPool<T>::GetEntities() const {
            std::vector<Entity> entities;
            entities.reserve(m_components.size());
            for (const auto& [entity, _] : m_components) {
                entities.push_back(entity);
            }
            return entities;
        }

    }

}