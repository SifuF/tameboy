cmake_minimum_required(VERSION 3.28)

project(tameboy VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory("${CMAKE_SOURCE_DIR}/submodules/SFML")

set(SOURCES
        src/main.cpp
        src/Bus.cpp
        src/CPULR35902.cpp
        src/PPU.cpp
        src/Screen.cpp
)

set(HEADERS
        src/Bus.hpp
        src/CPULR35902.hpp
        src/PPU.hpp
        src/Screen.hpp
        src/Utils.hpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
    sfml-graphics
    sfml-system
    sfml-window
    sfml-audio
)
