
#pragma once
// #include "vk_engine.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <player_camera.h>

struct RenderObject;
struct PlayerCamera;

struct Movement
{
    void move(RenderObject *obj, SDL_Event *ev, PlayerCamera *camera);
};