#pragma once
#include <vector>

#include <game/object/object.h>
#include <game/logic/puyo/puyo.h>

namespace board {

    // -- Constants --

    // 6x12 + hidden rows
    static constexpr int GhostRows = 2;
    static constexpr int Rows = 12 + GhostRows;
    static constexpr int Columns = 6;

    // -- Tags --

    // -- Components --

	struct Board {
        // Our entity grid
        std::vector<std::vector<object>> grid;
        Board() {
            grid.resize(Rows);
            for (auto& row : grid) {
                row.resize(Columns);
            }
        }

        // -- Utils --

        // Returns the entity at the cell of a given grid index
        // Returns none if out of bounds
        inline object getCell(puyo::GridIndex pos) const {
            if ((pos.x >= Columns) || (pos.x < 0)
                || (pos.y >= Rows) || (pos.y < 0))
                return none;
            return grid[pos.y][pos.x];
        }

        // Sets an entity at the cell with a given grid index
        inline void setCell(puyo::GridIndex pos, object puyo) {
            if ((pos.x < Columns) && (pos.x >= 0)
                && (pos.y < Rows) && (pos.y >= 0))
                grid[pos.y][pos.x] = puyo;
        }

        // Checks if a index is non empty or out of bounds within the board grid
        inline bool isBlocked(puyo::GridIndex pos) {
            return ((pos.x >= Columns) || (pos.x < 0)
                || (pos.y >= Rows) || (pos.y < 0)
                || none != grid[pos.y][pos.x]);
        }

	};
    
}
