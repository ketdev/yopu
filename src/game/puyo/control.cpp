#include "control.h"
#include "puyo.h"
#include "../player/board.h"
#include "../media/sound.h"
#include <iostream>

void puyo::control(registry& _reg) {
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
        auto& control = view.get<puyo::ControlAxis>(e);
        auto& slave = control.slave;
        auto& slaveIndex = _reg.get<puyo::GridIndex>(slave);
        auto& slaveRenderPosition = _reg.get<puyo::RenderPosition>(slave);

        if (!control.locked)
            std::cout << "(" << index.x << ", " << index.y << ") drop: " << index.drop << " = px:(" << renderPosition.x << ", " << renderPosition.y << ")" << std::endl;

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
        int dropSpeed = control.dropSpeed;
        bool softDrop = false;
        if (!control.locked) {
            if (input.keys[player::InputKey::Left].trigger) dx--;
            if (input.keys[player::InputKey::Right].trigger) dx++;

            if (input.keys[player::InputKey::RotateLeft].counter == 0) dr--; // no button repeat, only first press
            if (input.keys[player::InputKey::RotateRight].counter == 0) dr++; // no button repeat, only first press

            softDrop = input.keys[player::InputKey::Down].counter >= 0; // no repeat, computed on every frame
            if (ControlAxis::softDropSpeed > control.dropSpeed && softDrop) {
                dropSpeed = ControlAxis::softDropSpeed;
            }
        }

        // --- Lateral movement ---

        // Don't allow movement if already moving
        if (!control.shift && dx != 0) {

            // Check if destination is blocked our out of bounds
            bool blocked = isBlocked(board, { index.x + dx, index.y })
                || isBlocked(board, { slaveIndex.x + dx, slaveIndex.y });

            // Acknowledge movement
            if (!blocked) {
                // Make smooth animations
                setMove(e, dx, 0, LATERAL_SHIFT_FRAMES);
                setMove(slave, dx, 0, LATERAL_SHIFT_FRAMES);
                control.shift = true;

                // Update pair coordinates
                index.x += dx;
                slaveIndex.x += dx;

                // Play sound effect
                media::play(_reg, media::SoundEffect::Move);
            }
        }
        else {
            control.shift = false;
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
                    control.rotationCounter++;
                    if (control.rotationCounter % 2 == 1) {
                        rotable = false;
                    }
                    // Perform a quick turn
                    dst = {
                        2 * index.x - slaveIndex.x,
                        2 * index.y - slaveIndex.y,
                    };
                }

                // Push back in opposite direction
                if (rotable) {
                    push = {
                        index.x - dst.x,
                        index.y - dst.y
                    };
                }
            }

            // Acknowledge rotation
            if (rotable) {

                // Reset rotation counter
                control.rotationCounter = 0;

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
                    control.pushupCounter++;
                    renderPosition.y -= index.drop;
                    slaveRenderPosition.y -= slaveIndex.drop;
                    index.drop = slaveIndex.drop = halfTile - 1;
                    renderPosition.y += index.drop;
                    slaveRenderPosition.y += slaveIndex.drop;
                }

                // Play sound effect
                media::play(_reg, media::SoundEffect::Rotate);
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

        if (!control.locked) {
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
                media::play(_reg, media::SoundEffect::Drop);
            }

            // Grace period at the bottom
            if (blocked && index.drop == TILE_SIZE) {
                std::cout << "Grace: " << int(control.graceCounter) << std::endl;
                control.graceCounter++;
                if (softDrop) { // skip grace period when soft dropping
                    control.graceCounter = control.gracePeriod;
                }
            }

            // Grace period override
            if (blocked && control.pushupCounter >= ControlAxis::maxPushups) {
                std::cout << "Max pushups, Locking!" << std::endl;
                control.locked = true;
            }

            // Lock the pair at end of grace period
            if (blocked && index.drop == TILE_SIZE && control.graceCounter >= ControlAxis::gracePeriod) {
                std::cout << "Locked!" << std::endl;
                control.locked = true;
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