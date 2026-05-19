add_executable(fbm_water
    src/main.cpp
    resources/shaders/water.fs
    resources/shaders/water.vs
    )


target_include_directories(fbm_water PRIVATE
    include
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
    ${CMAKE_SOURCE_DIR}/third_party/glfw/include
    ${CMAKE_SOURCE_DIR}/third_party/stb/include
)

target_link_libraries(fbm_water PRIVATE
    glfw
    glad
    opengl32
)


set_target_properties(fbm_water PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
)
