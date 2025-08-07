#include <iostream>
#include "memory"

#include "fov.h"

struct Terrain {
    int transparency;
    bool visible;
    char glyph;
};

void setFOV(Terrain* t, bool visible) {
    t->visible = visible;
}

// A render tile stores fg and bg colors, as well as an ASCII glyph to render in a curses window.
// This converts a Terrain object to a RenderTile object based on transparency
render::RenderTile getRenderTile(Terrain t) {
    render::RenderTile toReturn = {};
    toReturn.glyph = t.glyph;
    toReturn.bg = render::Colors::BLACK;
    if (t.visible)
        toReturn.fg = render::Colors::WHITE;
    else
        toReturn.fg = render::Colors::BLACK;
    return toReturn;
}

int main() {

    // Sets up a renderer (the CursesRenderer class abstracts curses functionality)
    render::CursesRenderer renderer = {};
    renderer.init();
    renderer.screenClear();

    // Sets up a grid of tiles
    /*
    ....................
    ....................
    ....................
    ....................
    ....................
    .....#....##........
    ....................
    ....................
    ....................
    ....................
    .....#..............
    ....................
    ....................
    ....................
    ....................
    ...............#....
    ...............#....
    ....................
    ....................
    ....................
    */
    grid::Grid<Terrain> tiles = grid::Grid<Terrain>(20, 20);
    tiles.fill({1, false, '.'});
    tiles.setTile({0, false, '#'}, 5, 5);
    tiles.setTile({0, false, '#'}, 15, 15);
    tiles.setTile({0, false, '#'}, 16, 15);
    tiles.setTile({0, false, '#'}, 10, 5);
    tiles.setTile({0, false, '#'}, 5, 10);
    tiles.setTile({0, false, '#'}, 5, 11);

    // Defines lambda functions for extracting transparency and setting visibility on Terrain objects
    auto transparency = [](Terrain t) { return t.transparency; };
    auto setVisibility = [](Terrain t, bool visible) { t.visible = visible; return t; };

    // Extracts transparency data from the grid
    std::vector<std::vector<int>> tp = tiles.extractAttributes<int>(transparency);

    // Sets up an fovManager
    fov::FOV fovManager = {};

    // Actually calculates FOV
    std::vector<std::vector<bool>> fovTiles = fovManager.updateFOV(tp, {10, 10});
    
    // Sets visibility from fovTiles on the grid
    tiles.fillAttributes<bool>(fovTiles, setVisibility);

    // Finally draws the grid
    renderer.drawGrid<Terrain>(&tiles, getRenderTile);
    renderer.update();
    renderer.getKeyPressChar();

    renderer.end();
    return 0;
}