#include "GameState.hpp"
#include "Generators.hpp"
#include "RandomTable.hpp"
#include <string>
#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Missing arg" << std::endl;
        return 1;
    }
    std::srand(time(NULL));
    try {
        std::vector<int> data = Generators::fromFile(argv[1]);
        size_t size = std::sqrt(data.size());
        RandomTable table = RandomTable(size);
        GameState state(data, size, table);
        std::cout << state.hash() << std::endl;
        state.swap(GameState::DOWN);
        std::cout << state.hash() << std::endl;
        state.swap(GameState::UP);
        std::cout << state.hash() << std::endl;
        std::vector<int> solution_data = Generators::generateSolution(state.size());
        GameState solution = GameState(solution_data, state.size(), table);
        std::cout << "Manhattan distance: " << state.manhattanDistance(solution) << std::endl;
        std::cout << "Linear conflicts: " << state.linearConflict(solution) << std::endl;
        std::cout << "Out of row and column: " << state.outOfRowNColumn(solution) << std::endl;
    } catch (Generators::ParsingException e) {
        std::cout << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "Error : " << e.what() << std::endl;
        return 1;
    }
}
