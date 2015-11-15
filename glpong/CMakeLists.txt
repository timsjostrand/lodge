project(glpong)

cmake_minimum_required(VERSION 3.1)

# Include and build the engine.
add_game_library(glpong glpong.c)
