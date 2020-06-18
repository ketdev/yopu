#include "play.h"
#include <entt/entt.hpp>
#include <vector>

struct Constants {

};

struct Player {
    enum class Phase {
        Settled,
        Control,
        Freefall,
        Chain,
        GameOverWin,
        GameOverLose
    } phase;
};

struct PuyoGameState {
    std::vector<Player> players;

};

void play::step(const Events& events) {
    static PuyoGameState gameState;

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