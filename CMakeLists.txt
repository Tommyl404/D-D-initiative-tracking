cmake_minimum_required(VERSION 3.16)
project(DnDInitiativeTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_library(initiative_core
    src/core/Combatant.cpp
    src/core/InitiativeTracker.cpp
    src/storage/EncounterStore.cpp
    src/storage/RosterStore.cpp
    src/storage/SimpleJson.cpp
    src/utils/DiceRoller.cpp
    src/utils/Settings.cpp
)

target_include_directories(initiative_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(dnd_initiative src/main.cpp)

target_link_libraries(dnd_initiative PRIVATE initiative_core)

add_executable(testsuite tests/TestTurnManager.cpp)

target_link_libraries(testsuite PRIVATE initiative_core)

add_test(NAME unit_tests COMMAND testsuite)

