#ifndef _FOV_H
#define _FOV_H

#include <functional>
#include <memory>
#include <stdexcept>
#include <math.h>

#include "grid.h"

namespace fov {
    struct FOV {
        // Updates FOV - takes an origin point and a 2D vector of ints representing transparency
        // (0 = opaque, 1 = transparent; later I will try to implement translucent tiles)
        std::vector<std::vector<bool>> updateFOV(std::vector<std::vector<int>> transparency, std::pair<int, int> origin) {

            // Sets up a visibility map with the dimensions of the transparency map; all tiles set to false
            std::vector<std::vector<bool>> visibilityMap = {};
            for (size_t y = 0; y < transparency.size(); y++) {
                visibilityMap.push_back({});
                for (size_t x = 0; x < transparency.at(y).size(); x++) {
                    visibilityMap.at(y).push_back(false);
                }   
            }

            // Gets the transparency of a tile at (x, y)
            auto getTransparency = [&transparency](int x, int y) {
                return transparency[y][x];                
            };
            
            // Sets a tile in the map
            auto setTile = [&visibilityMap](Tile tile, bool val) {
                visibilityMap[tile.y].insert(visibilityMap[tile.y].begin() + tile.x, val);
            };

            // Calculates new start and end slopes for Row objects
            auto slope = [](Tile t) {
                int col = t.x;
                return (float) ((2.0f * col - 1.0f) / (2.0f * col));
            };

            // Checks if a given floor tile can be seen symmetrically from the origin
            auto isSymmetric = [](Row row, Tile t) {
                int col = t.x;
                return (col >= row.depth * row.startSlope
                    && col <= row.depth * row.endSlope);
            };

            // Sets the origin point on the map to be visible
            setTile({origin.first, origin.second}, true);

            Quadrant quadrant;
            for (int i = 0; i < 4; i++) { // For each quadrant of the map:
                quadrant = {i, origin};

                // Define lambda functions:

                // Checks if a tile in the transparency map is a wall (opaque)
                auto isWall = [&](Tile t) {
                    std::pair<int, int> coords = quadrant.transform(t);
                    return !getTransparency(coords.first, coords.second);
                };

                // Checks if a tile in the transparency map is a floor (transparent)
                auto isFloor = [&](Tile t) {
                    std::pair<int, int> coords = quadrant.transform(t);
                    return getTransparency(coords.first, coords.second);
                };

                // Marks a tile as visible
                auto reveal = [&quadrant, &setTile](Tile t) {
                    std::pair<int, int> coords = quadrant.transform(t);
                    setTile({coords.first, coords.second}, true);
                };

                // Main function - scans a row and all of its children, recursively
                std::function<void(Row)> scan = [&](Row row) {
                    
                    // Holds the previous tile
                    Tile prevTile;

                    // Holds the tiles of the current row
                    std::vector<Tile> tiles = row.tiles();

                    // Iterates through those tiles
                    for (Tile tile : tiles) {
                        try {
                            // If the tile is a wall or it's symmetric with the current row, reveal it
                            if (isWall(tile) || isSymmetric(row, tile)) {
                                reveal(tile);
                            }

                            // If the previous tile is a wall tile and the current tile is a floor tile, set the row's start slope
                            if (isWall(prevTile) && isFloor(tile)) {
                                row.startSlope = slope(tile);
                            }

                            // If the previous tile is a floor tile, and the current tile is a wall tile, scan the next row
                            if (isFloor(prevTile) && isWall(tile)) {
                                Row nextRow = row.next();
                                nextRow.endSlope = slope(tile);
                                scan(nextRow);
                            }
                            prevTile = tile;
                        } catch (const std::out_of_range& e) {
                            // If the algorithm goes outside the bounds of the map, catch that error and end the algorithm
                            return;
                        }
                    }
                    // If the previous tile is a floor tile, scan the next row
                    if (isFloor(prevTile)) {
                        scan(row.next());
                    }
                };

                // Start scanning!
                Row firstRow = {1, -1.0f, 1.0f};
                scan(firstRow);
            }
            return visibilityMap;
        }

        private:
        // Various other things this algorithm needs

        // Pretty self-explanatory enum
        enum Direction {
            NORTH, SOUTH, EAST, WEST
        };

        // Stores x and y coordinates
        struct Tile {
            int x;
            int y;
        };

        // Represents a row of tiles
        struct Row {
            int depth;
            float startSlope;
            float endSlope;

            // Gets those tiles
            std::vector<Tile> tiles() {
                auto roundTiesUp = [](float n) {
                    return floor(n + 0.5f);
                };

                auto roundTiesDown = [](float n) {
                    return ceil(n - 0.5f);
                };

                std::vector<Tile> toReturn = {};
                float minCol = roundTiesUp(depth * startSlope);
                float maxCol = roundTiesDown(depth * endSlope);
                for (int col = minCol; col <= maxCol; col++) {
                    toReturn.push_back({this->depth, col});
                }
                return toReturn;
            }

            // Gets the next row
            Row next() {
                return Row{depth + 1, startSlope, endSlope};
            }
        };
        
        // Represents one quadrant of the map
        struct Quadrant {

            // Cardinal direction and origin point of this quadrant
            int cardinal;
            std::pair<int, int> origin;

            // Converts the relative coordinates of a tile in a quadrant to absolute coordinates in a map
            std::pair<int, int> transform(Tile tile) {
                std::pair<int, int> relative = std::pair<int, int>(tile.x, tile.y);
                int col = relative.first;
                int row = relative.second;
                int ox = origin.first;
                int oy = origin.second;
                switch (cardinal) {
                    case Direction::NORTH: return {ox + col, oy - row};
                    case Direction::SOUTH: return {ox + col, oy + row};
                    case Direction::EAST: return {ox + row, oy + col};
                    case Direction::WEST: return {ox - row, oy + col};
                    default: throw std::invalid_argument("invalid quadrant direction");
                }
            }
        };
    };
}

#endif