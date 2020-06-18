#pragma once
#include <SDL2/SDL_scancode.h>
#include "../engine/utils/sdl_utils.h"

class IGame {
   public:
    virtual ~IGame(){};
    virtual void init() = 0;
    virtual void input(SDL_Scancode sc, bool isDown) = 0;
    virtual bool logic() = 0;
    virtual void render(int frame, int width, int height) = 0;
};