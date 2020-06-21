#include "play.h"

#include <algorithm>
#include <iostream>
#include <vector>
#include <map>
#include <set>

#include <game/object/player.h>
#include <game/object/board.h>

#include <game/media/sound.h>

/*TEMP*/ #include <game/logic/puyo/puyo.h>
/*TEMP*/ #include <game/logic/puyo/animate.h>

// TODO: optimization, shared pool struct with all subscribers, remove already processed index, save memory
// TODO: spawn garbage puyos as well


static void updateInput(registry& reg);
static void spawn(registry& reg);
static void control(registry& reg);
static void resolve(registry& reg);
static void freefall(registry& reg);


void play::step(State& play, registry& reg) {

    updateInput(reg);
    spawn(reg);

    control(reg);

    freefall(reg);
    resolve(reg);


    //for (auto& player : gameState.players) {
    //    updateInput();
    //
    //    switch (player.phase) {
    //    case Player::Phase::Settled:
    //        if (spawnerBlocked()) {
    //            player.phase = Player::Phase::GameOverLose;
    //            for (auto& other : gameState.players) {
    //                if (other == player) continue;
    //                other.phase = Player::Phase::GameOverWin;
    //            }
    //            return;
    //        }
    //        if (hasGarbage() && !chainedLastStep()) {
    //            spawnGarbage();
    //            player.phase = Player::Phase::Freefall;
    //        } else {
    //            spawnPuyoPair();
    //            player.phase = Player::Phase::Control;
    //        }
    //        break;
    //
    //    case Player::Phase::Control:
    //        if (!movedLastStep()) {
    //            applyMovement();
    //        }
    //        applyRotation();
    //        applyDrop();
    //        if (hitBottom() && !inGracePeriod()) {
    //            pairSplit();
    //            player.phase = Player::Phase::Freefall;
    //        }
    //        break;
    //
    //    case Player::Phase::Freefall:
    //        dropFloating();
    //        if (boardSettled()) {
    //            player.phase = Player::Phase::Chain;
    //        }
    //        break;
    //
    //    case Player::Phase::Chain:
    //        bool popped = false;
    //        auto groups = groupByColors();
    //        for (auto& group : groups) {
    //            if (group.size() >= PopSize) {
    //                popped = true;
    //                popGroupAndGarbageAround(group);
    //                addGroupBonus();
    //            }
    //        }
    //        if (popped) {
    //            calculateScore();
    //            for (auto& other : gameState.players) {
    //                if (other == player) continue;
    //                sendNuisance(other);
    //            }
    //            player.phase = Player::Phase::Freefall;
    //        }
    //        else {
    //            player.phase = Player::Phase::Settled;
    //        }
    //        break;
    //
    //    default:
    //    case Player::Phase::GameOverLose:
    //    case Player::Phase::GameOverWin:
    //        break;
    //    }
    //}
}

// --

static void updateInput(registry& _reg) {
    auto view = _reg.view<player::Input>();
    for (auto& e : view) {
        auto& input = view.get<player::Input>(e);
        for (size_t i = 0; i < player::InputKey::_Count; i++) {

            if (input.keys[i].isDown) {
                input.keys[i].counter++;
            }
            else {
                input.keys[i].counter = -1;
            }

            // Calculate repeat triggers
            input.keys[i].repeat =
                // initial press
                (input.keys[i].counter == 0)
                // first repeat
                || (input.keys[i].counter == player::Input::buttonRepeatDelay)
                // subsequent repeats
                || (input.keys[i].counter > player::Input::buttonRepeatDelay
                    && ((input.keys[i].counter - player::Input::buttonRepeatDelay) % player::Input::buttonSubsequentDelay) == 0);
        }

        // Accumulate move input triggers
        input.dx = 0;
        input.dr = 0;

        if (input.keys[player::InputKey::Left].repeat) input.dx--;
        if (input.keys[player::InputKey::Right].repeat) input.dx++;

        if (input.keys[player::InputKey::RotateLeft].counter == 0) input.dr--; // no button repeat, only first press
        if (input.keys[player::InputKey::RotateRight].counter == 0) input.dr++; // no button repeat, only first press

        input.softDrop = input.keys[player::InputKey::Down].isDown; // no repeat, computed on every frame

        if (input.dx || input.dr || input.softDrop)
            std::cout << "Input X:" << input.dx << " R:" << input.dr << " SD:" << input.softDrop << std::endl;
    }

}

// -- 

// Return a random number from 0 to max
static int randomInt(player::Spawner& spawner, int max) {
    return ((*spawner.randgen)() % max);
}

// Updates shared pool and returns the puyo at a given spawn index
//      spawnPool: a shared spawn pool used for all players
//      index: puyo sequence index
//      colorCount: Number of playable colors (3-5)
static puyo::Color nextColor(player::Spawner& spawner) {
    // Create more in pool if needed
    while (spawner.poolIndex >= spawner.pool->size()) {
        // Get random puyo color
        auto type = static_cast<puyo::Color>((*spawner.randgen)() % spawner.colorCount);
        spawner.pool->push_back(type);
    }
    puyo::Color color = (*spawner.pool)[spawner.poolIndex];
    spawner.poolIndex++;
    spawner.poolIndex %= 128;
    return color;
}

static object makePuyo(registry& reg, puyo::Color type, puyo::GridIndex pos, object player) {
    auto puyo = reg.create();
    reg.emplace<puyo::Color>(puyo, type);
    reg.emplace<puyo::GridIndex>(puyo, pos.x, pos.y, 0);
    reg.emplace<puyo::Parent>(puyo, player);

    // ---
    constexpr int OFFSET_X = 119;
    constexpr int OFFSET_Y = 263;

    // TODO: render position based on board, shift, etc...
    reg.emplace<puyo::RenderPosition>(puyo,
        OFFSET_X + pos.x * puyo::TILE_SIZE,
        OFFSET_Y + pos.y * puyo::TILE_SIZE + pos.drop * puyo::TILE_SIZE / puyo::DROP_RES);

    //reg.emplace<media::Texture>(puyo, media::Texture::Puyo);
    //reg.emplace<media::Position>(puyo,
    //    OFFSET_X + pos.x * puyo::TILE_SIZE,
    //    OFFSET_Y + pos.y * puyo::TILE_SIZE + pos.drop * puyo::TILE_SIZE / puyo::DROP_RES);

    // ---

    return puyo;
}

// SPAWN SYSTEM [Board, Spawner, Idle]
// Spawns puyos and garbages to the player board, 
// if spawner slots are full, sets the Game Over tag on the player
// + Adds garbage puyos
// + Adds Controlled Puyo and slave Puyo
// + Animation: Blinking
static void spawn(registry& reg) {
    auto view = reg.view<board::Board, player::Score, player::Spawner, player::Idle>();
    for (auto& player : view) {
        auto& board = view.get<board::Board>(player);
        auto& score = view.get<player::Score>(player);
        auto& spawner = view.get<player::Spawner>(player);

        // Game Over if one spawner cell is not empty
        if (board.isBlocked(spawner.mainSpawn) || board.isBlocked(spawner.slaveSpawn)) {
            reg.emplace_or_replace<player::GameOver>(player);
            std::cout << "Game Over" << std::endl;
            return;
        }

        // Drop garbage if didn't pop last round
        if (!score.popLastTurn && score.garbage) {

            std::cout << "Dropping garbage: " << score.garbage << std::endl;

            // Spawn garbage rows
            if (score.garbage > board::Columns) {
                int chunk = std::min(player::Score::maxGarbageRows * board::Columns, score.garbage);
                score.garbage -= chunk;

                // TODO
            }
            // Spawn individual garbage puyos
            else {
                bool spawnCol[board::Columns] = {};
                for (auto i = 0; i < score.garbage; i++) {
                    // Get free column to spawn garbage
                    int column = randomInt(spawner, board::Columns - i);
                    while (spawnCol[column]) {
                        column = ((column + 1) % board::Columns);
                    }
                    spawnCol[column] = true;

                    // Create garbage puyos on top row
                    puyo::GridIndex pos = { column, 0, puyo::DROP_RES };
                    auto garbage = makePuyo(reg, puyo::Color::Garbage, pos, player);
                    board.setCell(pos, garbage);
                }
                score.garbage = 0;
            }

            // Skip control phase
            reg.emplace<player::Freefalling>(player);
            reg.emplace<player::Chain>(player);
        }
        else {
            // Reset for new spawn turn
            score.popLastTurn = false;

            // Create puyo on spawners
            auto mainColor = nextColor(spawner);
            auto slaveColor = nextColor(spawner);
            auto mainPuyo = makePuyo(reg, mainColor, spawner.mainSpawn, player);
            auto slavePuyo = makePuyo(reg, slaveColor, spawner.slaveSpawn, player);

            // Set main puyo as controlled, with a reference to slave puyo
            reg.emplace<puyo::Control>(mainPuyo).slave = slavePuyo;
            reg.emplace<puyo::BlinkingAnimation>(mainPuyo);

            std::cout << "Spawn Main: " << spawner.poolIndex << " Type: " << static_cast<int>(mainColor) << std::endl;
            std::cout << "Spawn Slave: " << spawner.poolIndex << " Type: " << static_cast<int>(slaveColor) << std::endl;
        }

        reg.remove<player::Idle>(player);
    }
}

//==================================


// Accumulate move input triggers
constexpr int halfTile = puyo::DROP_RES / 2;
constexpr int LATERAL_SHIFT_FRAMES = 2;
constexpr int ROTATION_FRAMES = 7;

struct ControlFrame {
    registry& reg;
    const player::Input& input;
    board::Board& board;
    const object& main;
    const object& slave;
    const object& player;
    puyo::Control& control;
    puyo::GridIndex& mainIndex;
    puyo::GridIndex& slaveIndex;
    puyo::RenderPosition& mainPos;
    puyo::RenderPosition& slavePos;
};

// Update move animation
static void setMove(registry& reg, object puyo, int dx, int dy, int frames) {
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
// + State: Move sfx
static void applyMovement(const ControlFrame& frame) {

    // Don't allow movement if already moving
    if (!frame.control.shift && !frame.control.locked && frame.input.dx != 0) {

        // Check if destination is blocked our out of bounds
        bool blocked = frame.board.isBlocked({ frame.mainIndex.x + frame.input.dx, frame.mainIndex.y })
            || frame.board.isBlocked({ frame.slaveIndex.x + frame.input.dx, frame.slaveIndex.y });

        // Acknowledge movement
        if (!blocked) {

            // Make smooth animations
            setMove(frame.reg, frame.main, frame.input.dx, 0, LATERAL_SHIFT_FRAMES);
            setMove(frame.reg, frame.slave, frame.input.dx, 0, LATERAL_SHIFT_FRAMES);
            frame.control.shift = true;

            // Update pair coordinates
            frame.mainIndex.x += frame.input.dx;
            frame.slaveIndex.x += frame.input.dx;

            // State sound effect
            media::play(frame.reg, media::SoundEffect::Move);
        }
    }
    else {
        frame.control.shift = false;
    }
}

// Applies slave rotation with pushes
// + Animation: Rotation, Translate
// + State: Rotate sfx
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
        if (frame.board.isBlocked(dst) || frame.board.isBlocked(diag)) {

            // Opposite cell check
            auto opposite = puyo::GridIndex{
                2 * frame.mainIndex.x - dst.x,
                2 * frame.mainIndex.y - dst.y,
            };

            // Ghost row limitation: don't allow rotation to upright rotation on ghost rows when blocked (so we can't push up)
            if (dst.y < board::GhostRows && dst.x == frame.mainIndex.x) {
                std::cout << "ghost line pushup disallowed" << std::endl;
                rotable = false;
            }

            // Double-rotation
            if (frame.board.isBlocked(opposite)) {
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

            // State sound effect
            media::play(frame.reg, media::SoundEffect::Rotate);
        }
    }

}

// Applies drop with Grace Period and soft drop
// + Animation: Bouncing
// + State: Drop sfx
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
        bool mainBlocked = frame.board.isBlocked({ frame.mainIndex.x, frame.mainIndex.y + 1 });
        bool slaveBlocked = frame.board.isBlocked({ frame.slaveIndex.x, frame.slaveIndex.y + 1 });
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
            if (!frame.reg.has<puyo::BounceAnimation>(frame.main)
                && !frame.reg.has<puyo::BounceAnimation>(frame.slave)) {
                // Both bounce if stacked on top of each other
                bool stacked = (frame.mainIndex.x == frame.slaveIndex.x);

                // State bouncing animation
                if (mainBlocked || stacked) {
                    frame.reg.emplace<puyo::BounceAnimation>(frame.main);
                }
                if (slaveBlocked || stacked) {
                    frame.reg.emplace<puyo::BounceAnimation>(frame.slave);
                }

                // State placement sound effect 
                media::play(frame.reg, media::SoundEffect::Drop);
            }

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

// After control is locked and animations finished, separate puyos and either freefall or place on board
// - Animation: Blinking
static void pairSplit(const ControlFrame& frame) {

    // Wait for any rotation animations to finish
    bool rotating = frame.reg.has<puyo::RotateAnimation>(frame.slave);

    // Pair split when reached bottom and is locked out
    if (!rotating && frame.control.locked && frame.mainIndex.drop == puyo::DROP_RES) {
        std::cout << "pair split" << std::endl;

        // Delay placement
        // On Tsu, slave has an additional frame delay, here we do them symetrical
        if (frame.control.splitCounter == puyo::Control::splitFallDelay) {
            frame.reg.remove<puyo::Control>(frame.main);
            frame.reg.remove<puyo::BlinkingAnimation>(frame.main);

            // Set on board
            frame.board.setCell(frame.mainIndex, frame.main);
            frame.board.setCell(frame.slaveIndex, frame.slave);

            // Enter freefalling stage
            frame.reg.emplace_or_replace<player::Freefalling>(frame.player);
            frame.reg.emplace<player::Chain>(frame.player);
        }

        // Advance counter
        frame.control.splitCounter++;
    }
}

// CONTROL SYSTEM [Puyo, Control]
//  Move, Rotate and soft drop a puyo
//  Adds Falling Puyo pair after end of user control
//  Adds Animation: Movement, Rotation, Bounce
//  Plays Sound: Move, Rotate, Drop
static void control(registry& reg) {
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
            || !reg.has<board::Board>(player)) {
            continue;
        }
        auto& input = reg.get<player::Input>(player);
        auto& board = reg.get<board::Board>(player);

        // Build our control frame
        ControlFrame frame = {
            reg,
            input,
            board,
            main,
            slave,
            player,
            control,
            mainIndex,
            slaveIndex,
            mainPos,
            slavePos
        };

        applyMovement(frame);
        applyRotation(frame);
        applyDrop(frame);
        pairSplit(frame);
    }

}


//==================================

// -- 

using group = std::set<object>;

static std::vector<group> groupColors(registry& reg, const board::Board& board) {
    std::vector<group> groups;
    std::map<object, int> puyo2groupIndex;

    // Iterate each column and row, combine with already passed groups or create new 
    for (auto xi = 0; xi < board::Columns; xi++) {
        for (auto yi = board::Rows - 1; yi >= 0; yi--) {

            auto puyo = board.getCell({ xi, yi });
            auto left = board.getCell({ xi - 1, yi });
            auto down = board.getCell({ xi, yi + 1 });
            auto color = reg.try_get<puyo::Color>(puyo);
            auto colorLeft = reg.try_get<puyo::Color>(left);
            auto colorDown = reg.try_get<puyo::Color>(down);

            if (puyo == none || !color) break; // can skip rest of column
            if (*color == puyo::Color::Garbage) continue;

            bool matchLeft = (left != none && colorLeft && *color == *colorLeft);
            bool matchDown = (down != none && colorDown && *color == *colorDown);

            // create new group if no match
            if (!matchLeft && !matchDown) {
                puyo2groupIndex[puyo] = groups.size();
                groups.push_back(group());
                groups.back().insert(puyo);
            }

            auto leftIndex = puyo2groupIndex[left];
            auto downIndex = puyo2groupIndex[down];

            // merge all groups if matching
            if (matchLeft && matchDown) {
                puyo2groupIndex[puyo] = downIndex;
                groups[downIndex].insert(puyo);

                // merge if not already in same group
                if (leftIndex != downIndex) {
                    // transfer left -> down
                    for (auto& p : groups[leftIndex]) {
                        puyo2groupIndex[p] = downIndex;
                        groups[downIndex].insert(p);
                    }

                    // cleanup empty slot, last -> left
                    groups[leftIndex] = groups.back();
                    // update index
                    for (auto& p : groups[leftIndex]) {
                        puyo2groupIndex[p] = leftIndex;
                    }
                    groups.pop_back();
                }
            }

            // match color with left only
            if (matchLeft && !matchDown) {
                puyo2groupIndex[puyo] = leftIndex;
                groups[leftIndex].insert(puyo);
            }

            // match color with down only
            if (!matchLeft && matchDown) {
                puyo2groupIndex[puyo] = downIndex;
                groups[downIndex].insert(puyo);
            }
        }
    }

    return groups;
}

// RESOLVE SYSTEM [Player, Board, Chain]
//  Calculate chains: Identify puyo color groups, pop them if any (enter freefall again)
//  Sets board as Idle after all chains are resolved
//  Adds Gravity to falling puyos after popping
//  Adds Animation: Pop, Sending Chain
//  Plays Sound: Chain [1-7] & Nuisance [S,M,L]
static void resolve(registry& reg) {
    auto view = reg.view<board::Board, player::Score, player::Chain>();
    for (auto& player : view) {
        // Don't chain while freefalling
        if (reg.has<player::Freefalling>(player)) continue;

        std::cout << "resolve" << std::endl;

        auto& board = view.get<board::Board>(player);
        auto& score = view.get<player::Score>(player);
        auto& chain = view.get<player::Chain>(player);

        auto groups = groupColors(reg, board);

        // Score = (10 * PC) * (CP + CB + GB)
        // (10 * puyos cleared * variable factor, capped to 65535)
        // (chain power + color bonus + group bonus, capped to 999)
        // current chain length
        // number of puyo groups to be cleared (for group bonus)

        // Calculate score parts
        int puyoCount = 0;
        int groupBonus = 0;
        std::set<puyo::Color> clearedColors;

        // Pop larger groups
        bool popped = false;
        for (auto& group : groups) {
            if (group.size() >= player::Chain::popSize) {
                popped = true;
                auto size = group.size();

                // Accumulate puyo count
                puyoCount += size;

                // Accumulate group bonus: how many puyo are in the group
                //  4	0	
                //  5	2	
                //  6	3	
                //  7	4	
                //  8	5	
                //  9	6
                //  10	7
                //  11+	10
                if (size > 4) {
                    groupBonus += (size < 11) ? (size - 3) : 10;
                }

                puyo::Color color;
                for (auto& puyo : group) {
                    auto& index = reg.get<puyo::GridIndex>(puyo);
                    color = reg.get<puyo::Color>(puyo);

                    // clear surrounding garbage
                    const auto clearGarbage = [&](int x, int y) {
                        auto n = board.getCell({ x, y });
                        auto color = reg.try_get<puyo::Color>(n);
                        if (color && *color == puyo::Color::Garbage) {
                            board.setCell({ x, y }, none);
                            reg.destroy(n);
                        }
                    };
                    clearGarbage(index.x, index.y - 1);
                    clearGarbage(index.x + 1, index.y);
                    clearGarbage(index.x, index.y + 1);
                    clearGarbage(index.x - 1, index.y);

                    // clear puyo from game
                    board.setCell(index, none);
                    reg.destroy(puyo);

                    // TODO: pop animation
                }

                clearedColors.insert(color);
            }
        }

        // Score calculation
        if (popped) {
            score.popLastTurn = true;
            chain.length++;

            // Calculate chain power: length of chain
            //  1   0
            //  2	8
            //  3	16
            //  4	32
            //  ... +32
            int chainPower = 0;
            switch (chain.length) {
            case 1: chainPower = 0; break;
            case 2: chainPower = 8; break;
            case 3: chainPower = 16; break;
            default: chainPower = 32 * (chain.length - 3); break;
            }

            // Calculate color bonus: number of different color puyo were cleared in the chain
            //  1	0
            //  2	3
            //  3	6
            //  4	12
            //  5	24
            //  6   48
            int colorBonus = 0;
            if (clearedColors.size() > 1)
                colorBonus = 3 * (1 << (clearedColors.size() - 2));

            chain.sizePower = 10 * puyoCount;
            chain.chainPower = std::max(1, std::min(999, chainPower + colorBonus + groupBonus));
            auto chainStepScore = chain.sizePower * chain.chainPower;
            chain.scoreSum += chainStepScore;
            score.sum += chainStepScore;

            /*TEMP*/ std::cout << "-- CHAIN: " << chain.length << std::endl;
            /*TEMP*/ std::cout << "\t(10 * PC) * (CP + CB + GB)" << std::endl;
            /*TEMP*/ std::cout << "\t(10 * " << puyoCount << ") * (" << chainPower << " + " << colorBonus << " + " << groupBonus << ")" << std::endl;
            /*TEMP*/ std::cout << "\t" << chain.sizePower << " * " << chain.chainPower << std::endl;
            /*TEMP*/ std::cout << "\tStep Score: " << chain.sizePower * chain.chainPower << std::endl;

            // Calculate nuisances, while chaining
            double nuisancePoints = (static_cast<double>(chain.scoreSum) / player::Chain::nuisanceCost) + score.garbageLeftover;
            score.chainingGarbage = static_cast<int>(std::floor(nuisancePoints));

            std::cout << "\tChain Garbage: " << score.chainingGarbage << std::endl;

            // Freefall possibly hanging puyos
            reg.emplace<player::Freefalling>(player);
        }
        else {
            // Update garbage leftover for next chain
            double nuisancePoints = (static_cast<double>(chain.scoreSum) / player::Chain::nuisanceCost) + score.garbageLeftover;
            score.garbageLeftover = nuisancePoints - score.chainingGarbage;

            // Accumulate ready garbage
            score.garbage += score.chainingGarbage;
            score.chainingGarbage = 0;

            /*TEMP*/ std::cout << "-- CHAIN END" << std::endl;
            /*TEMP*/ std::cout << "\tChain Sum: " << chain.scoreSum << std::endl;
            /*TEMP*/ std::cout << "\tTotal Score: " << score.sum << std::endl;
            /*TEMP*/ std::cout << "\tTotal Garbage: " << score.garbage << std::endl;
            /*TEMP*/ std::cout << "\tGarbage Leftover: " << score.garbageLeftover << std::endl;

            reg.remove<player::Chain>(player);

            // Enter idle stage
            reg.emplace<player::Idle>(player);
        }

    }
}

// --

//  Adds Animation: Bounce
//  Plays Sound: Drop
static void freefall(registry& reg) {
    auto playerView = reg.view<board::Board, player::Freefalling>();
    for (auto& player : playerView) {
        auto& board = playerView.get<board::Board>(player);

        // Drop hanging puyos
        for (auto x = 0; x < board::Columns; x++) {
            int drop = 0;
            for (auto y = board::Rows - 1; y >= 0; y--) {
                if (!board.isBlocked({ x, y })) {
                    drop++;
                }
                else if (drop > 0 && board.isBlocked({ x,y })) {
                    std::cout << "Hang: (" << x << ", " << y << ") Drop: " << drop << std::endl;

                    // set at target
                    auto puyo = board.getCell({ x, y });
                    board.setCell({ x, y }, none);
                    board.setCell({ x, y + drop }, puyo);
                    reg.emplace_or_replace<puyo::GridIndex>(puyo, x, y + drop, puyo::DROP_RES);

                    // Add gravity animation
                    reg.emplace<puyo::GravityAnimation>(puyo, drop * puyo::TILE_SIZE, 1.0, 8.0, 0.1875);
                }

            }
        }

        // Wait while puyos are falling
        bool isFalling = false;
        auto gravityView = reg.view<puyo::Parent, puyo::GravityAnimation>();
        for (auto& puyo : gravityView) {
            auto& parent = gravityView.get<puyo::Parent>(puyo).player;
            if (parent == player) isFalling = true;
        }

        // Exit freefall stage
        if (!isFalling) {
            reg.remove<player::Freefalling>(player);
        }

    }

    /*
    Cases: Free fall speed differs for:
        puyos controlled by the player, after splitting a pair in two;
        single ojama puyos;
        bulk ojama puyos, depending on their respective column;
        when a chain disappears, puyos above the hole fill the gap by free-falling.


    Gravity acceleration

    Placement
        * When the gravity routine detects that the puyo has reached the floor, execution
        yields once again, calling the placement routine on the very next frame after
        free-fall. Both callbacks mark their respective object for cleanup and call the routine
        which will place the puyo on the virtual board representation in RAM.

    + Board settled event

    */
}
