add_library(rtype_asio_network SHARED
    src/AsioNetworkModule.cpp
    src/AsioNetworkModuleExports.cpp
)

target_include_directories(rtype_asio_network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/engine/include>
)

target_link_libraries(rtype_asio_network PUBLIC
    asio
    Threads::Threads
    rtype_core
)

if(WIN32)
    target_compile_definitions(rtype_asio_network PUBLIC
        _WIN32_WINNT=0x0601
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        _CRT_SECURE_NO_WARNINGS
    )
endif()

add_library(rtype_network
    src/TcpSocket.cpp
    src/TcpServer.cpp
    src/Serializer.cpp
    src/Deserializer.cpp
    src/LobbyServer.cpp
    src/LobbyClient.cpp
    src/RoomManager.cpp
    src/RoomClient.cpp
    src/GameServer.cpp
    src/GameClient.cpp
    src/NetworkTcpSocket.cpp
)

target_include_directories(rtype_network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libs/engine/include>
)

find_package(lz4 CONFIG REQUIRED)

target_link_libraries(rtype_network PUBLIC
    asio
    Threads::Threads
    nlohmann_json::nlohmann_json
    rtype_ecs
    lz4::lz4
)

if(WIN32)
    target_compile_definitions(rtype_network PUBLIC
        _WIN32_WINNT=0x0601      # Windows 7
        NOMINMAX                  # Prevent min/max macro conflicts
        WIN32_LEAN_AND_MEAN       # Exclude rarely-used Windows headers
        _CRT_SECURE_NO_WARNINGS   # Suppress deprecation warnings for standard functions
    )
endif()
