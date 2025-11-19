
cmake_minimum_required(VERSION 3.16)
project(X2R LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/disasm.cpp
    src/ELFLoader.cpp
)

add_executable(instrax ${SOURCES})

target_include_directories(x2r PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ELFIO
)

# Link Capstone
target_link_libraries(instrax PRIVATE capstone)
