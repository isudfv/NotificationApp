cmake_minimum_required(VERSION 3.27)

project(NotificationApp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 COMPONENTS Quick HttpServer Widgets Core REQUIRED)
find_package(Boost COMPONENTS program_options regex url filesystem REQUIRED)
find_package(mailio CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

add_executable(notification
    main.cpp
    DesktopNotificationManagerCompat.h
    DesktopNotificationManagerCompat.cpp
)

add_executable(boost-notification
    boost-server.cpp
    DesktopNotificationManagerCompat.h
    DesktopNotificationManagerCompat.cpp
    utils.h
    utils.cpp
)

set_target_properties(notification PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
)

set_target_properties(boost-notification  PROPERTIES
    WIN32_EXECUTABLE $<CONFIG:Release>
)

target_link_libraries(boost-notification PRIVATE
    Boost::filesystem
    mailio
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)

target_link_libraries(notification
    PRIVATE Qt6::HttpServer Qt6::Core
    mailio
    spdlog::spdlog
)

target_compile_options(notification PRIVATE "$<$<CONFIG:Release>:/Zi>")

target_compile_options(boost-notification
    PRIVATE
    "$<$<CONFIG:Release>:/Zi>"
    "/bigobj"
)

target_link_options(notification PRIVATE
    "$<$<CONFIG:Release>:/DEBUG>"
    "$<$<CONFIG:Release>:/OPT:REF>"
    "$<$<CONFIG:Release>:/OPT:ICF>"
)

target_link_options(boost-notification
    PRIVATE
    "$<$<CONFIG:Release>:/DEBUG>"
    "$<$<CONFIG:Release>:/OPT:REF>"
    "$<$<CONFIG:Release>:/OPT:ICF>"
    "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
    "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
)

add_custom_command(TARGET notification POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:notification> $<TARGET_FILE_DIR:notification>
    COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET boost-notification POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:boost-notification> $<TARGET_FILE_DIR:boost-notification>
    COMMAND_EXPAND_LISTS
)
