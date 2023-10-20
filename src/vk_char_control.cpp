#include <SDL2/SDL.h>
#include <vk_char_control.h>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <vk_engine.h>
#include <fstream>
#include <iostream>

void Movement::move(RenderObject *objectMove, SDL_Event *e, PlayerCamera *cam)
{
    // cam->process_input_event(e);
    float burst = 100;

    if (e->key.keysym.sym == SDLK_w)
    {
        glm::vec3 velocity = {0, 0, 0.01f};
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
        }
    if (e->key.keysym.sym == SDLK_s)
    {
        glm::vec3 velocity = {0, 0, -0.01f};
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
    }

    if (e->key.keysym.sym == SDLK_a)
    {
        glm::vec3 velocity = {0.01f, 0, 0};
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
    }
    if (e->key.keysym.sym == SDLK_d)
    {
        glm::vec3 velocity = {-0.01f, 0, 0};
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
    }
    // if (e->key.keysym.sym == SDLK_d)
    // {
    //     cam->yaw += 0.001f;
    // }
    // if (e->key.keysym.sym == SDLK_a)
    // {
    //     cam->yaw += -0.001f;
    // }
}