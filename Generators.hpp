#pragma once
#include "GameState.hpp"
#include <algorithm>
#include <fstream>
#include <cstring>
#include <memory>
#include <utility>
#include "RandomTable.hpp"

typedef std::vector<int> Data;
typedef size_t (*heuristic_f)(const GameState &lhs, const GameState &rhs);
typedef int (*update_heuristic_f)(const GameState &lhs, const GameState &rhs, const GameState::Point &point);

struct heuristic_t {
    heuristic_f full;
    update_heuristic_f update;
};

class Generators {
  public:

    Generators() : _size(0) {}

    class ParsingException : public std::exception {
        public:
            ParsingException(std::string message) : _message(message) {}
            virtual const char* what() const throw() {
                return _message.c_str();
            }
        private:
            std::string _message;
    };

    class FileReader : public std::ifstream {
        public:
            FileReader(std::string filename) : std::ifstream(filename) {}
            ~FileReader() { close(); }
    };

    void skip_whitespace(std::string &line, size_t &pos) {
        while (pos < line.size() && isspace(line[pos]))
            pos++;
    }

    heuristic_t setHeuristic(std::string option)
    {
        heuristic_t heuristic;
        if (option == "-m" || option == "--manhattan")
            heuristic = {.full = &GameState::manhattan,
                         .update = &GameState::updateManhattan};
        else if (option == "-l" || option == "--linearconflict")
            heuristic = {.full = &GameState::linearConflict,
                         .update = &GameState::updateLinearConflict};
        else if (option == "-h" || option == "--hamming")
            heuristic = {.full = &GameState::hamming,
                         .update = &GameState::updateHamming};
        else if (option == "-n" || option == "--none")
            heuristic = {.full = &GameState::noHeuristic,
                         .update = &GameState::updateNoHeuristic};
        else
            throw std::invalid_argument("Invalid heuristic option \"" + option + "\", options are:\n\
 -m or --manhattan\n -l or --linearconflict\n -h or --hamming\n -n or --none");
        return heuristic;
    }

    Data parse_file(std::ifstream &fs)
    {
        std::string line;
        size_t      line_count = 0;
        size_t      column_count;
        size_t      pos;
        int         nb;
        size_t      tmp_count;
        std::vector<bool> filled;
        size_t size = 0;
        Data data;

        while (getline(fs, line))
        {
            pos = 0;
            column_count = 0;
            skip_whitespace(line, pos);
            if (line[pos] == '#' || line.size() == 0)
                continue;
            while (pos < line.size() && line[pos] != '#')
            {
                nb = std::stoi(&line[pos], &tmp_count);
                pos += tmp_count;
                if (nb < 0)
                    throw ParsingException("Negative number in table");
                if (line_count == 0)
                {
                    if (column_count != 0)
                        throw ParsingException("First line should only contain the size of the table");
                    if (nb < 2)
                        throw ParsingException("Table size should be at least 3");
                    size = nb;
                    data.resize(nb * nb);
                    filled = std::vector<bool>(nb * nb, false);
                }
                else
                {
                    if (line_count > size)
                        throw ParsingException("Too many lines in table");
                    if (nb >= (int)(size * size))
                        throw ParsingException("Number too large in table");
                    if (column_count >= size)
                        throw ParsingException("Too many values on line " + std::to_string(line_count) + " of table");
                    data[(line_count - 1) * size + column_count] = nb;
                    if (filled[nb])
                        throw ParsingException("Duplicate number "  + std::to_string(nb) + " in table");
                    filled[nb] = true;
                }
                column_count++;
                skip_whitespace(line, pos);
            }
            if (line_count > 0 && column_count != size)
                throw ParsingException("Missing values on line " + std::to_string(line_count) + " of table");
            line_count++;
        }
        if (line_count == 0 || line_count - 1 < size)
            throw ParsingException("Missing lines in table");
        _size = size;
        return data;
    }

    Data initMap(std::string argument) {
        FileReader fs(argument);
        if (!fs.is_open()) {
            if (errno == 2) // No such file or directory
                try {
                    _size = std::stoi(argument); // Try to parse argument as size of map
                    if (_size <= 2)
                        throw ParsingException("Table size should be at least 3");
                    return generateRandom();
                }
                catch (ParsingException &e) {
                    throw ParsingException(e);
                }
                catch (std::exception &e) { 
                    throw ParsingException("Error opening \"" + argument + "\" : " + std::strerror(2));
                }
            else
                throw ParsingException("Error opening \"" + argument + "\" : " + std::strerror(errno));
        }
        try {
            return parse_file(fs);
        } catch (std::invalid_argument &e) {
            throw ParsingException("Invalid character in table");
        } catch (std::out_of_range &e) {
            throw ParsingException("Number too large in table");
        }
    }

    bool isEmptyBox(std::vector<int> table, const GameState::Point& p) {
        if (!p.in_bounds(_size))
            return false;
        return table[p.y * _size + p.x] == 0;
    }

    Data generateSolution()
    {
        Data data(_size * _size);
        size_t value = 0;
        GameState::Point pos;

        auto dir = GameState::directions.begin();
        while (value < _size * _size - 1)
        {
            data[pos.y * _size + pos.x] = ++value;
            if (!isEmptyBox(data, pos + dir->second)) {
                ++dir;
                if (dir == GameState::directions.end())
                    dir = GameState::directions.begin();
            }
            pos += dir->second;
        }
        return data;
    }

    Data generateRandom(){
        Data data(_size * _size);
        for (size_t i = 0; i < _size * _size; i++)
            data[i] = i;
        std::random_shuffle(data.begin(), data.end());
        return data;
    }

  private:

    size_t _size;
};
