/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** EventBus - Simple publish-subscribe event system for decoupled communication
*/

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <any>

namespace RType {

    namespace Core {


        class EventBus {
        public:
            EventBus() = default;
            ~EventBus() = default;

            // Non-copyable but movable
            EventBus(const EventBus&) = delete;
            EventBus& operator=(const EventBus&) = delete;
            EventBus(EventBus&&) = default;
            EventBus& operator=(EventBus&&) = default;


            template <typename EventT>
            size_t Subscribe(std::function<void(const EventT&)> handler) {
                std::type_index typeId = std::type_index(typeid(EventT));
                
                auto wrapper = [handler](const std::any& event) {
                    handler(std::any_cast<const EventT&>(event));
                };
                
                size_t id = m_nextId++;
                m_handlers[typeId].push_back({id, std::move(wrapper)});
                return id;
            }


            template <typename EventT>
            void Unsubscribe(size_t subscriptionId) {
                std::type_index typeId = std::type_index(typeid(EventT));
                
                auto it = m_handlers.find(typeId);
                if (it != m_handlers.end()) {
                    auto& handlers = it->second;
                    handlers.erase(
                        std::remove_if(handlers.begin(), handlers.end(),
                            [subscriptionId](const HandlerEntry& entry) {
                                return entry.id == subscriptionId;
                            }),
                        handlers.end()
                    );
                }
            }


            template <typename EventT>
            void Publish(const EventT& event) {
                std::type_index typeId = std::type_index(typeid(EventT));
                
                auto it = m_handlers.find(typeId);
                if (it != m_handlers.end()) {
                    for (const auto& entry : it->second) {
                        entry.handler(event);
                    }
                }
            }


            template <typename EventT>
            void ClearSubscribers() {
                std::type_index typeId = std::type_index(typeid(EventT));
                m_handlers.erase(typeId);
            }


            void Clear() {
                m_handlers.clear();
            }


            template <typename EventT>
            size_t GetSubscriberCount() const {
                std::type_index typeId = std::type_index(typeid(EventT));
                
                auto it = m_handlers.find(typeId);
                if (it != m_handlers.end()) {
                    return it->second.size();
                }
                return 0;
            }

        private:
            struct HandlerEntry {
                size_t id;
                std::function<void(const std::any&)> handler;
            };

            std::unordered_map<std::type_index, std::vector<HandlerEntry>> m_handlers;
            size_t m_nextId = 0;
        };



        namespace Events {

            /// Fired when an entity is created
            struct EntityCreated {
                uint32_t entity;
            };

            /// Fired when an entity is destroyed
            struct EntityDestroyed {
                uint32_t entity;
            };

            /// Fired when a collision occurs between two entities
            struct Collision {
                uint32_t entityA;
                uint32_t entityB;
            };

            /// Fired when a scene is loaded
            struct SceneLoaded {
                std::string sceneName;
            };

            /// Fired when a scene is unloaded
            struct SceneUnloaded {
                std::string sceneName;
            };

            /// Fired when input action is triggered
            struct ActionTriggered {
                std::string action;
                bool pressed;  // true = pressed, false = released
            };

        }

    }

}
