#pragma once
#include <vector>
#include "../entity.h"
#include "../puyo/puyo.h"

namespace player {

    // -- Tags --

    // -- Components --

	struct Board {
        // 6x12 + hidden rows
        static constexpr int ghostRows = 2;
        static constexpr int rows = 12 + ghostRows;
        static constexpr int columns = 6;

        // Our entity grid
        std::vector<std::vector<entity>> grid;
        Board() {
            grid.resize(rows);
            for (auto& row : grid) {
                row.resize(columns);
            }
        }
	};

    // -- Utils --

    // Returns the entity at the cell of a given grid index
    // Returns noentity if out of bounds
    static inline entity getCell(player::Board board, puyo::GridIndex pos) {
        if ((pos.x >= player::Board::columns) || (pos.x < 0)
            || (pos.y >= player::Board::rows) || (pos.y < 0))
            return noentity;
        return board.grid[pos.y][pos.x];
    }

    // Checks if a index is non empty or out of bounds within the board grid
    static inline bool isBlocked(player::Board board, puyo::GridIndex pos) {
        return ((pos.x >= player::Board::columns) || (pos.x < 0)
            || (pos.y >= player::Board::rows) || (pos.y < 0) 
            || noentity != board.grid[pos.y][pos.x]);
    }

}
