# Engine CMakeLists.txt - Generic Game Engine
#
# This builds the generic engine libraries that can be used by any game.
# Game-specific code belongs in games/<game_name>/

# Core subsystem (Logger, Engine, Module system)
add_subdirectory(src/Core)

# ECS subsystem (split into generic and rtype-specific)
add_subdirectory(src/ECS)

# Renderer plugin (SFML implementation)
add_subdirectory(src/Renderer)

# Audio plugin (SFML implementation)
add_subdirectory(src/Audio)

# Animation system
add_subdirectory(src/Animation)
