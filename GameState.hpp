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

    GameState(const std::vector<int>& data, size_t size, const RandomTable &table) : _data(data), _size(size), _table(table) {
        _hash = 0;
        for (size_t i = 0; i < _data.size(); ++i)
            if (_data[i] != 0)
                _hash ^= _table(_data[i], i % _size, (size_t)(i / _size));
        _reverseData.resize(_data.size());
        for (size_t i = 0; i < _data.size(); i++) {
            _reverseData[_data[i]] = i;
        }
        _zero = getPoint(find(0));
    }

    virtual ~GameState() {}

    GameState& operator=(const GameState& rhs)
    {
        _data = rhs._data;
        _size = rhs._size;
        _hash = rhs._hash;
        _reverseData = rhs._reverseData;
        _zero = rhs._zero;
        _moves = rhs._moves;
        _heuristicScore = rhs._heuristicScore;
        return *this;
    }

    GameState clone() const {
        auto g = GameState(_data, _size, _table);
        g._moves = _moves;
        g._heuristicScore = _heuristicScore;
        return g;
    }

    Point neighbor(Direction d) {
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
    }

    size_t find(size_t value) const {
        return _reverseData[value];
    }

    // static size_t find(const GameState &rhs, size_t value) {
    //     return rhs._reverseData[value];
    // }

    void setHeuristicScore(double score) {
        _heuristicScore = score;
    }

    double getHeuristicScore() const {
        return _heuristicScore;
    }

    static double noHeuristic(const GameState &lhs, const GameState &rhs) {
        return 0;
    }

    static double manhattanDistance(const GameState &lhs, const GameState &rhs) { // heuristic nb 1
        double distance = 0;
        // std::cout << "Heuristic is Manhattan" << std::endl;
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            // std::cout << p.distance(rhs.getPoint(rhs.find(i))) << " ";
            distance += p.distance(rhs.getPoint(rhs.find(i)));
        }
        // std::cout << std::endl;
        return distance;
    }

    static double linearConflict(const GameState &lhs, const GameState &rhs) {  // heuristic nb 2
        double conflict = manhattanDistance(lhs, rhs);
        // std::cout << "Heuristic is Linear conflict" << std::endl;
        std::vector<bool> right_column(lhs._size * lhs._size, false); // should we store this in the class? would be faster
        std::vector<bool> right_line(lhs._size * lhs._size, false);
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            if (p.x == rhs.getPoint(rhs.find(i)).x && p.y != rhs.getPoint(rhs.find(i)).y) {
                right_line[lhs.find(i)] = true;
            }
            if (p.y == rhs.getPoint(rhs.find(i)).y && p.x != rhs.getPoint(rhs.find(i)).x) { // true if right column AND wrong line
                right_column[lhs.find(i)] = true; // index is the index, we don't care about the value in this function
            }
        }
        Point p = lhs.getPoint(0);
        Point tmp;
        for (p.y = 0; (size_t)p.y < lhs._size; p.y++) {
            for (p.x = 0; (size_t)p.x < lhs._size; p.x++) { // for each box except 0
                if (lhs[p] == 0)
                    continue;
                if (right_line[lhs.getIndex(p)]) {
                    tmp = p;
                    while((size_t)tmp.y < lhs._size - 1) {
                        tmp += directions[DOWN];
                        if (lhs[tmp] != 0 && right_line[lhs.getIndex(tmp)]) // find a box that is in it's right line too
                        {
                            // std::cerr << "Vertical linear conflict  between " << lhs[p] << " and " << lhs[tmp] << std::endl;
                            if (rhs.getPoint(rhs.find(lhs[p])).y > rhs.getPoint(rhs.find(lhs[tmp])).y) // check if boxes are switched (here we always have p.y > tmp.y so if final_p.y < final_tmp.y then we have a conflict)
                                conflict += 2;
                        }
                    }
                }
                if (right_column[lhs.getIndex(p)]) {
                    tmp = p;
                    while((size_t)tmp.x < lhs._size - 1) {
                        tmp += directions[RIGHT];
                        if (lhs[tmp] != 0 && right_column[lhs.getIndex(tmp)])
                        {
                            // std::cerr << "Horizontal linear conflict between " << lhs[p] << " and " << lhs[tmp] << std::endl;
                            if (rhs.getPoint(rhs.find(lhs[p])).x > rhs.getPoint(rhs.find(lhs[tmp])).x)
                                conflict += 2;
                        }
                    }
                }
            }
        }
        return conflict; // each conflict adds at least 2 more steps to solve
    }

    static double outOfRowNColumn(const GameState &lhs, const GameState &rhs) { // heuristic nb 3
        double out = 0;
        // std::cout << "Heuristic is Out of Row/Column" << std::endl;
        if (rhs._size != lhs._size)
            throw std::invalid_argument("GameStates have different size");
        for (size_t i = 1; i < lhs._reverseData.size(); i++) { // skip 0 (empty box)
            Point p = lhs.getPoint(lhs.find(i));
            if (p.x != rhs.getPoint(rhs.find(i)).x && p.y != rhs.getPoint(rhs.find(i)).y)
                out++;
        }
        return out;
    }

    void push_move(Direction d) {
        _moves.push_back(d);
    }

    std::vector<Direction> get_moves() const {
        return _moves;
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

    // static Point getPoint(size_t index, size_t size) {
    //     return Point(index % size, (int)(index / size));
    // }

    int getIndex(const Point& p) const {
        return p.y * _size + p.x;
    }

    // static int getIndex(const Point& p, size_t size) {
    //     return p.y * size + p.x;
    // }

    int operator[](Point p) const {
        return _data[p.x + p.y * _size];
    }

    int& operator[](Point p) {
        return _data[p.x + p.y * _size];
    }

    friend bool operator>(const GameState &lhs, const GameState &rhs) {
        return lhs._heuristicScore > rhs._heuristicScore;
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
    Point _zero;
    std::vector<Direction> _moves;
    const RandomTable& _table;
    double          _heuristicScore;

    // GameState& operator=(const GameState&) = delete;
};

std::map<GameState::Direction, GameState::Point> GameState::directions = {
    {RIGHT, Point(1, 0)},
    {DOWN, Point(0, 1)},
    {LEFT, Point(-1, 0)},
    {UP, Point(0, -1)}
};
