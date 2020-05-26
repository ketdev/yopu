#pragma once
#include <vector>
#include "../entity.h"
#include "../settings.hpp"
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

    // -- Systems --

    void control();
    void freefall();
    void chain();

}
