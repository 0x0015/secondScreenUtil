cmake_minimum_required(VERSION 3.16.0)

project(secondScreenUtil VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
	message(STATUS "Found CCache - using as compiler launcher")
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)
get_target_property(QtCoreLocation Qt6::Core LOCATION)
message(STATUS "Qt6::Core resolved to: ${QtCoreLocation}")

find_package(X11 REQUIRED)
find_package(LayerShellQt REQUIRED)

add_executable(secondScreenUtil
    main.cpp
    mainWindow.cpp
    systemInfo.cpp
    util.cpp
    keyboardWidget.cpp
    keyboard.cpp
)

target_compile_options(secondScreenUtil PRIVATE -Wall)
target_link_libraries(secondScreenUtil PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets X11 LayerShellQt::Interface)

