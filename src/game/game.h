#pragma once
#include <entt/entt.hpp>
#include <SDL2/SDL_scancode.h>

#include <game/logic/play.h>

#include "../engine/igame.hpp"
#include "settings.h"

#include "../engine/graphics/texture.h"
#include "../engine/draw/sprite_render.h"

class Game : public IGame {
public:
    Game() {}
    virtual ~Game() {}
    virtual void init();
    virtual void input(SDL_Scancode sc, bool isDown);
    virtual bool logic();
    virtual void render(int frame, int width, int height);

private: // shared memory
    Settings _settings;
    entt::registry _reg;

    play::State _play;

private: // resources
    Texture _texture;
    std::unique_ptr<SpriteRender> _render;

private: // factories
    entt::entity makePlayer(int index);

private:  // render systems
    void applyTranslationAnimation();
    void applyRotationAnimation();
    void drawPuyos();
};