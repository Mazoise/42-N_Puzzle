#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <map>
#include <memory>
#include "RandomTable.hpp"

class GameState {
  public:

    struct Point {
        Point(int x = 0, int y = 0) : x(x), y(y) {}

        Point operator+(const Point& other) const {
            return Point(x + other.x, y + other.y);
        }

        Point operator-(const Point& other) const {
            return Point(x - other.x, y - other.y);
        }

        Point operator+=(const Point& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        Point operator-=(const Point& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Point operator*(const int& other) const {
            return Point(x * other, y * other);
        }

        Point operator/(const int& other) const {
            return Point(x / other, y / other);
        }

        bool in_bounds(int size) const {
            return x >= 0 && x < size && y >= 0 && y < size;
        }

        int x;
        int y;
    };

    enum Direction {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };

    static std::map<Direction, Point> directions;

    GameState(const std::vector<int>& data, size_t size, const RandomTable &table) : _data(data), _size(size), _table(table) {
        auto i = std::find(_data.begin(), _data.end(), 0);
        size_t index = std::distance(_data.begin(), i);
        _zero = Point(index % _size, (int)(index / _size));
        _hash = 0;
        for (size_t i = 0; i < _data.size(); ++i)
            if (_data[i] != 0)
                _hash ^= _table(_data[i], i % _size, (size_t)(i / _size));
    }

    virtual ~GameState() {}

    void swap(Direction d) {
        Point new_pos = _zero + directions[d];
        if (!new_pos.in_bounds(_size)) 
            return ;
        int nb = (*this)[new_pos];
        _hash ^= _table(nb, new_pos.x, new_pos.y);
        _hash ^= _table(nb, _zero.x, _zero.y);
        std::swap((*this)[_zero], (*this)[new_pos]);
        _zero = new_pos;
    }

    size_t size() const {
        return _size;
    }

    uint64_t hash() const {
        return _hash;
    }

    int operator[](Point p) const {
        return _data[p.x + p.y * _size];
    }

    int& operator[](Point p) {
        return _data[p.x + p.y * _size];
    }

    friend std::ostream& operator<<(std::ostream& os, const GameState& table) {
        for (size_t i = 0; i < table._size; i++) {
            for (size_t j = 0; j < table._size; j++)
                os << std::setfill(' ') << std::setw(4) << table._data[i * table._size + j] << " ";
            os << std::endl;
        }
        return os;
    }

  private:
    std::vector<int> _data;
    size_t _size;
    Point _zero;
    uint64_t _hash;
    const RandomTable& _table;

    GameState& operator=(const GameState&) = delete;
};

std::map<GameState::Direction, GameState::Point> GameState::directions = {
    {RIGHT, Point(1, 0)},
    {DOWN, Point(0, 1)},
    {LEFT, Point(-1, 0)},
    {UP, Point(0, -1)}
};