// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once
#include "vk_types.h"
#include <SDL2/SDL_events.h>
#include <glm/glm.hpp>
// #include <vk_engine.h>

struct RenderObject;

struct PlayerCamera
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 inputAxis;

    float pitch{0}; // up-down rotation
    float yaw{0};   // left-right rotation

    bool bSprint = false;
    bool bLocked;

    void process_input_event(SDL_Event *ev);
    void update_camera(float deltaSeconds);

    glm::mat4 get_view_matrix_obj(RenderObject *obj);
    glm::mat4 get_view_matrix();
    glm::mat4 get_projection_matrix(bool bReverse = true);
    glm::mat4 get_rotation_matrix();
    glm::mat4 get_rotation_matrix_obj();
};