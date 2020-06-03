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

        // -- Utils --

        // Returns the entity at the cell of a given grid index
        // Returns noentity if out of bounds
        inline entity getCell(puyo::GridIndex pos) const {
            if ((pos.x >= columns) || (pos.x < 0)
                || (pos.y >= rows) || (pos.y < 0))
                return noentity;
            return grid[pos.y][pos.x];
        }

        // Sets an entity at the cell with a given grid index
        inline void setCell(puyo::GridIndex pos, entity puyo) {
            if ((pos.x < columns) && (pos.x >= 0)
                && (pos.y < rows) && (pos.y >= 0))
                grid[pos.y][pos.x] = puyo;
        }

        // Checks if a index is non empty or out of bounds within the board grid
        inline bool isBlocked(puyo::GridIndex pos) {
            return ((pos.x >= columns) || (pos.x < 0)
                || (pos.y >= rows) || (pos.y < 0)
                || noentity != grid[pos.y][pos.x]);
        }

	};
    
}
