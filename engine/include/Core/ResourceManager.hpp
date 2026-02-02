/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** ResourceManager - Centralized asset loading with caching
*/

#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>
#include <typeindex>

namespace RType {

    namespace Core {


        class ResourceManager {
        public:
            ResourceManager() = default;
            ~ResourceManager() = default;

            // Non-copyable
            ResourceManager(const ResourceManager&) = delete;
            ResourceManager& operator=(const ResourceManager&) = delete;


            template <typename T>
            void RegisterLoader(std::function<std::shared_ptr<T>(const std::string&)> loader) {
                std::type_index typeId = std::type_index(typeid(T));
                m_loaders[typeId] = [loader](const std::string& path) -> std::shared_ptr<void> {
                    return loader(path);
                };
            }


            template <typename T>
            std::shared_ptr<T> Load(const std::string& path) {
                std::string cacheKey = GetCacheKey<T>(path);
                
                // Check cache first
                auto it = m_cache.find(cacheKey);
                if (it != m_cache.end()) {
                    auto locked = it->second.lock();
                    if (locked) {
                        return std::static_pointer_cast<T>(locked);
                    }
                    // Expired, remove from cache
                    m_cache.erase(it);
                }

                // Load using registered loader
                std::type_index typeId = std::type_index(typeid(T));
                auto loaderIt = m_loaders.find(typeId);
                if (loaderIt == m_loaders.end()) {
                    return nullptr;  // No loader registered
                }

                auto resource = loaderIt->second(path);
                if (resource) {
                    m_cache[cacheKey] = resource;
                    return std::static_pointer_cast<T>(resource);
                }
                return nullptr;
            }


            template <typename T>
            void Preload(const std::vector<std::string>& paths) {
                for (const auto& path : paths) {
                    Load<T>(path);
                }
            }


            template <typename T>
            bool IsCached(const std::string& path) const {
                std::string cacheKey = GetCacheKey<T>(path);
                auto it = m_cache.find(cacheKey);
                if (it != m_cache.end()) {
                    return !it->second.expired();
                }
                return false;
            }


            void CleanupExpired() {
                for (auto it = m_cache.begin(); it != m_cache.end(); ) {
                    if (it->second.expired()) {
                        it = m_cache.erase(it);
                    } else {
                        ++it;
                    }
                }
            }


            void Clear() {
                m_cache.clear();
            }


            size_t GetCacheSize() const {
                return m_cache.size();
            }


            size_t GetActiveCacheCount() const {
                size_t count = 0;
                for (const auto& [key, weakPtr] : m_cache) {
                    if (!weakPtr.expired()) {
                        count++;
                    }
                }
                return count;
            }

        private:
            template <typename T>
            std::string GetCacheKey(const std::string& path) const {
                return std::string(typeid(T).name()) + ":" + path;
            }

            std::unordered_map<std::string, std::weak_ptr<void>> m_cache;
            std::unordered_map<std::type_index, std::function<std::shared_ptr<void>(const std::string&)>> m_loaders;
        };

    }

}
