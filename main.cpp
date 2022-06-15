#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cerrno>
#include <cstring>
#include <iomanip>

class Table {
    public:
        Table() : _data(std::vector<int>()), _size(0) {}
        ~Table() {
            _data.clear();
        }

        class ParsingException : public std::exception {
            public:
                ParsingException(std::string message) : _message(message) {}
                virtual const char* what() const throw() {
                    return _message.c_str();
                }
            private:
                std::string _message;
        };

        void skip_whitespace(std::string &line, size_t &pos) {
            while (pos < line.size() && isspace(line[pos]))
                pos++;
        }

        void parse_file(std::fstream &fs)
        {
            std::string line;
            size_t      line_count = 0;
            size_t      column_count;
            size_t      pos;
            int         nb;
            size_t      tmp_count;
            std::vector<bool> filled;

            while (getline(fs, line))
            {
                pos = 0;
                column_count = 0;
                skip_whitespace(line, pos);
                if (line_count > _size + 1)
                    throw ParsingException("Too many lines in table");
                if (line[pos] == '#' || line.size() == 0)
                    continue;
                while (pos < line.size())
                {
                    try {
                        nb = std::stoi(&line[pos], &tmp_count);
                        pos += tmp_count;
                        if (nb < 0)
                            throw ParsingException("Negative number in table");
                    } catch (std::invalid_argument &e) {
                        throw ParsingException("Invalid character in table");
                    }
                    catch (std::out_of_range &e) {
                        throw ParsingException("Number too large in table");
                    }
                    if (line_count == 0)
                    {
                        if (column_count != 0)
                            throw ParsingException("First line should only contain the size of the table");
                        _size = nb;
                        _data.resize(nb * nb);
                        filled = std::vector<bool>(nb * nb, false);
                    }
                    else
                    {
                        if (nb > _size * _size)
                            throw ParsingException("Number too large in table");
                        if (column_count >= _size)
                            throw ParsingException("Too many numbers on line " + std::to_string(line_count - 1) + " of table");
                        _data[(line_count - 1) * _size + column_count] = nb;
                        if (filled[nb])
                            throw ParsingException("Duplicate number in table " + std::to_string(nb));
                        filled[nb] = true;
                    }
                    column_count++;
                    skip_whitespace(line, pos);
                }
                line_count++;
            }
        }

    friend std::ostream& operator<<(std::ostream& os, const Table& table) {
        for (size_t i = 0; i < table._size; i++) {
            for (size_t j = 0; j < table._size; j++)
                os << std::setfill(' ') << std::setw(4) << table._data[i * table._size + j] << " ";
            if (i != table._size - 1)
                os << std::endl;
        }
        return os;
    }

    private:
        std::vector<int>    _data;
        size_t              _size;
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << "Missing arg" << std::endl;
        return 1;
    }
    std::fstream fs;
    Table table;
    fs.open(argv[1], std::ios::in);
    if (!fs.is_open())
    {
        std::cerr << "Error opening " << argv[1] << " : " << std::strerror(errno) << std::endl;
        return 1;
    }
    try {
        table.parse_file(fs);
    } catch (Table::ParsingException e) {
        std::cerr << "Parsing error : " << e.what() << std::endl;
        fs.close();
        return 1;
    }
    fs.close();
    std::cout << table << std::endl;
}
