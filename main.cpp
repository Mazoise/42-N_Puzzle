#include "Generators.hpp"
#include "Puzzle.hpp"
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char *argv[])
{
    if (argc < 3) // ignoring if more args
    {
        std::cerr << "Missing arguments : first argument should be heuristic options\
 (-h, -l, -m or -n), second should be map file or a size for random map" << std::endl;
        return 1;
    }
    std::srand(time(NULL));
    try {
        Generators gen;
        Puzzle puzzle(gen.setHeuristic(argc, argv), gen.initMap(argv[argc - 1]), gen.generateSolution());
        auto solution = puzzle.solve();
        puzzle.play(solution);
    } catch (Generators::ParsingException e) {
        std::cerr << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::invalid_argument e) {
        std::cerr << "Invalid argument : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cerr << "Error : " << e.what() << std::endl;
        throw;
        return 1;
    }
}
