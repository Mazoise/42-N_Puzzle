#pragma once

#include "GameState.hpp"
#include "Generators.hpp"
#include "RandomTable.hpp"
#include <queue>
#include <set>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <queue>

typedef std::vector<GameState::Direction> Solution;

class Puzzle {
  public:
    Puzzle(heuristic_t heuristic, const std::vector<int>& data):
        _size(std::sqrt(data.size())),
        _table(RandomTable(_size)),
        _initial(data, _size, _table),
        _solution(GameState(Generators::generateSolution(_size), _size, _table)),
        _heuristic(heuristic),
        _total_states(0),
        _max_ressource(0)
    {}

    //A* search, return right sequence of moves for n-puzzle
    Solution solve() {
        std::priority_queue<GameState, std::vector<GameState>, std::greater<GameState> > queue;
        GameState::Point last_move;
        // std::unordered_map<uint64_t, uint64_t> came_from;
        std::unordered_map<uint64_t, size_t> visited;

        if(_initial.isSolvable() != _solution.isSolvable()) { // solution solvability can be precalculated for each size
            std::cout << "Solution is not solvable" << std::endl;
            return Solution();
        }
        // else // tmp
        // {
        //     std::cout << "U";
        //     return Solution();
        // }
        _initial.setHeuristicScore(_heuristic(_initial, _solution));
        queue.push(_initial); //calls copy constructor
        while (!queue.empty()) {
            GameState current = queue.top();
            if (queue.size() + visited.size() > _max_ressource)
                _max_ressource = queue.size() + visited.size();
            queue.pop();
            _total_states++;
            if (current.hash() == _solution.hash()) {
                return current.get_moves();
            }
            visited.insert(std::make_pair(current.hash(), current.getHeuristicScore()));
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
                next.setHeuristicScore(_heuristic(next, _solution) + (next.get_moves().size() + 1) - (next.get_moves().size() + 1) * 0.000001); // tie breaking
                auto already_visited = visited.find(next.hash());
                if (already_visited == visited.end() || already_visited->second > next.getHeuristicScore()) {
                    if (already_visited != visited.end())
                        already_visited->second = next.getHeuristicScore();
                    next.push_move(move.first);
                    queue.push(next);
                    // came_from.insert(std::make_pair(next.hash(), current.hash()));
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
            std::cout << "\x1B[2J\x1B[H";
            std::cout << "Number of moves : " << sol.size() << std::endl;
            std::cout << "Max ressource : " << _max_ressource << std::endl;
            std::cout << "Total states : " << _total_states << std::endl;
            std::cout << current << std::endl;
            getchar();
        }
    }

  private:
    size_t      _size;
    RandomTable _table;
    GameState   _initial;
    GameState   _solution;
    heuristic_t _heuristic;
    size_t      _total_states;
    size_t      _max_ressource;
};
