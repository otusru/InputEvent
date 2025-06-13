cmake_minimum_required(VERSION 3.10)
project(AudioRecorder)

set(CMAKE_CXX_STANDARD 23)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(AudioRecorder
    main.cpp
    tray_icon.cpp
    audio_recorder.cpp
)

target_link_libraries(AudioRecorder Qt6::Core Qt6::Gui Qt6::Widgets)
