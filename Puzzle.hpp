#pragma once

#include "GameState.hpp"
#include "Generators.hpp"
#include "RandomTable.hpp"
#include <queue>
#include <set>
#include <iostream>
#include <vector>

typedef std::vector<GameState::Direction> Solution;

class Puzzle {
  public:
    Puzzle(const std::vector<int>& data):
        _size(std::sqrt(data.size())),
        _table(RandomTable(_size)),
        _initial(data, _size, _table),
        _solution(GameState(Generators::generateSolution(_size), _size, _table))
    {}

    //A* search, return right sequence of moves for n-puzzle
    Solution solve() {
        std::queue<GameState> queue;
        std::set<uint64_t> visited;
        GameState::Point last_move;

        queue.push(_initial.clone());
        while (!queue.empty()) {
            GameState current = queue.front();
            queue.pop();
            if (current.hash() == _solution.hash()) {
                return current.get_moves();
            }
            visited.insert(current.hash());
            for (auto& move : current.directions) {
                if (move.second == last_move * -1) {
                    continue;
                }
                last_move = move.second;
                GameState::Point neighbor = current.neighbor(move.first);
                if (!neighbor.in_bounds(_size))
                    continue;
                GameState next = current.clone();
                next.swap(neighbor);
                if (visited.find(next.hash()) == visited.end()) {
                    next.push_move(move.first);
                    queue.push(next);
                }
            }
        }
        std::cout << "No solution" << std::endl;
        return Solution();
    }

    void play(const Solution& sol) {
        GameState current = _initial.clone();
        for (auto& move : sol) {
            auto neighbor = current.neighbor(move);
            current.swap(neighbor);
            std::cout << current << std::endl;
            getchar();
        }
    }

  private:
    size_t _size;
    RandomTable _table;
    GameState _initial;
    GameState _solution;
};
