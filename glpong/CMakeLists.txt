project(glpong)

cmake_minimum_required(VERSION 3.1)

# Include local settings.
include(cmake_user.txt)

# Include the engine.
include(${ENGINE_PATH}/CMakeLists.txt)

# Build the engine.
add_game_library(glpong glpong.c)
