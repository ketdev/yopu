#include "control.h"
#include "puyo.h"
#include "freefall.h"
#include "../player/board.h"
#include "../player/input.h"
#include "../media/sound.h"
#include <iostream>

// Accumulate move input triggers
constexpr int halfTile = puyo::DROP_RES / 2;
constexpr int LATERAL_SHIFT_FRAMES = 2;
constexpr int ROTATION_FRAMES = 7;

struct ControlFrame {
    registry& reg;
    const player::Input& input;
    const player::Board& board;
    const entity& main;
    const entity& slave;
    puyo::Control& control;
    puyo::GridIndex& mainIndex;
    puyo::GridIndex& slaveIndex;
    puyo::RenderPosition& mainPos;
    puyo::RenderPosition& slavePos;
};

// Update move animation
static void setMove(registry& reg, entt::entity puyo, int dx, int dy, int frames) {
    if (reg.has<puyo::TranslateAnimation>(puyo)) {
        auto& anim = reg.get<puyo::TranslateAnimation>(puyo);
        anim.dx += dx * puyo::TILE_SIZE;
        anim.dy += dy * puyo::TILE_SIZE;
        anim.frames = frames;
    }
    else {
        reg.emplace<puyo::TranslateAnimation>(puyo, dx * puyo::TILE_SIZE, dy * puyo::TILE_SIZE, frames);
    }
};

// Update rotate animation
static void setRotate(registry& reg, entt::entity puyo, int srcDx, int srcDy, int dstDx, int dstDy, int frames) {
    if (reg.has<puyo::RotateAnimation>(puyo)) {
        auto& anim = reg.get<puyo::RotateAnimation>(puyo);
        // Retain source delta
        anim.dstDx = dstDx * puyo::TILE_SIZE;
        anim.dstDy = dstDy * puyo::TILE_SIZE;
        anim.frames = frames;
    }
    else {
        reg.emplace<puyo::RotateAnimation>(puyo,
            srcDx * puyo::TILE_SIZE, srcDy * puyo::TILE_SIZE, // Source delta
            dstDx * puyo::TILE_SIZE, dstDy * puyo::TILE_SIZE, // Destination delta
            frames);
    }
};


// Applies lateral movement
// + Animation: Translate
// + Play: Move sfx
static void applyMovement(const ControlFrame& frame) {

    // Don't allow movement if already moving
    if (!frame.control.shift && !frame.control.locked && frame.input.dx != 0) {

        // Check if destination is blocked our out of bounds
        bool blocked = isBlocked(frame.board, { frame.mainIndex.x + frame.input.dx, frame.mainIndex.y })
            || isBlocked(frame.board, { frame.slaveIndex.x + frame.input.dx, frame.slaveIndex.y });

        // Acknowledge movement
        if (!blocked) {

            // Make smooth animations
            setMove(frame.reg, frame.main, frame.input.dx, 0, LATERAL_SHIFT_FRAMES);
            setMove(frame.reg, frame.slave, frame.input.dx, 0, LATERAL_SHIFT_FRAMES);
            frame.control.shift = true;

            // Update pair coordinates
            frame.mainIndex.x += frame.input.dx;
            frame.slaveIndex.x += frame.input.dx;

            // Play sound effect
            media::play(frame.reg, media::SoundEffect::Move);
        }
    }
    else {
        frame.control.shift = false;
    }
}

// Applies slave rotation with pushes
// + Animation: Rotation, Translate
// + Play: Rotate sfx
static void applyRotation(const ControlFrame& frame) {
    if (!frame.control.locked && frame.input.dr != 0) {

        bool rotable = true;
        auto dst = puyo::GridIndex{
            frame.mainIndex.x - (frame.slaveIndex.y - frame.mainIndex.y) * frame.input.dr,
            frame.mainIndex.y + (frame.slaveIndex.x - frame.mainIndex.x) * frame.input.dr
        };
        auto diag = puyo::GridIndex{
            dst.x - frame.mainIndex.x + frame.slaveIndex.x,
            dst.y - frame.mainIndex.y + frame.slaveIndex.y
        };
        auto push = puyo::GridIndex{ 0, 0 };

        // Check if destination or diagonal is blocked
        if (isBlocked(frame.board, dst) || isBlocked(frame.board, diag)) {

            // Opposite cell check
            auto opposite = puyo::GridIndex{
                2 * frame.mainIndex.x - dst.x,
                2 * frame.mainIndex.y - dst.y,
            };

            // Ghost row limitation: don't allow rotation to upright rotation on ghost rows when blocked (so we can't push up)
            if (dst.y < player::Board::ghostRows && dst.x == frame.mainIndex.x) {
                std::cout << "ghost line pushup disallowed" << std::endl;
                rotable = false;
            }

            // Double-rotation
            if (isBlocked(frame.board, opposite)) {
                frame.control.rotationCounter++;
                if (frame.control.rotationCounter % 2 == 1) {
                    rotable = false;
                }
                // Perform a quick turn
                dst = {
                    2 * frame.mainIndex.x - frame.slaveIndex.x,
                    2 * frame.mainIndex.y - frame.slaveIndex.y,
                };
            }

            // Push back in opposite direction
            if (rotable) {
                push = {
                    frame.mainIndex.x - dst.x,
                    frame.mainIndex.y - dst.y
                };
            }
        }

        // Acknowledge rotation
        if (rotable) {

            // Reset rotation counter
            frame.control.rotationCounter = 0;

            // Make smooth animations : Can update previous animations if any
            if (push.x != 0 || push.y != 0) {
                setMove(frame.reg, frame.main, push.x, push.y, LATERAL_SHIFT_FRAMES);
                setMove(frame.reg, frame.slave, push.x, push.y, LATERAL_SHIFT_FRAMES);
            }
            setRotate(frame.reg, frame.slave, 
                frame.slaveIndex.x - frame.mainIndex.x, 
                frame.slaveIndex.y - frame.mainIndex.y, 
                dst.x - frame.mainIndex.x, 
                dst.y - frame.mainIndex.y, ROTATION_FRAMES);

            // Update pair coordinates
            frame.mainIndex.x += push.x;
            frame.mainIndex.y += push.y;
            frame.slaveIndex.x = dst.x + push.x;
            frame.slaveIndex.y = dst.y + push.y;

            // upward push sets just above midline height
            if (push.y < 0) {
                std::cout << "Pushing y" << push.y << ", drop " << frame.mainIndex.drop << " -> 15" << std::endl;
                frame.control.pushupCounter++;
                frame.mainPos.y -= frame.mainIndex.drop * puyo::TILE_SIZE / puyo::DROP_RES;
                frame.slavePos.y -= frame.slaveIndex.drop * puyo::TILE_SIZE / puyo::DROP_RES;
                frame.mainIndex.drop = frame.slaveIndex.drop = puyo::DROP_RES / 2;
                frame.mainPos.y += frame.mainIndex.drop * puyo::TILE_SIZE / puyo::DROP_RES;
                frame.slavePos.y += frame.slaveIndex.drop * puyo::TILE_SIZE / puyo::DROP_RES;
            }

            // Play sound effect
            media::play(frame.reg, media::SoundEffect::Rotate);
        }
    }

}

// Applies drop with Grace Period and soft drop
// + Animation: Bouncing
// + Play: Drop sfx
static void applyDrop(const ControlFrame& frame) {

    // Accumulate move input triggers
    int dropSpeed = frame.control.dropSpeed;
    bool softDrop = frame.input.softDrop;

    if (!frame.control.locked && softDrop && puyo::Control::softDropSpeed > frame.control.dropSpeed) {
        dropSpeed = puyo::Control::softDropSpeed;
    }

    // limit drop to end of tile, 
    // so softdrop can only place at mid or top states
    if (frame.mainIndex.drop + dropSpeed > puyo::DROP_RES) dropSpeed = puyo::DROP_RES - frame.mainIndex.drop;

    // Applying drop speed
    frame.mainIndex.drop += dropSpeed;
    frame.slaveIndex.drop += dropSpeed;
    frame.mainPos.y += dropSpeed * puyo::TILE_SIZE / puyo::DROP_RES;
    frame.slavePos.y += dropSpeed * puyo::TILE_SIZE / puyo::DROP_RES;

    if (!frame.control.locked) {
        // Check if cell below  is blocked our out of bounds
        bool mainBlocked = isBlocked(frame.board, { frame.mainIndex.x, frame.mainIndex.y + 1 });
        bool slaveBlocked = isBlocked(frame.board, { frame.slaveIndex.x, frame.slaveIndex.y + 1 });
        bool blocked = mainBlocked || slaveBlocked;

        // Going on to the next cell
        if (!blocked && frame.mainIndex.drop == puyo::DROP_RES) {
            frame.mainIndex.drop = frame.slaveIndex.drop = 0;
            frame.mainIndex.y++, frame.slaveIndex.y++;
        }

        // Grace period at the bottom
        if (blocked && frame.mainIndex.drop == puyo::DROP_RES) {
            // Start bounce
            // In Tsu, plays bounce right after passing half a cell, 
            // but drop display position is different (rendered a half tile ahead)
            // Instead, we play bounce animations but without delaying gameplay
            if (!frame.reg.has<puyo::BounceAnimation>(frame.main)) {
                // Both bounce if stacked on top of each other
                bool stacked = (frame.mainIndex.x == frame.slaveIndex.x);

                // Play bouncing animation
                if (mainBlocked || stacked) {
                    frame.reg.emplace<puyo::BounceAnimation>(frame.main);
                }
                if (slaveBlocked || stacked) {
                    frame.reg.emplace<puyo::BounceAnimation>(frame.slave);
                }

                // Play placement sound effect 
                media::play(frame.reg, media::SoundEffect::Drop);
            }

            std::cout << "Grace: " << int(frame.control.graceCounter) << std::endl;
            frame.control.graceCounter++;
            if (softDrop) { // skip grace period when soft dropping
                frame.control.graceCounter = frame.control.gracePeriod;
            }
        }

        // Grace period override
        if (blocked && frame.control.pushupCounter >= puyo::Control::maxPushups) {
            std::cout << "Max pushups, Locking!" << std::endl;
            frame.control.locked = true;
        }

        // Lock the pair at end of grace period
        if (blocked && frame.mainIndex.drop == puyo::DROP_RES && frame.control.graceCounter >= puyo::Control::gracePeriod) {
            std::cout << "Locked!" << std::endl;
            frame.control.locked = true;
        }
    }
}


void puyo::control(registry& reg) {
    auto view = reg.view<puyo::GridIndex, puyo::RenderPosition, puyo::Parent, puyo::Control>();
    for (auto& main : view) {
        // Get main and slave structures
        auto& mainIndex = view.get<puyo::GridIndex>(main);
        auto& control = view.get<puyo::Control>(main);
        auto& player = view.get<puyo::Parent>(main).player;
        auto& slave = control.slave;
        auto& slaveIndex = reg.get<puyo::GridIndex>(slave);
        auto& mainPos = view.get<puyo::RenderPosition>(main);
        auto& slavePos = reg.get<puyo::RenderPosition>(slave);

        // Get input and board from associated player
        if (!reg.has<player::Input>(player) 
            || !reg.has<player::Board>(player)) {
            continue;
        }
        auto& input = reg.get<player::Input>(player);
        auto& board = reg.get<player::Board>(player);

        // Build our control frame
        ControlFrame frame = {
            reg,
            input,
            board,
            main,
            slave,
            control,
            mainIndex,
            slaveIndex,
            mainPos,
            slavePos
        };

        applyMovement(frame);
        applyRotation(frame);
        applyDrop(frame);

        /*TEMP*/ if (input.keys[player::InputKey::Up].isDown) {
            control.locked = true;
            mainIndex.drop == puyo::DROP_RES;
        }

        // Wait for any rotation animations to finish
        bool rotating = reg.has<puyo::RotateAnimation>(slave);

        /*TEMP*/ if (rotating) std::cout << "anim playing" << std::endl;

        // Pair split when reached bottom and is locked out
        if (!rotating && frame.control.locked && frame.mainIndex.drop == puyo::DROP_RES) {
            reg.remove<Control>(frame.main);

            // Freefall puyo that isn't blocked
            bool mainBlocked = isBlocked(frame.board, { frame.mainIndex.x, frame.mainIndex.y + 1 });
            bool slaveBlocked = isBlocked(frame.board, { frame.slaveIndex.x, frame.slaveIndex.y + 1 });
            bool stacked = (frame.mainIndex.x == frame.slaveIndex.x);

            // Setup gravity parameters
            if (!stacked && !mainBlocked) {
                auto& mainGravity = reg.emplace<Gravity>(frame.main, Gravity::mainPuyoFallDelay,
                    Gravity::puyoInitialSpeed, Gravity::puyoTerminalSpeed, Gravity::puyoAcceleration);
            }
            else {
                // TODO: next step: placement
            }
            if (!stacked && !slaveBlocked) {
                auto& slaveGravity = reg.emplace<Gravity>(frame.slave, Gravity::slavePuyoFallDelay,
                    Gravity::puyoInitialSpeed, Gravity::puyoTerminalSpeed, Gravity::puyoAcceleration);
            }
            else {
                // TODO: next step: placement
            }
        }
    }

}