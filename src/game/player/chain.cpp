#include "chain.h"
#include "board.h"
#include "spawner.h"
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
            
            // match color with left
            if (left != noentity && colorLeft && *color == *colorLeft) {
                auto groupIndex = puyo2groupIndex[left];

                // add to group
                puyo2groupIndex[puyo] = groupIndex;
                groups[groupIndex].insert(puyo);
            }

            // match color with below
            if (down != noentity && colorDown && *color == *colorDown) {
                auto groupIndex = puyo2groupIndex[down];

                // merge groups
                if (puyo2groupIndex.find(puyo) != puyo2groupIndex.end()) {
                    auto toMerge = puyo2groupIndex[puyo];
                    for (auto& p : groups[toMerge]) {
                        groups[groupIndex].insert(p);
                        puyo2groupIndex[p] = groupIndex;
                    }
                    
                    // remove old group
                    groups[toMerge] = groups.back();
                    groups.pop_back();
                } 
                else {
                    // add to group
                    puyo2groupIndex[puyo] = groupIndex;
                    groups[groupIndex].insert(puyo);
                }
            }

            // make new solo group
            if (puyo2groupIndex.find(puyo) == puyo2groupIndex.end()) {
                puyo2groupIndex[puyo] = groups.size();
                groups.push_back(group());
                groups.back().insert(puyo);
            }
        }
    }

    return groups;
}

void player::resolve(registry& reg) {
    auto view = reg.view<player::Board, player::Score, player::Chain>();
    for (auto& player : view) {
        auto& board = view.get<player::Board>(player);
        auto& score = view.get<player::Score>(player);
        auto& chain = view.get<player::Chain>(player);

        auto groups = groupColors(reg, board);


        std::cout << "Calculating chains" << std::endl;

        // move to idle stage
        reg.remove<player::Chain>(player);
        reg.emplace<player::Idle>(player);
    }
}