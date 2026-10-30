cmake_minimum_required(VERSION 4.0)

# Version 4.0+ is needed to set working directory
# Make sure to use forward slashes (/) in the path!

project(icp VERSION 2026.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    # OpenCV extracted to C:/OpenCV (see 01lab instructions), other location: -DOpenCV_DIR=...
    find_package(OpenCV CONFIG REQUIRED PATHS "C:/OpenCV/build")
elseif(UNIX)
    find_package(OpenCV CONFIG REQUIRED)
endif()

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

# target properties can be set only AFTER add_executable(...)
set_target_properties(${PROJECT_NAME} PROPERTIES
    DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
