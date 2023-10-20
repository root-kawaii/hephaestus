
# Add source to this project's executable.
add_executable(hephaestus
    main.cpp
    vk_engine.cpp
    vk_engine.h
    vk_types.h
    vk_initializers.cpp
    vk_initializers.h
    vk_mesh.h
    vk_mesh.cpp
    vk_textures.h
    vk_textures.cpp
    player_camera.h
    player_camera.cpp
    vk_char_control.cpp
    vk_char_control.h
    )


set_property(TARGET hephaestus PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:hephaestus>")

target_include_directories(hephaestus PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(hephaestus vkbootstrap vma glm tinyobjloader imgui stb_image)

target_link_libraries(hephaestus Vulkan::Vulkan sdl2)

add_dependencies(hephaestus Shaders)
