#pragma once
#include <SDL2/SDL_scancode.h>
#include "sdl_utils.hpp"

class Game {
   public:
    virtual ~Game(){};
    virtual void init(SDL::Renderer& renderer) = 0;
    virtual void input(SDL_Scancode) = 0;
    virtual bool logic() = 0;
    virtual void render(SDL::Renderer& renderer, int frame) = 0;
};