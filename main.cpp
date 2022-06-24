#include "Generators.hpp"
#include "Puzzle.hpp"
#include "GameState.hpp"
#include <string>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    static double (*heuristic[])(const GameState &lhs, const GameState &rhs) = {
                GameState::manhattanDistance,
                GameState::linearConflict,
                GameState::outOfRowNColumn
    }
    if (argc != 2)
    {
        std::cout << "Missing arg" << std::endl;
        return 1;
    }
    std::srand(time(NULL));
    try {
        Puzzle puzzle(Generators::fromFile(argv[1]));
        auto solution = puzzle.solve();
        std::cout << solution.size() << std::endl;
        puzzle.play(solution);
    } catch (Generators::ParsingException e) {
        std::cout << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "Error : " << e.what() << std::endl;
        throw;
        return 1;
    }
}
