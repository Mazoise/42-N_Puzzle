#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <map>

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

    GameState(const std::vector<int>& data, size_t size) : _data(data), _size(size) {
        auto i = std::find(_data.begin(), _data.end(), 0);
        size_t index = std::distance(_data.begin(), i);
        _zero = Point(index % _size, (int)(index / _size));
    }

    virtual ~GameState() {}

    void swap(Direction d) { // no bound checks
        auto tmp = (*this)[_zero];
        (*this)[_zero] = (*this)[_zero + directions[d]];
        (*this)[_zero + directions[d]] = tmp;
    }

    size_t size() const {
        return _size;
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
};

std::map<GameState::Direction, GameState::Point> GameState::directions = {
    {RIGHT, Point(1, 0)},
    {DOWN, Point(0, 1)},
    {LEFT, Point(-1, 0)},
    {UP, Point(0, -1)}
};