#include "VisualLobbyClient.hpp"
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::string addr = "127.0.0.1";
    uint16_t port = 4242;
    std::string name = "Player";

    if (argc > 1) {
        addr = argv[1];
    }
    if (argc > 2) {
        port = std::stoi(argv[2]);
    }
    if (argc > 3) {
        name = argv[3];
    }

    try {
        Client::VisualLobbyClient client(addr, port, name);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
