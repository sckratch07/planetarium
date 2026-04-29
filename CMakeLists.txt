cmake_minimum_required(VERSION 4.0)

project(Planetarium LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui OpenGLWidgets)

qt6_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE HEADERS "include/*.hpp")
file(GLOB_RECURSE SOURCES "src/*.cpp")
qt6_add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} "res.qrc")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

find_package(SFML COMPONENTS Graphics Window System CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Window SFML::Graphics SFML::System)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND Qt6::windeployqt
    ARGS
        --no-translations
        --no-system-d3d-compiler
        $<TARGET_FILE:${PROJECT_NAME}>
)