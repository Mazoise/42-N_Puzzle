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
#include <list>

typedef std::list<GameState::Direction> Solution;

class Puzzle {
  public:
    Puzzle(heuristic_t heuristic, const Data& data, const Data& solution):
        _size(std::sqrt(data.size())),
        _table(RandomTable(_size)),
        _initial(data, _size, _table),
        _solution(solution, _size, _table),
        _heuristic(heuristic),
        _total_states(0),
        _max_ressource(0)
    {}

    Solution solve() {
        std::priority_queue<GameState, std::vector<GameState>, std::greater<GameState> > queue;
        GameState::Point last_move;
        std::unordered_map< uint64_t, size_t > visited;

        if(!_initial.isSolvable()) {
            std::cout << _initial << "\nPuzzle is not solvable" << std::endl;
            return Solution();
        }
        _initial.setHeuristicScore(_heuristic.full(_initial, _solution));
        queue.push(_initial); //calls copy constructor
        while (!queue.empty()) {
            GameState current(std::move(const_cast<GameState&>(queue.top()))); // hack to move out of priority_queue, safe because we pop just after
            queue.pop();
            if (current.hash() == _solution.hash()) {
                return constructSolution();
            }
            visited.insert(std::make_pair(current.hash(), current.getDepth()));
            for (auto& move : current.directions) {
                GameState::Point neighbor = current.neighbor(move.first);
                if (!neighbor.in_bounds(_size))
                    continue;
                GameState next(current);
                next.setHeuristicScore(next.getHeuristicScore() + _heuristic.update(next, _solution, neighbor));
                next.swap(neighbor);
                // next.setHeuristicScore(_heuristic.full(next, _solution)); // old version of heuristic, not used anymore (not opti)
                // std::cerr << next.getHeuristicScore() << std::endl;
                auto already_visited = visited.find(next.hash());
                if (already_visited == visited.end() || already_visited->second > next.getDepth() + 1) {
                    if (already_visited != visited.end()) {
                        already_visited->second = next.getDepth() + 1;
                        _came_from[next.hash()] = move.first;
                    }
                    else
                        _came_from.insert({next.hash(), move.first});
                    queue.push(std::move(next));
                }
            }
            if (queue.size() + visited.size() > _max_ressource)
                _max_ressource = queue.size() + visited.size();
            _total_states++;
        }
        std::cout << _initial << "\nPuzzle is not solvable" << std::endl;
        return Solution();
    }

    Solution constructSolution() {
        GameState current(_solution);
        std::unordered_map<GameState::Direction, GameState::Direction> reverse = {
            {GameState::LEFT, GameState::RIGHT},
            {GameState::RIGHT, GameState::LEFT},
            {GameState::UP, GameState::DOWN},
            {GameState::DOWN, GameState::UP}
        };
        Solution solution;

        while (current != _initial) {
            GameState::Direction r = reverse[_came_from[current.hash()]];
            GameState::Point neighbor = current.neighbor(r);
            current.swap(neighbor);
            solution.push_front(r);
        }
        return solution;
    }

    void play(const Solution& sol) {
        GameState current(_initial);

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
    size_t              _size;
    RandomTable         _table;
    GameState           _initial;
    GameState           _solution;
    heuristic_t         _heuristic;
    size_t              _total_states;
    size_t              _max_ressource;
    std::unordered_map<uint64_t, GameState::Direction> _came_from;
};
