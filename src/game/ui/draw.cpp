#include "draw.h"

#include <algorithm>
#include <memory>
#include <engine/draw/sprite_render.h>
#include <engine/draw/color_render.h>

// rendered objects
#include <game/object/board.h>

struct DrawState {
    ColorRender colorRender;
    SpriteRender spriteRender;    
};

static glm::vec4 color(int r, int g, int b, float a) {
    return glm::vec4{ float(r) / 0xff, float(g) / 0xff, float(b) / 0xff, a };
}
static glm::vec4 color(int hex, float alpha = 1.0) {
    return glm::vec4{ 
        float((hex >> 8 * 2) & 0xff) / 0xff,
        float((hex >> 8 * 1) & 0xff) / 0xff, 
        float((hex >> 8 * 0) & 0xff) / 0xff, 
        alpha
    };
}

void ui::draw(int frame, int width, int height) {
    static DrawState state;

    state.colorRender.update(width, height);
    state.spriteRender.update(width, height);

    // layout sizes
    const int sideMargin = width * 4 / 100;

    const int controlColumnWidth = width * 20 / 100;
    const int controlTopOffset = height * 15 / 100;
    const int controlHeight = controlColumnWidth * 25 / 30; // maintain proportions

    // we need to fit whole board, garbage and score areas
    const int boardCellSize = std::min(
        width * 20 / 100 / board::Columns,
        height / board::Rows);

    const int boardWidth = boardCellSize * board::Columns;
    const int boardHeight = boardCellSize * (board::Rows - board::GhostRows);
    const int boardTopOffset = (height - boardHeight) / 2; // center

    const glm::ivec2 nextMargin = { boardCellSize / 5, boardCellSize / 2 };
    const glm::ivec2 next2Margin = { boardCellSize / 4, 0 };

    const int middleArea = width - 2 *(sideMargin + controlColumnWidth + boardWidth);

    // colors
    const glm::vec4 marginColor = color(54, 129, 96, 0.5f);

    const glm::vec4 controlColumnColor = color(254, 221, 106, 0.5f);
    const glm::vec4 controlAreaColor = color(61, 231, 227, 0.5f);

    const glm::vec4 boardColumnColor = color(31, 188, 245, 0.5f);
    const glm::vec4 boardCellColor = color(0x50cff7, 0.5f);
    const glm::vec4 boardCellColor2 = color(0x1fbcf5, 0.5f);
    
    const glm::vec4 middleAreaColor = color(237, 88, 120, 0.5f);

    // margin
    state.colorRender.draw({ 0, 0 }, { sideMargin, height }, marginColor);
    state.colorRender.draw({ width - sideMargin, 0 }, { sideMargin, height }, marginColor);

    // control column
    state.colorRender.draw({ sideMargin, 0 }, { controlColumnWidth, height }, controlColumnColor);
    state.colorRender.draw({ width - sideMargin - controlColumnWidth,0 }, { controlColumnWidth, height }, controlColumnColor);

    // control area
    state.colorRender.draw({ sideMargin, controlTopOffset }, { controlColumnWidth, controlHeight }, controlAreaColor);
    state.colorRender.draw({ width - sideMargin - controlColumnWidth, controlTopOffset }, { controlColumnWidth, controlHeight }, controlAreaColor);

    // board column
    state.colorRender.draw({ sideMargin + controlColumnWidth, 0 }, { boardWidth, height }, boardColumnColor);
    state.colorRender.draw({ width - sideMargin - controlColumnWidth - boardWidth,0 }, { boardWidth, height }, boardColumnColor);

    // board cells
    //state.colorRender.draw({ sideMargin + controlColumnWidth, boardTopOffset }, { boardColumnWidth, boardHeight }, boardCellColor);
    state.colorRender.draw({ width - sideMargin - controlColumnWidth - boardWidth, boardTopOffset }, { boardWidth, boardHeight }, boardCellColor);

    // player1 board
    glm::vec2 b0offset = { sideMargin + controlColumnWidth, boardTopOffset };
    glm::vec2 cellSize = { boardCellSize, boardCellSize };
    for (int x = 0; x < board::Columns; x++) {
        for (int y = 0; y < board::Rows - board::GhostRows; y++) {
            glm::vec2 pos = b0offset + glm::vec2{x * boardCellSize, y * boardCellSize};
            glm::vec4 color = boardCellColor;
            if ((x + y) % 2 == 0)
                color = boardCellColor2;
            state.colorRender.draw(pos, cellSize, color);
        }
    }

    // player1 next
    state.colorRender.draw(
        { sideMargin + controlColumnWidth + boardWidth + nextMargin.x, boardTopOffset + nextMargin.y },
        cellSize, boardCellColor);
    state.colorRender.draw(
        { sideMargin + controlColumnWidth + boardWidth + nextMargin.x, boardTopOffset + nextMargin.y + 1 * cellSize.y },
        cellSize, boardCellColor2);

    state.colorRender.draw(
        { sideMargin + controlColumnWidth + boardWidth + nextMargin.x + next2Margin.x, boardTopOffset + nextMargin.y + 2 * cellSize.y + next2Margin.y },
        cellSize, boardCellColor);
    state.colorRender.draw(
        { sideMargin + controlColumnWidth + boardWidth + nextMargin.x + next2Margin.x, boardTopOffset + nextMargin.y + 3 * cellSize.y + next2Margin.y },
        cellSize, boardCellColor2);

    // middle column
    state.colorRender.draw({ sideMargin + controlColumnWidth + boardWidth, 0 }, { middleArea, height }, middleAreaColor);

}
