project(@GAME_NAME@)

cmake_minimum_required(VERSION 3.1)

# Include and build the engine.
add_game_library(@GAME_NAME@ main.c)
