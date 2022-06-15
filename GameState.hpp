#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <iomanip>

class GameState {
  public:
    enum Direction {
        UP,
        DOWN,
        LEFT,
        RIGHT
    };

    GameState(const std::vector<int>& data, size_t size) : _data(data), _size(size) {}
    virtual ~GameState() {}

    void swap(Direction d);

    friend std::ostream& operator<<(std::ostream& os, const GameState& table) {
        for (size_t i = 0; i < table._size; i++) {
            for (size_t j = 0; j < table._size; j++)
                os << std::setfill(' ') << std::setw(4) << table._data[i * table._size + j] << " ";
            os << std::endl;
        }
        return os;
    }

  private:
    std::vector<int>    _data;
    size_t              _size;
};
