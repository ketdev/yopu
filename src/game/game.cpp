#include "game.hpp"

#include <iostream>
#include <vector>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <cmath>

#include "entity.h"

// -- Game Implementation --

void Game::init(SDL::Renderer& renderer) {
    // Load resources
    _tex = Loader::loadTexture(renderer, ASSET_PATH_TEXTURE);
    _sound.move = Loader::loadMusic(ASSET_PATH_SOUND_MOVE);
    _sound.rotate = Loader::loadMusic(ASSET_PATH_SOUND_ROTATE);
    _sound.drop = Loader::loadMusic(ASSET_PATH_SOUND_DROP);

    // Create player entities
    makePlayer(0);
}
void Game::input(SDL_Scancode sc, bool isDown) {
    auto view = _reg.view<player::Input>();
    for (auto& e : view) {
        auto& input = view.get<player::Input>(e);
        auto& controller = _settings.controllers[input.controllerIndex];
        for (size_t i = 0; i < player::InputKey::_Count; i++) {
            if (sc == controller[i]) {
                input.keys[i].next = isDown;
            }
        }
    }
}
bool Game::logic() {
    std::cout << "-- Logic --------------------------------------------" << std::endl;
    updateInput();

    player::spawn(_reg);

    control();
    freefall();
    return true;
}
void Game::render(SDL::Renderer& renderer, int frame) {
    std::cout << "-- Render: (" << frame << ")" << std::endl;
    
    applyTranslationAnimation();
    applyRotationAnimation();
    drawPuyos(renderer);
}


// -- Factories --

entt::entity Game::makePlayer(int index) {
    // Shared puyo pool
    static std::shared_ptr<std::vector<puyo::Type>> pool(new std::vector<puyo::Type>);
    static std::shared_ptr<std::mt19937> randgen(new std::mt19937(std::random_device{}()));

    auto player = _reg.create();
    _reg.emplace<player::Input>(player, index);     // Control input
    _reg.emplace<player::Board>(player);            // Game board
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
        auto& type = view.get<puyo::Color>(e).type;
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
        case puyo::Type::Red:
            ix = 0, iy = 0;
            if (blink) { iy = 9; }
            break;
        case puyo::Type::Green:
            ix = 0, iy = 1;
            if (blink) { ix = 1, iy = 9; }
            break;
        case puyo::Type::Blue:
            ix = 0, iy = 2;
            if (blink) { ix = 2, iy = 9; }
            break;
        case puyo::Type::Yellow:
            ix = 0, iy = 3;
            if (blink) { ix = 3, iy = 9; }
            break;
        case puyo::Type::Violet:
            ix = 0, iy = 4;
            if (blink) { ix = 4, iy = 9; }
            break;
        case puyo::Type::Garbage:
            ix = 6, iy = 12;
            break;
        default:
            break;
        }

        SDL_Rect src{ ix * TILE_SIZE, iy * TILE_SIZE, TILE_SIZE, TILE_SIZE };
        SDL_Rect dst{ pos.x, pos.y, src.w, src.h };
        SDL_RenderCopy(renderer.get(), _tex.get(), &src, &dst);
    }

}

void Game::renderSprite(SDL::Renderer& renderer) {
    auto view = _reg.view<puyo::Sprite>();
    for (auto& e : view) {
        auto& sprite = view.get<puyo::Sprite>(e);
        SDL_RenderCopy(renderer.get(), _tex.get(), &sprite.src, &sprite.dst);
    }
}

// -- Update Systems --

void Game::updateInput() {
    auto view = _reg.view<player::Input>();
    for (auto& e : view) {
        auto& input = view.get<player::Input>(e);
        for (size_t i = 0; i < player::InputKey::_Count; i++) {
            
            if (input.keys[i].next) {
                input.keys[i].counter++;
            }
            else {
                input.keys[i].counter = -1;
            }

            // Calculate triggers
            input.keys[i].trigger =
                // initial press
                (input.keys[i].counter == 0) 
                // first repeat
                || (input.keys[i].counter == _settings.buttonRepeatDelay) 
                // subsequent repeats
                || (input.keys[i].counter > _settings.buttonRepeatDelay
                    && ((input.keys[i].counter - _settings.buttonRepeatDelay) % _settings.buttonSubsequentDelay) == 0);
                
            // Update for next frame
            input.keys[i].curr = input.keys[i].next;
        }
    }
}

void Game::control() {
    static auto isBlocked = [&](player::Board board, puyo::GridIndex pos) -> bool {
        return (pos.x >= player::Board::columns) || (pos.x < 0)
            || (pos.y >= player::Board::rows) || (pos.y < 0)
            || (noentity != board.grid[pos.y][pos.x]);
    };
    static auto setMove = [&](entt::entity e, int dx, int dy, int frames) {
        if (_reg.has<puyo::TranslateAnimation>(e)) {
            auto& anim = _reg.get<puyo::TranslateAnimation>(e);
            anim.dx += dx * TILE_SIZE;
            anim.dy += dy * TILE_SIZE;
            anim.frames = frames;
        }
        else {
            _reg.emplace<puyo::TranslateAnimation>(e, dx * TILE_SIZE, dy * TILE_SIZE, frames);
        }
    };
    static auto setRotate = [&](entt::entity e, int srcDx, int srcDy, int dstDx, int dstDy, int frames) {
        if (_reg.has<puyo::RotateAnimation>(e)) {
            auto& anim = _reg.get<puyo::RotateAnimation>(e);
            // Retain source delta
            anim.dstDx = dstDx * TILE_SIZE;
            anim.dstDy = dstDy * TILE_SIZE;
            anim.frames = frames;
        }
        else {
            _reg.emplace<puyo::RotateAnimation>(e,
                srcDx * TILE_SIZE, srcDy * TILE_SIZE, // Source delta
                dstDx * TILE_SIZE, dstDy * TILE_SIZE, // Destination delta
                frames);
        }
    };

    auto view = _reg.view<puyo::Parent, puyo::GridIndex, puyo::RenderPosition, puyo::ControlAxis>();
    for (auto& e : view) {
        const auto& parent = view.get<puyo::Parent>(e).entity;
        auto& index = view.get<puyo::GridIndex>(e);
        auto& renderPosition = view.get<puyo::RenderPosition>(e);
        auto& axis = view.get<puyo::ControlAxis>(e);
        auto& slave = axis.slave;
        auto& slaveIndex = _reg.get<puyo::GridIndex>(slave);
        auto& slaveRenderPosition = _reg.get<puyo::RenderPosition>(slave);

        if (!axis.locked)
            std::cout << "-- (" << index.x << ", " << index.y << ") drop: " << index.drop << " = px:(" << renderPosition.x << ", " << renderPosition.y << ")" << std::endl;

        // Get input and board from player
        if (!_reg.has<player::Input>(parent) || !_reg.has<player::Board>(parent)) {
            continue;
        }
        auto& input = _reg.get<player::Input>(parent);
        auto& board = _reg.get<player::Board>(parent);

        // Accumulate move input triggers
        const int halfTile = TILE_SIZE / 2;
        int8_t dx = 0;
        int8_t dr = 0;
        int dropSpeed = _settings.dropSpeed;
        bool softDrop = false;
        if (!axis.locked) {
            if (input.keys[player::InputKey::Left].trigger) dx--;
            if (input.keys[player::InputKey::Right].trigger) dx++;

            if (input.keys[player::InputKey::RotateLeft].counter == 0) dr--; // no button repeat, only first press
            if (input.keys[player::InputKey::RotateRight].counter == 0) dr++; // no button repeat, only first press

            softDrop = input.keys[player::InputKey::Down].counter >= 0; // no repeat, computed on every frame
            if (_settings.softDropSpeed > _settings.dropSpeed && softDrop) {
                dropSpeed = _settings.softDropSpeed;
            }
        }

        // --- Lateral movement ---

        // Don't allow movement if already moving
        if (!axis.shift && dx != 0) {

            // Check if destination is blocked our out of bounds
            bool blocked = isBlocked(board, { index.x + dx, index.y })
                || isBlocked(board, { slaveIndex.x + dx, slaveIndex.y });

            // Acknowledge movement
            if (!blocked) {
                // Make smooth animations
                setMove(e, dx, 0, LATERAL_SHIFT_FRAMES);
                setMove(slave, dx, 0, LATERAL_SHIFT_FRAMES);
                axis.shift = true;

                // Update pair coordinates
                index.x += dx;
                slaveIndex.x += dx;

                // Play sound effect
                Mix_PlayMusic(_sound.move.get(), 0);
            }
        } else {
            axis.shift = false;
        }

        // --- Rotation ---

        if (dr != 0) {

            bool rotable = true;
            auto dst = puyo::GridIndex{
                index.x - (slaveIndex.y - index.y) * dr,
                index.y + (slaveIndex.x - index.x) * dr
            };
            auto diag = puyo::GridIndex{
                dst.x - index.x + slaveIndex.x,
                dst.y - index.y + slaveIndex.y
            };
            auto push = puyo::GridIndex{ 0, 0 };

            // Check if destination or diagonal is blocked
            if (isBlocked(board, dst) || isBlocked(board, diag)) {

                // Opposite cell check
                auto opposite = puyo::GridIndex{
                    2 * index.x - dst.x,
                    2 * index.y - dst.y,
                };

                // Ghost row limitation: don't allow rotation to upright rotation on ghost rows when blocked (so we can't push up)
                if (dst.y < player::Board::ghostRows && dst.x == index.x) {
                    std::cout << "ghost line pushup disallowed" << std::endl;
                    rotable = false;
                }

                // Double-rotation
                if (isBlocked(board, opposite)) {
                    axis.rotationCounter++;
                    if (axis.rotationCounter % 2 == 1) {
                        rotable = false;
                    }
                    // Perform a quick turn
                    dst = {
                        2 * index.x - slaveIndex.x,
                        2 * index.y - slaveIndex.y,
                    };
                }

                // Push back in opposite direction
                if(rotable) {
                    push = {
                        index.x - dst.x,
                        index.y - dst.y
                    };
                }
            }

            // Acknowledge rotation
            if (rotable) {

                // Reset rotation counter
                axis.rotationCounter = 0;

                // Make smooth animations : Can update previous animations if any
                if (push.x != 0 || push.y != 0) {
                    setMove(e, push.x, push.y, LATERAL_SHIFT_FRAMES);
                    setMove(slave, push.x, push.y, LATERAL_SHIFT_FRAMES);
                }
                setRotate(slave, slaveIndex.x - index.x, slaveIndex.y - index.y, dst.x - index.x, dst.y - index.y, ROTATION_FRAMES);

                // Update pair coordinates
                index.x += push.x;
                index.y += push.y;                
                slaveIndex.x = dst.x + push.x;
                slaveIndex.y = dst.y + push.y;

                // upward push sets just above midline height
                if (push.y < 0) {
                    std::cout << "Pushing y" << push.y << ", drop " << index.drop << " -> 15" << std::endl;
                    axis.pushupCounter++;
                    renderPosition.y -= index.drop;
                    slaveRenderPosition.y -= slaveIndex.drop;                    
                    index.drop = slaveIndex.drop = halfTile - 1;
                    renderPosition.y += index.drop;
                    slaveRenderPosition.y += slaveIndex.drop;
                }

                // Play sound effect
                Mix_PlayMusic(_sound.rotate.get(), 0);
            }
        }

        // --- Soft Drop ---
        // Only check axis, slave is synced        
        bool wasAbove = (index.drop < halfTile);

        // limit drop to end of tile, 
        // so softdrop can only place at mid or top states
        if (index.drop + dropSpeed > TILE_SIZE) dropSpeed = TILE_SIZE - index.drop;

        // Applying drop speed
        index.drop += dropSpeed;
        slaveIndex.drop += dropSpeed;
        renderPosition.y += dropSpeed;
        slaveRenderPosition.y += dropSpeed;

        if (!axis.locked) {
            // Check if cell below  is blocked our out of bounds
            bool blocked = isBlocked(board, { index.x, index.y + 1 })
                || isBlocked(board, { slaveIndex.x, slaveIndex.y + 1 });

            // Going on to the next cell
            if (!blocked && index.drop == TILE_SIZE) {
                index.drop = slaveIndex.drop = 0;
                index.y++, slaveIndex.y++;
            }

            // Start bounce
            if (blocked && index.drop >= halfTile && !_reg.has<puyo::BounceAnimation>(e)) {
                // Play bouncing animation
                // TODO: bounce only if have something under
                _reg.emplace<puyo::BounceAnimation>(e);
                _reg.emplace<puyo::BounceAnimation>(slave);

                // Play placement sound effect 
                Mix_PlayMusic(_sound.drop.get(), 0);
            }

            // Grace period at the bottom
            if (blocked && index.drop == TILE_SIZE) {
                std::cout << "Grace: " << int(axis.graceCounter) << std::endl;
                axis.graceCounter++;
                if (softDrop) { // skip grace period when soft dropping
                    axis.graceCounter = _settings.gracePeriod;
                }
            }

            // Grace period override
            if (blocked && axis.pushupCounter >= _settings.maxPushups) {
                std::cout << "Max pushups, Locking!" << std::endl;
                axis.locked = true;
            }

            // Lock the pair at end of grace period
            if (blocked && index.drop == TILE_SIZE && axis.graceCounter >= _settings.gracePeriod) {
                std::cout << "Locked!" << std::endl;
                axis.locked = true;
            }
        }

        // ------

        // Wait for any rotation animations to finish
        if (_reg.has<puyo::RotateAnimation>(slave)) {
            // wait
            std::cout << "anim playing" << std::endl;
        }

    }

}

void Game::freefall() {

    /*
    For the main puyo:

        initialization updates on-screen coordinates and blocked status under both puyos to check which one should free-fall;
        initializes free-fall parameters if the main puyo shall fall;
        yields execution.
        If the main puyo should not free-fall, the callback skips to the placement phase and yields execution.

    For the slave puyo:

        initialization updates on-screen coordinates;
        splits pair by overwriting the link to the main puyo as a parent object: the new parent object is the relevant player status object;
        yields execution.
    
    */

}
