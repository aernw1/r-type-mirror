add_library(rtype_network
    src/UdpSocket.cpp
    src/Serializer.cpp
    src/Deserializer.cpp
    src/LobbyServer.cpp
    src/LobbyClient.cpp
)

target_include_directories(rtype_network PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(rtype_network PUBLIC asio Threads::Threads)
