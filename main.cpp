#include "Generators.hpp"
#include "Puzzle.hpp"
#include <string>
#include <iostream>
#include <fstream>

void print_usage() {
    std::cout << "\
Usage : ./n_puzzle [OPTION]... [ARG] \n\
Implementation of the A* algorithm to solve N-puzzles\n\
\n\
ARG is either a size or a path to a map\n\
\n\
Options:\n\
  -m, --manhattan-distance\tmanhattan distance heuristic\n\
  -l, --linear-conflict\t\tmanhattan distance + linear conflict heuristic\n\
  -h, --hamming\t\t\thamming disntance heuristic\n\
  -g, --greedy\t\t\tgreedy search (Not guaranteed to find the shortest solution)\n\
\n\
No option will run the A* with uniform cost search\n\
\n\
Example:\n\
  ./n_puzzle -l 4" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) // ignoring if more args
    {
        print_usage();
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
        std::cerr << e.what() << std::endl;
        print_usage();
        return 1;
    } catch (std::exception e) {
        std::cerr << "Error : " << e.what() << std::endl;
        throw;
        return 1;
    }
}
