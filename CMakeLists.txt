cmake_minimum_required(VERSION 3.16)
project(DnDInitiativeTracker LANGUAGES CXX)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Gui Test REQUIRED)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Gui Test REQUIRED)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Test)
else()
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Test)
endif()

add_library(app_sources
    src/models/Combatant.cpp
    src/models/InitiativeModel.cpp
    src/models/TurnManager.cpp
    src/stores/EncounterStore.cpp
    src/stores/RosterStore.cpp
    src/ui/MainWindow.cpp
    src/undo/UndoCommands.cpp
    src/utils/DiceRoller.cpp
    src/utils/Settings.cpp
)

target_include_directories(app_sources PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(app_sources PUBLIC ${QT_LIBRARIES})

add_executable(dnd_initiative src/main.cpp)

target_link_libraries(dnd_initiative PRIVATE app_sources ${QT_LIBRARIES})

add_executable(testsuite tests/TestTurnManager.cpp)
target_link_libraries(testsuite PRIVATE app_sources ${QT_LIBRARIES})

add_test(NAME unit_tests COMMAND testsuite)

