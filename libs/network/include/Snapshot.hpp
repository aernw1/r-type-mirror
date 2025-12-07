/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** Snapshot system for network synchronization
*/

#ifndef RTYPE_SNAPSHOT_HPP
#define RTYPE_SNAPSHOT_HPP

#include "ECS/Registry.hpp"
#include "ECS/Entity.hpp"
#include "ComponentData.hpp"
#include "Serialization/Serializer.hpp"
#include <vector>
#include <functional>

namespace RType {
namespace Network {

struct Snapshot {
    size_t tick;
    bool wasAck;
    std::vector<ECS::ComponentData> data;

    Snapshot();
    Snapshot(size_t tick, ECS::Registry const& registry);
};

using CanSendFunc = std::function<bool(ECS::Entity, uint8_t)>;

void diffSnapshots(
    Engine::Serializer& diff,
    Snapshot const& previous,
    Snapshot const& current,
    CanSendFunc const& canSend
);

std::vector<std::byte> diffSnapshots(
    Snapshot const& previous,
    Snapshot const& current
);

} // namespace Network
} // namespace RType

#endif /* !RTYPE_SNAPSHOT_HPP */
