#pragma once

#include "GameState.hpp"
#include "Generators.hpp"
#include "RandomTable.hpp"
#include <queue>
#include <set>

class Puzzle {
  public:
    Puzzle(const std::vector<int>& data):
        _size(std::sqrt(data.size())),
        _table(RandomTable(_size)),
        _state(GameState(data, _size, _table)),
        _solution(GameState(Generators::generateSolution(_size), _size, _table))
    {}

    //A* search, return right sequence of moves for n-puzzle
    void solve() {
        std::queue<GameState> queue;
        std::set<uint64_t> visited;
        queue.push(_state);
        while (!queue.empty()) {
            GameState current = queue.front();
            queue.pop();
            if (current.hash() == _solution.hash()) {;
                std::cout << current << std::endl;
                return;
            }
            visited.insert(current.hash());
            for (auto& move : current.directions) {
                GameState::Point neighbor = current.neighbor(move.first);
                if (!neighbor.in_bounds(_size))
                    continue;
                GameState next = current.clone();
                next.swap(neighbor);
                if (visited.find(next.hash()) == visited.end()) {
                    queue.push(next);
                }
            }
        }
        std::cout << "No solution" << std::endl;
    }

  private:
    size_t _size;
    RandomTable _table;
    GameState _state;
    GameState _solution;
};