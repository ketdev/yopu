#pragma once
#include <entt/entt.hpp>
#include <random>
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

private:  // shared memory
    std::mt19937 _rand;
    Settings _settings;
    entt::registry _reg;

private: // resources
    SDL::Texture _tex;
    struct {
        SDL::MixMusic move;
        SDL::MixMusic rotate;
    } _sound;

private: // factories
    entt::entity makePlayer(int index);
    entt::entity makePuyo(entt::entity parent, puyo::Type type, uint8_t x, uint8_t y);

private:  // systems
    void applyTranslationAnimation();
    void applyRotationAnimation();
    void drawPuyos(SDL::Renderer& renderer);
    void updateInput();
    void spawn();
    void control();
    // void freefall();

};