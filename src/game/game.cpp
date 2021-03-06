#include "game.hpp"

#include <iostream>
#include <vector>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <cmath>

#include "entity.h"

// Get our systems
#include "puyo\puyo.h"
#include "player\board.h"

#include "player\input.h"
#include "player\spawn.h"
#include "puyo\control.h"
#include "player\freefall.h"
#include "player\resolve.h"

#include "puyo\animate.h"
#include "media\sound.h"
#include "media\sprite.h"

// -- Game Implementation --

void Game::init(SDL::Renderer& renderer) {
    // Load resources
    _tex = Loader::loadTexture(renderer, ASSET_PATH_TEXTURE);

    // Create our initial entities
    makeBackground();
    makePlayer(0);
}

void Game::input(SDL_Scancode sc, bool isDown) {
    // Forward input events to our input components
    auto view = _reg.view<player::Input>();
    for (auto& player : view) {
        auto& input = view.get<player::Input>(player);
        auto& controller = _settings.controllers[input.controllerIndex];
        for (size_t i = 0; i < player::InputKey::_Count; i++) {
            if (sc == controller[i]) {
                input.keys[i].isDown = isDown;
            }
        }
    }
}

bool Game::logic() {
    /*TEMP*/ //std::cout << "-- Logic --------------------------------------------" << std::endl;

    player::updateInput(_reg);
    player::spawn(_reg);
    puyo::control(_reg);
    player::freefall(_reg);
    player::resolve(_reg);
    
    return true;
}

void Game::render(SDL::Renderer& renderer, int frame) {
    /*TEMP*/ //std::cout << "-- Render: (" << frame << ")" << std::endl;

    media::soundPlayer(_reg);
    puyo::gravity(_reg);

    media::spriteRender(_reg, renderer);

    applyTranslationAnimation();
    applyRotationAnimation();
    drawPuyos(renderer);
}


// -- Factories --

entt::entity Game::makeBackground() {
    auto bg = _reg.create();    
    _reg.emplace<media::Texture>(bg, media::Texture::Background);
    return bg;
}

entt::entity Game::makePlayer(int index) {
    // Shared puyo pool
    static std::shared_ptr<std::vector<puyo::Color>> pool(new std::vector<puyo::Color>);
    static std::shared_ptr<std::mt19937> randgen(new std::mt19937(std::random_device{}()));

    auto player = _reg.create();
    _reg.emplace<player::Input>(player, index);     // Control input
    _reg.emplace<player::Board>(player);            // Game board
    _reg.emplace<player::Score>(player);            // Score
    auto& spawner = _reg.emplace_or_replace<player::Spawner>(player);
    spawner.randgen = randgen;
    spawner.pool = pool;
    spawner.colorCount = _settings.colorCount;
    _reg.emplace<player::Idle>(player);             // Initial game status
    return player;
}

// -- Render Systems --

void Game::applyTranslationAnimation() {
    // Apply translation animation    
    auto view = _reg.view<puyo::RenderPosition, puyo::TranslateAnimation>();
    for (auto& e : view) {
        auto& pos = view.get<puyo::RenderPosition>(e);
        auto& anim = view.get<puyo::TranslateAnimation>(e);

        // calculate step delta
        auto stepX = anim.dx / anim.frames;
        auto stepY = anim.dy / anim.frames;

        // linear interpolation
        pos.x += stepX;
        pos.y += stepY;

        // update frame counter and already moved steps
        anim.dx -= stepX;
        anim.dy -= stepY;
        anim.frames--;

        // stop when reached end
        if (anim.frames == 0) {
            _reg.remove<puyo::TranslateAnimation>(e);
        }
    }    
}

void Game::applyRotationAnimation() {
    // Apply translation animation    
    auto view = _reg.view<puyo::RenderPosition, puyo::RotateAnimation>();
    for (auto& e : view) {
        auto& pos = view.get<puyo::RenderPosition>(e);
        auto& anim = view.get<puyo::RotateAnimation>(e);

        auto dot = anim.dstDx * anim.srcDx  + anim.dstDy * anim.srcDy;
        auto cross = anim.dstDy * anim.srcDx - anim.dstDx * anim.srcDy;
        auto mag = glm::sqrt(anim.srcDx * anim.srcDx + anim.srcDy * anim.srcDy) 
            * glm::sqrt(anim.dstDx * anim.dstDx + anim.dstDy * anim.dstDy);

        // calculate step delta
        auto c = (dot / mag);
        auto s = (cross / mag);
        auto angle = atan2(s, c);
        auto step = angle / anim.frames;

        // Update src
        s = sin(step);
        c = cos(step);
        auto x = c * anim.srcDx - s * anim.srcDy;
        auto y = s * anim.srcDx + c * anim.srcDy;

        // compute movement delta
        anim.frames--;
        int dx = x - anim.srcDx;
        int dy = y - anim.srcDy;

        // set to dest to remove rounding errors at end
        if (anim.frames == 0) {
            dx = anim.dstDx - anim.srcDx;
            dy = anim.dstDy - anim.srcDy;
        }

        // update position
        pos.x += dx;
        pos.y += dy;
        anim.srcDx += dx;
        anim.srcDy += dy;

        if (anim.frames == 0) {
            _reg.remove<puyo::RotateAnimation>(e);
        }
    }
}

void Game::drawPuyos(SDL::Renderer& renderer) {    
    // Render puyo to screen
    auto view = _reg.view<puyo::Color, puyo::RenderPosition>();
    for (auto& e : view) {
        auto& type = view.get<puyo::Color>(e);
        auto& pos = view.get<puyo::RenderPosition>(e);

        // Axis blinking animation
        auto blink = _reg.has<puyo::BlinkingAnimation>(e);
        if (blink) {
            auto& axis = _reg.get<puyo::BlinkingAnimation>(e);
            axis.counter++;
            // 10 frames on, 10 off
            if ((axis.counter / BLINKING_FRAMES) % 2 == 0) {
                blink = false;
            }
        }

        int ix = {}, iy = {};
        switch (type) {
        case puyo::Color::Red:
            ix = 0, iy = 0;
            if (blink) { iy = 9; }
            break;
        case puyo::Color::Green:
            ix = 0, iy = 1;
            if (blink) { ix = 1, iy = 9; }
            break;
        case puyo::Color::Blue:
            ix = 0, iy = 2;
            if (blink) { ix = 2, iy = 9; }
            break;
        case puyo::Color::Yellow:
            ix = 0, iy = 3;
            if (blink) { ix = 3, iy = 9; }
            break;
        case puyo::Color::Violet:
            ix = 0, iy = 4;
            if (blink) { ix = 4, iy = 9; }
            break;
        case puyo::Color::Garbage:
            ix = 6, iy = 12;
            break;
        default:
            break;
        }

        SDL_Rect src{ ix * puyo::TILE_SIZE, iy * puyo::TILE_SIZE, puyo::TILE_SIZE, puyo::TILE_SIZE };
        SDL_Rect dst{ pos.x, pos.y, src.w, src.h };
        SDL_RenderCopy(renderer.get(), _tex.get(), &src, &dst);
    }

}
