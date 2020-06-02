#include "resolve.h"
#include "board.h"
#include "spawner.h"
#include "../puyo/freefall.h"
#include "../puyo/animate.h"
#include <iostream>
#include <map>
#include <vector>
#include <set>

using group = std::set<entity>;

static std::vector<group> groupColors(registry& reg, const player::Board& board) {
    std::vector<group> groups;
    std::map<entity, int> puyo2groupIndex;

    // Iterate each column and row, combine with already passed groups or create new 
    for (auto xi = 0; xi < player::Board::columns; xi++) {
        for (auto yi = player::Board::rows - 1; yi >= 0; yi--) {

            auto puyo = board.getCell({ xi, yi });
            auto left = board.getCell({ xi - 1, yi });
            auto down = board.getCell({ xi, yi + 1 });
            auto color = reg.try_get<puyo::Color>(puyo);
            auto colorLeft = reg.try_get<puyo::Color>(left);
            auto colorDown = reg.try_get<puyo::Color>(down);

            if (puyo == noentity || !color) break; // can skip rest of column
            if (*color == puyo::Color::Garbage) continue;
            
            bool matchLeft = (left != noentity && colorLeft && *color == *colorLeft);
            bool matchDown = (down != noentity && colorDown && *color == *colorDown);

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

void player::resolve(registry& reg) {
    auto view = reg.view<player::Board, player::Score, player::Chain>();
    for (auto& player : view) {
        // Don't chain while freefalling
        if (reg.has<player::Freefalling>(player)) continue;

        std::cout << "resolve" << std::endl;

        auto& board = view.get<player::Board>(player);
        auto& score = view.get<player::Score>(player);
        auto& chain = view.get<player::Chain>(player);

        auto groups = groupColors(reg, board);

        /*TEMP*/ // Print groups
        /*TEMP*/ std::cout << "Groups:" << std::endl;
        /*TEMP*/ int groupIndex = 0;
        /*TEMP*/ for (auto& g : groups) {
        /*TEMP*/     std::cout << "  Group:" << groupIndex++ << std::endl;
        /*TEMP*/     for (auto& p : g) {
        /*TEMP*/         auto color = reg.try_get<puyo::Color>(p);
        /*TEMP*/         auto index = reg.try_get<puyo::GridIndex>(p);
        /*TEMP*/         std::cout << "    (" << index->x << "," << index->y << ") color: " << int(*color) << std::endl;
        /*TEMP*/     }
        /*TEMP*/ }
        /*TEMP*/ // Print board
        /*TEMP*/ std::cout << "Board:" << std::endl;
        /*TEMP*/ for (auto yi = 0; yi < player::Board::rows; yi++) {
        /*TEMP*/     for (auto xi = 0; xi < player::Board::columns; xi++) {
        /*TEMP*/         auto puyo = board.getCell({ xi, yi });
        /*TEMP*/         auto colorPtr = reg.try_get<puyo::Color>(puyo);
        /*TEMP*/         if (colorPtr) {
        /*TEMP*/             std::cout << int(*colorPtr);
        /*TEMP*/         }
        /*TEMP*/         else {
        /*TEMP*/             std::cout << '.';
        /*TEMP*/         }
        /*TEMP*/     }
        /*TEMP*/     std::cout << std::endl;
        /*TEMP*/ }

        // Pop larger groups
        bool popped = false;
        for (auto& group : groups) {
            if (group.size() >= player::groupPopSize) {
                popped = true;
                for (auto& puyo : group) {
                    auto& index = reg.get<puyo::GridIndex>(puyo);

                    // clear puyo from game
                    board.setCell(index, noentity);
                    reg.destroy(puyo);

                    // TODO: pop animation
                }
            }
        }

        // Freefall possibly hanging puyos
        if (popped) {
            reg.emplace<player::Freefalling>(player);
        }
        // Enter idle stage
        else {
            reg.remove<player::Chain>(player);
            reg.emplace<player::Idle>(player);
        }
    }
}