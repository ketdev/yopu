#pragma once
#include <iostream>
#include <entt/entt.hpp>

#include "../engine/game.hpp"
#include "../engine/loader.hpp"

class BoardGame : public Game {
   public:
    BoardGame() {}
    virtual ~BoardGame() {}
    virtual void init(SDL::Renderer& renderer) {
        _puyo = Loader::loadTexture(renderer, "src/assets/classic_puyos.png");
    }
    virtual void input(SDL_Scancode) {}
    virtual bool logic() { 
        // std::cout << "Hi!" << std::endl;
        return true; 
    }
    virtual void render(SDL::Renderer& renderer, int frame) {
        SDL_RenderCopy(renderer.get(), _puyo.get(), NULL, NULL);
    }

   private:
    SDL::Texture _puyo;
    entt::registry _reg;
};