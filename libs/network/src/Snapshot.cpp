/*
** EPITECH PROJECT, 2023
** R-Type
** File description:
** Snapshot implementation
*/

#include "Snapshot.hpp"
#include <algorithm>

namespace RType {
namespace Network {

using EntityNumber = uint32_t;
using ComponentId = uint8_t;
using UpdateType = bool;

Snapshot::Snapshot()
    : tick(0), wasAck(true), data()
{
}

Snapshot::Snapshot(size_t tick, ECS::Registry const& registry)
    : tick(tick), wasAck(false), data(registry.CollectData())
{
}

static void diffAdd(
    Engine::Serializer& diff,
    std::vector<ECS::ComponentData>::const_iterator const& it,
    CanSendFunc const& canSend
)
{
    if (!canSend(it->entity, it->componentId))
        return;

    diff.serializeTrivial(EntityNumber(it->entity));
    diff.serializeTrivial(ComponentId(it->componentId));
    diff.serializeTrivial(UpdateType(0x01));  // Add/Modify
    diff.insert(it->data);
}

static void diffRemove(
    Engine::Serializer& diff,
    std::vector<ECS::ComponentData>::const_iterator const& it
)
{
    diff.serializeTrivial(EntityNumber(it->entity));
    diff.serializeTrivial(ComponentId(it->componentId));
    diff.serializeTrivial(UpdateType(0x00));  // Remove
}

void diffSnapshots(
    Engine::Serializer& diff,
    Snapshot const& previous,
    Snapshot const& current,
    CanSendFunc const& canSend
)
{
    auto previousIt = previous.data.begin();

    for (auto currentIt = current.data.begin(); currentIt != current.data.end(); ++currentIt) {
        if (previousIt == previous.data.end()) {
            diffAdd(diff, currentIt, canSend);
            continue;
        }

        if (previousIt->componentId == currentIt->componentId
            && previousIt->entity == currentIt->entity) {
            if (previousIt->data != currentIt->data) {
                // Modified, send update
                diffAdd(diff, currentIt, canSend);
            }
            ++previousIt;
            continue;
        }

        // Different identifiers, either removal or addition
        for (auto it = previousIt; it != previous.data.end(); ++it) {
            if (it->componentId == currentIt->componentId
                && it->entity == currentIt->entity) {
                // Current exists in previous, removals happened
                for (auto it2 = previousIt; it2 != it; ++it2) {
                    diffRemove(diff, it2);
                }
                previousIt = it + 1;
                break;
            }
        }
        // Current does not exist in previous, it was added
        diffAdd(diff, currentIt, canSend);
    }

    // Remaining components in previous are removals
    for (auto it = previousIt; it != previous.data.end(); ++it) {
        diffRemove(diff, it);
    }
}

std::vector<std::byte> diffSnapshots(
    Snapshot const& previous,
    Snapshot const& current
)
{
    Engine::Serializer diff;
    diffSnapshots(
        diff, previous, current,
        [](ECS::Entity, uint8_t) { return true; }
    );
    return diff.finalize();
}

} // namespace Network
} // namespace RType
