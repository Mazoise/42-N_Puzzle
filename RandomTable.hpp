#pragma once

#include <vector>
#include <random>
#include <iostream>

class RandomTable {
  public:
    RandomTable(size_t size): _size(size) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
        size_t length = std::pow(_size, 4);
        _table.resize(length);
        for (size_t i = 0; i < _table.size(); ++i)
            _table[i] = dist(gen);
    }

    uint64_t operator()(size_t i, size_t x, size_t y) const {
        // std::cout << "(" << i << ", " << x << ", " << y << ") = " << _table[i * _size * _size + x * _size + y] << std::endl;
        return _table[i * _size * _size + x * _size + y];
    }

    friend std::ostream& operator<<(std::ostream& os, const RandomTable& table) {
        for (size_t i = 0; i < table._table.size(); ++i) {
            os << table._table[i] << std::endl;
        }
        os << "size : " << table._size << " " << table._table.size() << std::endl;
        return os;
    }

  private:
    std::vector<uint64_t> _table;
    size_t _size;
};