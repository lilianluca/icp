# Prototypy nápadů na semestrální projekt: každý prototypes/*.cpp je samostatný program

find_package(OpenGL REQUIRED)

file(GLOB PROTOTYPE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

foreach(src ${PROTOTYPE_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})

    set_target_properties(${name} PROPERTIES
        DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    target_link_libraries(${name} PRIVATE
        GLEW::GLEW
        glfw
        glm::glm
        OpenGL::GL
    )
endforeach()
