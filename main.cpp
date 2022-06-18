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
        std::vector<int> data = Generators::generateSolution(3);
        size_t size = std::sqrt(data.size());
        RandomTable table = RandomTable(size);
        GameState state(data, size, table);
        std::cout << state.hash() << std::endl;
        state.swap(GameState::DOWN);
        std::cout << state.hash() << std::endl;
        state.swap(GameState::UP);
        std::cout << state.hash() << std::endl;
    } catch (Generators::ParsingException e) {
        std::cout << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "Error : " << e.what() << std::endl;
        return 1;
    }
}
