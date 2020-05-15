#include "game.hpp"

#include <iostream>
#include <vector>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <cmath>

// -- Typedefs --

static const entt::entity noentity = {};

// -- Game Implementation --

void Game::init(SDL::Renderer& renderer) {
    // seeding a pseudo random number generator with a random source
    _rand.seed(std::random_device{}());

    // Load resources
    _tex = Loader::loadTexture(renderer, ASSET_PATH_TEXTURE);
    _sound.move = Loader::loadMusic(ASSET_PATH_SOUND_MOVE);
    _sound.rotate = Loader::loadMusic(ASSET_PATH_SOUND_ROTATE);

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
    updateInput();
    spawn();
    control();
    // freefall();

    // std::cout << "Hi!" << std::endl;
    return true;
}
void Game::render(SDL::Renderer& renderer, int frame) {
    //std::cout << "Frame: " << frame << std::endl;
    
    updateTranslationAnimation();
    updateRotationAnimation();
    drawPuyos(renderer);
}

// -- Factories --

entt::entity Game::makePlayer(int index) {
    auto player = _reg.create();
    _reg.emplace<player::Input>(player, index);     // Control input
    _reg.emplace<player::Board>(player, _settings.boardRows, _settings.boardColumns); // Game board
    _reg.emplace<player::Idle>(player);             // Initial game status
    return player;
}

entt::entity Game::makePuyo(entt::entity parent, puyo::Type type, uint8_t x, uint8_t y) {
    auto puyo = _reg.create();
    _reg.emplace<puyo::Parent>(puyo, parent);       // Associated player entity
    _reg.emplace<puyo::Color>(puyo, type);
    _reg.emplace<puyo::GridIndex>(puyo, x, y);

    // TODO: render position based on board, shift, etc...
    _reg.emplace<puyo::RenderPosition>(puyo, x * TILE_SIZE, y * TILE_SIZE);
    return puyo;
}

// -- Systems --

void Game::updateTranslationAnimation() {
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

void Game::updateRotationAnimation() {
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
        auto axisBlink = _reg.has<puyo::ControlAxis>(e);
        if (axisBlink) {
            auto& axis = _reg.get<puyo::ControlAxis>(e);
            axis.animationCounter++;
            // 10 frames on, 10 off
            if ((axis.animationCounter / 10) % 2 == 0) {
                axisBlink = false;
            }
        }

        int ix = {}, iy = {};
        switch (type) {
        case puyo::Type::Red:
            ix = 0, iy = 0;
            if (axisBlink) { iy = 9; }
            break;
        case puyo::Type::Green:
            ix = 0, iy = 1;
            if (axisBlink) { ix = 1, iy = 9; }
            break;
        case puyo::Type::Blue:
            ix = 0, iy = 2;
            if (axisBlink) { ix = 2, iy = 9; }
            break;
        case puyo::Type::Yellow:
            ix = 0, iy = 3;
            if (axisBlink) { ix = 3, iy = 9; }
            break;
        case puyo::Type::Violet:
            ix = 0, iy = 4;
            if (axisBlink) { ix = 4, iy = 9; }
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

void Game::spawn() {
    // Our spawn system stores a shared spawn pool used for all players
    static std::vector<puyo::Type> spawnPool;
    static auto nextSpawn = [&](uint32_t index) -> puyo::Type {
        // Create more in pool if needed
        while (index >= spawnPool.size()) {
            // Get random puyo color
            auto type = static_cast<puyo::Type>(_rand() % _settings.colorCount);
            spawnPool.push_back(type);
            std::cout << "Spawn Index: " << index
                << " Type: " << static_cast<int>(type) << std::endl;
        }
        return spawnPool[index];
    };

    auto view = _reg.view<player::Board, player::Idle>();
    for (auto& e : view) {
        auto& board = view.get<player::Board>(e);

        entt::entity axis = noentity;
        for (auto pos : _settings.spawner) {

            // Game Over if one spawner cell is not empty
            if (noentity != board.grid[pos.y][pos.x]) {
                _reg.emplace<player::GameOver>(e);
                return;
            }

            // Create puyo on spawner
            auto type = nextSpawn(board.spawnPoolIndex++);
            auto puyo = makePuyo(e, type, pos.x, pos.y);

            // Set first spawned puyo as controlled
            if (axis == noentity) {
                _reg.emplace<puyo::ControlAxis>(puyo);
                axis = puyo;
            }
            // Set slave parent to axis and axis slave to puyo
            else {                
                _reg.get<puyo::Parent>(puyo).entity = axis;
                _reg.get<puyo::ControlAxis>(axis).slave = puyo;
            }
        }

        _reg.remove<player::Idle>(e);
    }
}

void Game::control() {
    static auto isBlocked = [&](player::Board board, puyo::GridIndex pos) -> bool {
        return (pos.x >= _settings.boardColumns) || (pos.x < 0)
            || (pos.y >= _settings.boardRows) || (pos.y < 0)
            || (noentity != board.grid[pos.y][pos.x]);
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

        // Get input and board from player
        if (!_reg.has<player::Input>(parent) || !_reg.has<player::Board>(parent)) {
            continue;
        }
        auto& input = _reg.get<player::Input>(parent);
        auto& board = _reg.get<player::Board>(parent);

        // Accumulate move input triggers
        int8_t dx = 0;
        if (input.keys[player::InputKey::Left].trigger) dx--;
        if (input.keys[player::InputKey::Right].trigger) dx++;

        int8_t dr = 0;
        if (input.keys[player::InputKey::RotateLeft].counter == 0) dr--; // no button repeat, only first press
        if (input.keys[player::InputKey::RotateRight].counter == 0) dr++; // no button repeat, only first press

        float dy = 0;
        if (input.keys[player::InputKey::Down].trigger) dy += PUYO_MOVE_Y_STEP;

        //if cleanup: (end of game->won ?)
        //    cleanup slave(remove this)
        //    cleanup block(remove this)
        //    return

        /*TEMP*/ bool locked = false; // once the player should not have control over it


        // --- Lateral movement ---

        // Don't allow movement if already moving
        bool shifting = _reg.has<puyo::TranslateAnimation>(e) || _reg.has<puyo::TranslateAnimation>(slave);

        if (!shifting && !locked && dx != 0) {

            // Check if destination is blocked our out of bounds
            bool blocked = isBlocked(board, { index.x + dx, index.y })
                || isBlocked(board, { slaveIndex.x + dx, slaveIndex.y });

            // Acknowledge movement
            if (!blocked) {
                // Make smooth animations
                _reg.emplace<puyo::TranslateAnimation>(e, dx * TILE_SIZE, 0, LATERAL_SHIFT_FRAMES);
                _reg.emplace<puyo::TranslateAnimation>(slave, dx * TILE_SIZE, 0, LATERAL_SHIFT_FRAMES);

                // Update pair coordinates
                index.x += dx;
                slaveIndex.x += dx;

                // Play sound effect
                Mix_PlayMusic(_sound.move.get(), 0);
            }
        }

        // --- Rotation ---

        if (!shifting && !locked && dr != 0) {

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
                if (dst.y < GHOST_ROWS && dst.x == index.x) {
                    std::cout << "ghost line pushup disalowed" << std::endl;
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

                // Make smooth animations
                if (push.x != 0 || push.y != 0) {
                    _reg.emplace<puyo::TranslateAnimation>(e, push.x * TILE_SIZE, push.y * TILE_SIZE, LATERAL_SHIFT_FRAMES);
                    _reg.emplace<puyo::TranslateAnimation>(slave, push.x * TILE_SIZE, push.y * TILE_SIZE, LATERAL_SHIFT_FRAMES);
                }

                // Can replace previous rotation animation if any
                if (_reg.has<puyo::RotateAnimation>(slave)) {
                    auto& anim = _reg.get<puyo::RotateAnimation>(slave);
                    // Retain source delta
                    anim.dstDx = (dst.x - index.x) * TILE_SIZE;
                    anim.dstDy = (dst.y - index.y) * TILE_SIZE;
                    anim.frames = ROTATION_FRAMES;
                }
                else {
                    _reg.emplace<puyo::RotateAnimation>(slave,
                        (slaveIndex.x - index.x) * TILE_SIZE, (slaveIndex.y - index.y) * TILE_SIZE, // Source delta
                        (dst.x - index.x) * TILE_SIZE, (dst.y - index.y) * TILE_SIZE, // Destination delta
                        ROTATION_FRAMES);
                }

                // Update pair coordinates
                index.x += push.x;
                index.y += push.y;                
                slaveIndex.x = dst.x + push.x;
                slaveIndex.y = dst.y + push.y;

                // Play sound effect
                Mix_PlayMusic(_sound.rotate.get(), 0);
            }
        }

        // --- Soft Drop ---

        // pair fall and lock
    }

}