#ifndef _GRID_H
#define _GRID_H

#include <vector>
#include <stdexcept>

namespace grid {
    template <typename T>
    struct Grid {

        // Constructor
        Grid(int w, int h) : width(w), height(h), data(h, std::vector<T>(w)) {
            if (height < 0 || width < 0) {
                throw std::invalid_argument("Grid dimensions cannot be negative.");
            }
        }

        // Fills the grid with copies of a specific element
        void fill(T element, int startX = 0, int startY = 0, size_t w = 0, size_t h = 0) {
            if (w == 0) w = width;
            if (h == 0) h = height;
            for (size_t y = startY; y < h; y++) {
                for (size_t x = startX; x < w; x++) {
                    data[y][x] = element;
                }
            }
        }

        // Extracts an attribute from each element and returns them all as a 2D vector
        template <typename U>
        std::vector<std::vector<U>> extractAttributes(std::function<U(T)> extractFromElement) {
            std::vector<std::vector<U>> toReturn = {};
            for (size_t y = 0; y < data.size(); y++) {
                toReturn.push_back(std::vector<U>{});
                for (size_t x = 0; x < data[y].size(); x++) {
                    toReturn[y].push_back(extractFromElement(data[y][x]));
                }
            }
            return toReturn;
        }

        // Takes a 2D vector like the one returned by extractFromAttributes() and sets the corresponding attributes accordingly
        template <typename U>
        void fillAttributes(std::vector<std::vector<U>> attributeData, std::function<T(T, U)> setAttribute) {
            for (size_t y = 0; y < data.size(); y++) {
                for (size_t x = 0; x < data[y].size(); x++) {
                    setTile(setAttribute(getTile(x, y), attributeData.at(y).at(x)), x, y);
                }
            }
        }

        // Sets a specific tile
        void setTile(T element, int x, int y) {
            data[y][x] = element;
        }

        // Gets a specific tile
        T getTile(int x, int y) {
            return data[y][x];
        }

        // Gets width
        int getWidth() {
            return width;
        }
        
        // Gets height
        int getHeight() {
            return height;
        }

        // Gets data
        std::vector<std::vector<T>> getData() {
            return data;
        }

        // Sets data
        void setData(std::vector<std::vector<T>> data) {
            this->data = data;
        }

        private:
        int width;
        int height;
        std::vector<std::vector<T>> data;
    };
}

#endif