cmake_minimum_required(VERSION 3.29)
project(qt_random_picker LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_executable(random_picker
    WIN32
    main.cpp
    auto_resize_label.h
    auto_resize_label.cpp
    config_dialogue.h
    config_dialogue.cpp
    history_list_model.h
    history_list_model.cpp
    main_window.h
    main_window.cpp
    main_window_config.h
    main_window_config.cpp
)

target_link_libraries(random_picker PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
