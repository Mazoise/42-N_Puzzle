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

        size_t distance(const Point &other) const {
            return std::abs(x - other.x) + std::abs(y - other.y);
        }

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
        _hash = 0;
        for (size_t i = 0; i < _data.size(); ++i)
            if (_data[i] != 0)
                _hash ^= _table(_data[i], i % _size, (size_t)(i / _size));
        _reverseData.resize(_data.size());
        for (size_t i = 0; i < _data.size(); i++) {
            _reverseData[_data[i]] = i;
        }
    }

    virtual ~GameState() {}

    void swap(Direction d) {
        Point zero = getPoint(find(0));
        Point p = zero + directions[d];
        if (!p.in_bounds(_size))
            throw std::out_of_range("Point is out of bounds, swap failed");
        int nb = (*this)[p];
        _hash ^= _table(nb, p.x, p.y);
        _hash ^= _table(nb, zero.x, zero.y);
        std::swap((*this)[zero], (*this)[p]);
        _reverseData[0] = getIndex(p); 
        _reverseData[(*this)[zero]] = getIndex(zero);
    }

    size_t find(size_t value) const {
        return _reverseData[value];
    }

    int manhattanDistance(const GameState &rhs) const { // heuristic nb 1
        int distance = 0;
        if (rhs._size != _size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < _reverseData.size(); i++) { // skip 0 (empty box)
            Point p = getPoint(find(i));
            // std::cout << p.distance(rhs.getPoint(rhs.find(i))) << " ";
            distance += p.distance(rhs.getPoint(rhs.find(i)));
        }
        // std::cout << std::endl;
        return distance;
    }

    int linearConflict(const GameState &rhs) const {  // heuristic nb 2
        int conflict = 0;
        std::vector<bool> right_column(_size * _size, false); // should we store this in the class? would be faster
        std::vector<bool> right_line(_size * _size, false);
        if (rhs._size != _size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < _reverseData.size(); i++) { // skip 0 (empty box)
            Point p = getPoint(find(i));
            if (p.x == rhs.getPoint(rhs.find(i)).x && p.y != rhs.getPoint(rhs.find(i)).y) {
                right_line[find(i)] = true;
            }
            if (p.y == rhs.getPoint(rhs.find(i)).y && p.x != rhs.getPoint(rhs.find(i)).x) { // true if right column AND wrong line
                right_column[find(i)] = true; // index is the index, we don't care about the value in this function
            }
        }
        Point p = getPoint(0);
        Point tmp;
        for (p.y = 0; p.y < _size; p.y++) {
            for (p.x = 0; p.x < _size; p.x++) { // for each box except 0
                if ((*this)[p] == 0)
                    continue;
                if (right_line[getIndex(p)]) {
                    tmp = p;
                    while(tmp.y < _size - 1) {
                        tmp += directions[DOWN];
                        if ((*this)[tmp] != 0 && right_line[getIndex(tmp)]) // find a box that is in it's right line too
                        {
                            // std::cerr << "Vertical linear conflict  between " << (*this)[p] << " and " << (*this)[tmp] << std::endl;
                            if (rhs.getPoint(rhs.find((*this)[p])).y > rhs.getPoint(rhs.find((*this)[tmp])).y) // check if boxes are switched (here we always have p.y > tmp.y so if final_p.y < final_tmp.y then we have a conflict)
                                conflict++;
                        }
                    }
                }
                if (right_column[getIndex(p)]) {
                    tmp = p;
                    while(tmp.x < _size - 1) {
                        tmp += directions[RIGHT];
                        if ((*this)[tmp] != 0 && right_column[getIndex(tmp)])
                        {
                            // std::cerr << "Horizontal linear conflict between " << (*this)[p] << " and " << (*this)[tmp] << std::endl;
                            if (rhs.getPoint(rhs.find((*this)[p])).x > rhs.getPoint(rhs.find((*this)[tmp])).x)
                                conflict++;
                        }
                    }
                }
            }
        }
        return conflict * 2; // each conflict adds at least 2 more steps to solve
    }

    int outOfRowNColumn(const GameState &rhs) const { // heuristic nb 3
        int out = 0;
        if (rhs._size != _size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < _reverseData.size(); i++) { // skip 0 (empty box)
            Point p = getPoint(find(i));
            if (p.x != rhs.getPoint(rhs.find(i)).x && p.y != rhs.getPoint(rhs.find(i)).y)
                out++;
        }
        return out;
    }

    size_t size() const {
        return _size;
    }

    uint64_t hash() const {
        return _hash;
    }

    Point getPoint(size_t index) const {
        return Point(index % _size, (int)(index / _size));
    }

    int getIndex(const Point& p) const {
        return p.y * _size + p.x;
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
        // REVERSE DATA :
        // for (size_t i = 0; i < table._size; i++) {
        //     for (size_t j = 0; j < table._size; j++)
        //         os << std::setfill(' ') << std::setw(4) << table._reverseData[i * table._size + j] << " ";
        //     os << std::endl;
        // }
        return os;
    }

  private:
    std::vector<int> _data;
    std::vector<int> _reverseData; // gives the index of the value in the _data vector, will speed up the heuristic calculations
    size_t _size;
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
