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

                    // clear puyo from game
                    board.setCell(index, noentity);
                    reg.destroy(puyo);

                    // TODO: pop animation
                }

                clearedColors.insert(color);
            }
        }

        // Score calculation
        if (popped) {
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