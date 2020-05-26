#pragma once
#include <entt/entt.hpp>
#include <SDL2/SDL_scancode.h>

#include "../engine/igame.hpp"
#include "../engine/loader.hpp"
#include "settings.hpp"
#include "player.hpp"
#include "puyo.hpp"

class Game : public IGame {
public:
    Game() {}
    virtual ~Game() {}
    virtual void init(SDL::Renderer& renderer);
    virtual void input(SDL_Scancode sc, bool isDown);
    virtual bool logic();
    virtual void render(SDL::Renderer& renderer, int frame);

private: // shared memory
    Settings _settings;
    entt::registry _reg;

private: // resources
    SDL::Texture _tex;
    struct {
        SDL::MixMusic move;
        SDL::MixMusic rotate;
        SDL::MixMusic drop;
    } _sound;

private: // factories
    entt::entity makePlayer(int index);
    entt::entity makePuyo(entt::entity parent, puyo::Type type, uint8_t x, uint8_t y);

private:  // render systems
    void applyTranslationAnimation();
    void applyRotationAnimation();
    void drawPuyos(SDL::Renderer& renderer);

    void renderSprite(SDL::Renderer& renderer);

private:  // update systems
    void updateInput();

    void control();
    void freefall();

};