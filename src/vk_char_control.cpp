#include <SDL2/SDL.h>
#include <vk_char_control.h>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <vk_engine.h>
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;

void Movement::move(RenderObject *objectMove, SDL_Event *e, PlayerCamera *cam)
{
    // cam->process_input_event(e);
    float burst = 100;

    if (e->key.keysym.sym == SDLK_w)
    {
        glm::vec3 velocity = {1.f, 0, 1.f};
        velocity *= 0.01f * glm::normalize(cam->position - objectMove->position);
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
    }
    if (e->key.keysym.sym == SDLK_s)
    {
        glm::vec3 velocity = {-1.f, 0, -1.f};
        velocity *= 0.01f * glm::normalize(cam->position - objectMove->position);
        objectMove->position += velocity * burst;
        glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        objectMove->transformMatrix = translation * scale;

        cam->position += velocity * burst;
    }

    // if (e->key.keysym.sym == SDLK_a)
    // {
    //     glm::vec3 velocity = {0.01f, 0, 0};
    //     objectMove->position += velocity * burst;
    //     glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
    //     glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
    //     objectMove->transformMatrix = translation * scale;

    //     cam->position += velocity * burst;
    // }
    // if (e->key.keysym.sym == SDLK_d)
    // {
    //     glm::vec3 velocity = {-0.01f, 0, 0};
    //     objectMove->position += velocity * burst;
    //     glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
    //     glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
    //     objectMove->transformMatrix = translation * scale;

    //     cam->position += velocity * burst;
    // }
    if (e->key.keysym.sym == SDLK_d)
    {
        float inc = 20.f;
        cam->yaw += inc;

        // glm::mat4 rot = glm::rotate(glm::mat4{1}, cam->yaw / 1000.f * (1 / 6), {0, -1, 0});
        // objectMove->transformMatrix = rot;

        // float sinv;
        // sinv = 10 * sin(inc);
        // float cosv;
        // cosv = 10 * cos(inc);
        // objectMove->position += glm::vec3{-sinv, 0, -cosv};
        // glm::mat4 translation = glm::translate(glm::mat4{1.0}, objectMove->position);
        // glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        // objectMove->transformMatrix = translation * scale;
    }
    if (e->key.keysym.sym == SDLK_a)
    {
        float inc = 20.f;
        cam->yaw -= inc;

        // glm::mat4 rot = glm::rotate(glm::mat4{1}, cam->yaw / 1000.f, {0, -1, 0});
        // objectMove->transformMatrix = rot;

        // float sinv;
        // sinv = 10 * sin(inc);
        // float cosv;
        // cosv = 10 * cos(inc);
        // objectMove->position += glm::vec3{-sinv, 0, +cosv};
        // glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3{-sinv, 0, +cosv});
        // glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(1, 1, 1));
        // objectMove->transformMatrix = translation * scale;
    }
}