cmake_minimum_required(VERSION 3.10)

# Set the project name
project(memory)

# Include headers
include_directories(include)

# Add the library as SHARED
add_library(memory SHARED
    src/memory.c
)

# Set C standard
set_target_properties(memory PROPERTIES
    C_STANDARD 17
)

