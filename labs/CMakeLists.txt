# Cvičení: každý labs/*.cpp je samostatný program (01lab.cpp -> target "01lab")
# Sdílené hlavičky patří do labs/common/

file(GLOB LAB_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(src ${LAB_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})

    set_target_properties(${name} PROPERTIES
        DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    target_link_libraries(${name} PRIVATE
        GLEW::GLEW
        glfw
        glm::glm
        nlohmann_json::nlohmann_json
        ${OpenCV_LIBS}
        imgui::imgui
    )

    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common
        ${OpenCV_INCLUDE_DIRS}
    )
endforeach()
