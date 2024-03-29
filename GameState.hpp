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

        Point(Point const& other) : x(other.x), y(other.y) {}

        Point & operator=(Point const &rhs) {
            x = rhs.x;
            y = rhs.y;
            return *this;
        }

        bool operator==(const Point& other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Point& other) const {
            return !(*this == other);
        }

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

        Point operator*(int other) const {
            return Point(x * other, y * other);
        }

        Point operator/(int other) const {
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

    GameState(const std::vector<int>& data, size_t size, const RandomTable &table) : _data(data), _size(size), _table(table), _depth(0) {
        _hash = 0;
        for (size_t i = 0; i < _data.size(); ++i)
            if (_data[i] != 0)
                _hash ^= _table(_data[i], i % _size, (size_t)(i / _size));
        _reverseData.resize(_data.size());
        for (size_t i = 0; i < _data.size(); i++) {
            _reverseData[_data[i]] = i;
        }
        _zero = getPoint(find(0));
        // std::cout << "constructor" << std::endl;
    }

    GameState(const GameState& rhs): _table(rhs._table) {
        _data = rhs._data;
        _size = rhs._size;
        _hash = rhs._hash;
        _reverseData = rhs._reverseData;
        _zero = rhs._zero;
        _depth = rhs._depth;
        _heuristicScore = rhs._heuristicScore;
        _redundantMove = rhs._redundantMove;
        // std::cout << "copy constructor" << std::endl;
    }

    GameState(GameState&& rhs): _table(rhs._table) {
        _data = std::move(rhs._data);
        _reverseData = std::move(rhs._reverseData);
        _size = rhs._size;
        _hash = rhs._hash;
        _zero = rhs._zero;
        _depth = rhs._depth;
        _heuristicScore = rhs._heuristicScore;
        _redundantMove = rhs._redundantMove;
        // std::cout << "move constructor" << std::endl;
    }

    virtual ~GameState() {}

    GameState& operator=(GameState&& rhs)
    {
        _data = std::move(rhs._data);
        _reverseData = std::move(rhs._reverseData);
        _size = rhs._size;
        _hash = rhs._hash;
        _zero = rhs._zero;
        _depth = rhs._depth;
        _heuristicScore = rhs._heuristicScore;
        _redundantMove = rhs._redundantMove;
        // std::cout << "move operator" << std::endl;
        return *this;
    }

    Point neighbor(Direction d) const {
        return _zero + directions[d];
    }

    void swap(Point &neighbor) {
        int nb = (*this)[neighbor];
        _hash ^= _table(nb, neighbor.x, neighbor.y);
        _hash ^= _table(nb, _zero.x, _zero.y);
        std::swap((*this)[_zero], (*this)[neighbor]);
        _reverseData[0] = getIndex(neighbor); 
        _reverseData[(*this)[_zero]] = getIndex(_zero);
        _zero = neighbor;
        ++_depth;
    }

    size_t find(size_t value) const {
        return _reverseData[value];
    }

    void setHeuristicScore(size_t score) {
        _heuristicScore = score;
    }

    size_t getHeuristicScore() const {
        return _heuristicScore;
    }

    size_t getDepth() const {
        return _depth;
    }

    static size_t noHeuristic(const GameState &, const GameState &) {
        return 0;
    }

    static int updateNoHeuristic(const GameState &, const GameState &, const Point &) {
        return 0;
    }

    static size_t manhattan(const GameState &lhs, const GameState &rhs) { // heuristic nb 1
        size_t distance = 0;
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            distance += p.distance(rhs.getPoint(rhs.find(i)));
        }
        return distance;
    }

    static int updateManhattan(const GameState &lhs, const GameState &rhs, const Point &neighbor) { 
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        Point dest = rhs.getPoint(rhs.find(lhs[neighbor]));
        Point p = lhs._zero;
        return p.distance(dest) - neighbor.distance(dest);
    }

    static size_t countInversions(const GameState &lhs, const GameState &rhs) {
        size_t conflict = 0;
        std::vector<bool> right_column(lhs._size * lhs._size, false);
        std::vector<bool> right_row(lhs._size * lhs._size, false);

        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            if (p.x == rhs.getPoint(rhs.find(i)).x) {
                right_row[lhs.find(i)] = true;
            }
            if (p.y == rhs.getPoint(rhs.find(i)).y) { // true if right column AND wrong line
                right_column[lhs.find(i)] = true; // index is the index, we don't care about the value in this function
            }
        }
        Point p = lhs.getPoint(0);
        Point tmp;
        for (p.y = 0; (size_t)p.y < lhs._size; p.y++) {
            for (p.x = 0; (size_t)p.x < lhs._size; p.x++) { // for each box except 0
                if (lhs[p] == 0)
                    continue;
                if (right_row[lhs.getIndex(p)]) {
                    tmp = p;
                    while((size_t)tmp.y < lhs._size - 1) {
                        tmp += directions[DOWN];
                        if (lhs[tmp] != 0 && right_row[lhs.getIndex(tmp)]) // find a box that is in it's right line too
                            conflict += rhs.getPoint(rhs.find(lhs[p])).y > rhs.getPoint(rhs.find(lhs[tmp])).y; // check if boxes are switched (here we always have p.y > tmp.y so if final_p.y < final_tmp.y then we have a conflict)
                    }
                }
                if (right_column[lhs.getIndex(p)]) {
                    tmp = p;
                    while((size_t)tmp.x < lhs._size - 1) {
                        tmp += directions[RIGHT];
                        if (lhs[tmp] != 0 && right_column[lhs.getIndex(tmp)])
                            conflict += rhs.getPoint(rhs.find(lhs[p])).x > rhs.getPoint(rhs.find(lhs[tmp])).x;
                    }
                }
            }
        }
        return conflict;
    }

    static int countColumnInversions(const GameState &lhs, const GameState &rhs, const Point &point, const int value) {
        size_t  conflict = 0;
        Point   other;
        Point   other_dest;
        Point   dest = rhs.getPoint(rhs.find(value));

        if (point.x == dest.x) {
            for (other = Point(point.x, 0); other.y < (int)lhs._size; other += directions[DOWN]) {
                other_dest = rhs.getPoint(rhs.find(lhs[other]));
                if (lhs[other] != value && lhs[other] != 0
                    && other.x == other_dest.x) {
                    if (other.y > point.y)
                        conflict += dest.y > other_dest.y;
                    else
                        conflict += dest.y < other_dest.y;
                }
            }
        }
        return conflict;
    }

    static int countRowInversions(const GameState &lhs, const GameState &rhs, const Point &point, const int value)
    {
        size_t conflict = 0;
        Point   other;
        Point   other_dest;
        Point dest = rhs.getPoint(rhs.find(value));

        if (point.y == dest.y) {
            for (other = Point(0, point.y);
                 other.x < (int)lhs._size;
                 other += directions[RIGHT]) {
                other_dest = rhs.getPoint(rhs.find(lhs[other]));
                if (lhs[other] != value && lhs[other] != 0
                && other.y == other_dest.y) {
                    if (other.x > point.x)
                        conflict += dest.x > other_dest.x;
                    else
                        conflict += dest.x < other_dest.x;
                }
            }
        }
        return conflict;
    }

    static int updateInversions(const GameState &lhs, const GameState &rhs, const Point &neighbor) {
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        if (neighbor.x == lhs._zero.x)
            return countRowInversions(lhs, rhs, lhs._zero, lhs[neighbor])
                 - countRowInversions(lhs, rhs, neighbor, lhs[neighbor]);
        else
            return countColumnInversions(lhs, rhs, lhs._zero, lhs[neighbor])
                 - countColumnInversions(lhs, rhs, neighbor, lhs[neighbor]);
    }

    bool isSolvable() {
        int conflict = 0;
        bool solvable_solution = ((_size - 2) / 4) % 2;

        for (size_t i = 1; i < _reverseData.size() - 1; i++) { // skip 0 (empty box)
            for (size_t j = i + 1; j < _reverseData.size(); j++) {
                if (_reverseData[i] > _reverseData[j])
                    conflict++;
            }
        }
        if (_size % 2 || (_size - _zero.y) % 2 == 0)
            return conflict % 2 != solvable_solution;
        else
            return conflict % 2 == solvable_solution;
    }

    static size_t linearConflict(const GameState &lhs, const GameState &rhs) {  // heuristic nb 2
        return  manhattan(lhs, rhs) + countInversions(lhs, rhs) * 2;
    }

    static int updateLinearConflict(const GameState &lhs, const GameState &rhs, const Point &neighbor) {
        return (updateManhattan(lhs, rhs, neighbor) + updateInversions(lhs, rhs, neighbor) * 2);
    }

    static size_t hamming(const GameState &lhs, const GameState &rhs) { // heuristic nb 3
        size_t out = 0;
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            if (p.x != rhs.getPoint(rhs.find(i)).x && p.y != rhs.getPoint(rhs.find(i)).y)
                out++;
        }
        return out;
    }

    static int updateHamming(const GameState &lhs, const GameState &rhs, const Point &neighbor) {
        Point dest = rhs.getPoint(rhs.find(lhs[neighbor]));
        return (lhs._zero.x != dest.x && lhs._zero.y != dest.y)
             - (neighbor.x != dest.x && neighbor.y != dest.y);
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

    void setRedondant(const Point& p) {
        _redundantMove = p;
    }

    bool isRedundant(const Point& p) {
        return p == _redundantMove;
    }

    int operator[](Point p) const {
        return _data[p.x + p.y * _size];
    }

    int& operator[](Point p) {
        return _data[p.x + p.y * _size];
    }

    friend bool operator>(const GameState &lhs, const GameState &rhs) {
        size_t f_l = lhs._heuristicScore + lhs._depth * _g;
        size_t f_r = rhs._heuristicScore + rhs._depth * _g;
        if (f_l == f_r)
            return lhs._heuristicScore > rhs._heuristicScore;
        return f_l > f_r;
    }

    friend bool operator==(const GameState &lhs, const GameState &rhs) {
        return lhs._hash == rhs._hash;
    }

    friend bool operator!=(const GameState &lhs, const GameState &rhs) {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const GameState& table) {
        for (size_t i = 0; i < table._size; i++) {
            for (size_t j = 0; j < table._size; j++)
                if (table._data[i * table._size + j])
                    os << std::setfill(' ') << std::setw(4) << table._data[i * table._size + j] << " ";
                else
                    os << std::setfill(' ') << std::setw(4) << " " << " ";
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
    static bool             _g;

  private:
    std::vector<int>        _data;
    std::vector<int>        _reverseData; // gives the index of the value in the _data vector, will speed up the heuristic calculations
    size_t                  _size;
    uint64_t                _hash;
    Point                   _zero;
    const RandomTable&      _table;
    size_t                  _depth;
    size_t                  _heuristicScore;
    Point                   _redundantMove;

    GameState& operator=(const GameState&) = delete;
};

std::map<GameState::Direction, GameState::Point> GameState::directions = {
    {RIGHT, Point(1, 0)},
    {DOWN, Point(0, 1)},
    {LEFT, Point(-1, 0)},
    {UP, Point(0, -1)}
};

bool GameState::_g = true;
