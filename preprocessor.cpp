/*

    arfcc: Yeah, everyone needs to code a compiler right?
    Copyright (C) 2023 arf20 (Ángel Ruiz Fernandez)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    main.c: Program entry point

*/


#include <string>
#include <sstream>
#include <algorithm>

#include "preprocessor.hpp"

std::string preprocess(std::stringstream& ss) {
    std::string line;
    int linen = 0;
    while (std::getline(ss, line)) {
        auto start = line.find_first_not_of(" \t");
        if (line[start] == '#') {
            // Preprocessor directive
            auto dstart = find_if(line.begin() + start + 1, line.end(),
                isalpha) - line.begin();

            auto dend = std::find_if_not(line.begin() + dstart + 1, line.end(),
                isalpha) - line.begin();

            std::string directive = line.substr(dstart, dend - start - 1);

            std::string args = line.substr(dend);

            std::cout << "Line " << linen << ": Preprocessor directive: " << directive << args << std::endl;
        }

        linen++;
    }
}
