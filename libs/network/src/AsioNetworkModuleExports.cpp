/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** AsioNetworkModule plugin exports (CreateModule/DestroyModule)
*/

#include "AsioNetworkModule.hpp"

extern "C" {
#if defined(_WIN32) || defined(_WIN64)
    #define RTYPE_MODULE_EXPORT __declspec(dllexport)
#else
    #define RTYPE_MODULE_EXPORT __attribute__((visibility("default")))
#endif

    RTYPE_MODULE_EXPORT RType::Core::IModule* CreateModule() {
        return new Network::AsioNetworkModule();
    }

    RTYPE_MODULE_EXPORT void DestroyModule(RType::Core::IModule* module) {
        delete module;
    }
}

