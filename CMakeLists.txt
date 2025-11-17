
cmake_minimum_required(VERSION 3.16)
project(InstraX LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/disasm.cpp
    src/ELFLoader.cpp
)

# Create executable
add_executable(instrax ${SOURCES})

# Include directories
# - Your project headers
# - ELFIO headers
target_include_directories(instrax PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ELFIO
)
