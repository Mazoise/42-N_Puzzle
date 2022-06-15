#include "GameState.hpp"
#include "GameStateBuilder.hpp"
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
    try {
        GameState table = GameStateBuilder::fromFile(argv[1]);
        std::cout << table;
    } catch (GameStateBuilder::ParsingException e) {
        std::cout << "Parsing error : " << e.what() << std::endl;
        return 1;
    } catch (std::exception e) {
        std::cout << "Error : " << e.what() << std::endl;
        return 1;
    }
}
