#include "Generators.hpp"
#include "Puzzle.hpp"
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Missing arg" << std::endl;
        return 1;
    }
    std::srand(time(NULL));
    try {
        Puzzle puzzle(Generators::setHeuristic(argv[1]), Generators::fromFile(argv[2]));
        auto solution = puzzle.solve();
        // std::cout << solution.size() << std::endl;
        // puzzle.play(solution);
    } catch (Generators::ParsingException e) {
        std::cout << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "Error : " << e.what() << std::endl;
        throw;
        return 1;
    }
}
