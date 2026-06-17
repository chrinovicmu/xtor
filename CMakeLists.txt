
cmake_minimum_required(VERSION 3.16)
project(X2R LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/disasm.cpp
    src/ELFLoader.cpp
    src/IRLift.cpp
)

add_executable(instrax ${SOURCES})

target_include_directories(instrax PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/ELFIO    # if you vendor ELFIO here
)

target_link_libraries(instrax PRIVATE capstone)

find_package(Zydis REQUIRED)
target_link_libraries(xtor PRIVATE Zydis::Zydis)

