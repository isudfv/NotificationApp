cmake_minimum_required(VERSION 3.16)

project(NotificationApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick HttpServer Widgets Core REQUIRED)
find_package(mailio CONFIG REQUIRED)
#find_package(nlohmann_json CONFIG REQUIRED)
#find_package(range-v3 CONFIG REQUIRED)

add_executable(notification
    main.cpp
    DesktopNotificationManagerCompat.h
    DesktopNotificationManagerCompat.cpp
)

#[[
qt_add_qml_module(appqml
    URI NotificationApp
    VERSION 1.0
    QML_FILES main.qml
)
]]

set_target_properties(notification PROPERTIES
#[[
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
]]
    WIN32_EXECUTABLE $<CONFIG:Release>
)

target_link_libraries(notification
    PRIVATE #[[Qt6::Quick]] Qt6::HttpServer #[[Qt6::Widgets]] Qt6::Core
    mailio
    #        nlohmann_json::nlohmann_json
    #        range-v3::meta range-v3::concepts range-v3::range-v3
)
add_custom_command(TARGET notification POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:notification> $<TARGET_FILE_DIR:notification>
    COMMAND_EXPAND_LISTS
)