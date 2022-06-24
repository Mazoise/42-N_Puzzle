#pragma once
#include "GameState.hpp"
#include <algorithm>
#include <fstream>
#include <cstring>
#include <memory>
#include <utility>
#include "RandomTable.hpp"

typedef std::vector<int> Data;

class Generators {
  public:
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

    static void skip_whitespace(std::string &line, size_t &pos) {
        while (pos < line.size() && isspace(line[pos]))
            pos++;
    }

    static double (*(setHeuristic(std::string option)))(const GameState &lhs, const GameState &rhs)
    {
        if (option == "-mh")
            return &GameState::manhattanDistance;
        else if (option == "-lc")
            return &GameState::linearConflict;
        else if (option == "-rc")
            return &GameState::outOfRowNColumn;
        else if (option == "-nh")
            return &GameState::noHeuristic;
        else
            throw std::invalid_argument("Invalid heuristic option, add -mh, -lc or -rc");
    }

    static Data parse_file(std::ifstream &fs)
    {
        std::string line;
        size_t      line_count = 0;
        size_t      column_count;
        size_t      pos;
        int         nb;
        size_t      tmp_count;
        std::vector<bool> filled;
        size_t size;
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
                        throw ParsingException("Table size should be at least 2");
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
        if (line_count - 1 < size)
            throw ParsingException("Missing lines in table");
        return data;
    }

    static Data fromFile(std::string filename) {
        FileReader fs(filename);
        if (!fs.is_open()) {
            throw ParsingException("Error opening " + filename + " : " + std::strerror(errno));
        }
        try {
            return parse_file(fs);
        } catch (std::invalid_argument &e) {
            throw ParsingException("Invalid character in table");
        } catch (std::out_of_range &e) {
            throw ParsingException("Number too large in table");
        }
    }

    static bool isEmptyBox(std::vector<int> table, size_t size, const GameState::Point& p) {
        if (!p.in_bounds(size))
            return false;
        return table[p.y * size + p.x] == 0;
    }

    static Data generateSolution(size_t size)
    {
        Data data(size * size);
        size_t value = 0;
        GameState::Point pos;

        auto dir = GameState::directions.begin();
        while (value < size * size - 1)
        {
            data[pos.y * size + pos.x] = ++value;
            if (!isEmptyBox(data, size, pos + dir->second)) {
                ++dir;
                if (dir == GameState::directions.end())
                    dir = GameState::directions.begin();
            }
            pos += dir->second;
        }
        return data;
    }

    static Data generateRandom(size_t size){
        Data data(size * size);
        for (size_t i = 0; i < size * size; i++)
            data[i] = i;
        std::random_shuffle(data.begin(), data.end());
        return data;
    }

};