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
#include <vector>
#include <tuple>
#include <filesystem>

#include "config.hpp"

#include "preprocessor.hpp"

std::string stripbegend(std::string str) {
    auto beg = str.find_first_not_of(" \t");
    auto end = str.find(" \t", beg);
    return str.substr(beg, end - beg);
}

std::string preprocess(std::stringstream& ss) {
    std::vector<line_t> preprocessed;

    std::vector<std::tuple<std::string, std::string>> defines;

    std::string line;
    int linen = 0;
    while (std::getline(ss, line)) {
        auto start = line.find_first_not_of(" \t");
        
        std::stringstream liness(line);
        std::string firststat;
        liness >> firststat;

        if (firststat[0] == '#') {
            if (firststat == "#") {
                liness >> firststat;
                firststat.insert(firststat.begin(), '#');
            }
            // Preprocessor directive
            std::cout << "Line " << linen << ": Preprocessor directive: "
                << firststat << std::endl;

            if (firststat == "#define") {
                // #define CNAME expression
                //        ^ space required
                std::string cname, expression;
                liness >> cname;
                std::getline(liness, expression);
                expression = stripbegend(expression);

                defines.push_back(std::tuple<std::string, std::string>(cname, expression));

                std::cout << "Defined " << cname << " as " << expression << std::endl;
            } else if (firststat.substr(0, 8) == "#include") {
                // #include <stuff>
                // space not required
                bool relative = true;
                auto dirbeg = line.find('#');
                auto argbeg = line.find("\"", dirbeg);
                if (argbeg == std::string::npos) {
                    argbeg = line.find("<", dirbeg);
                    relative = false;
                }
                if (argbeg == std::string::npos) {
                    std::cout << "Line " << linen << ": Error: invalid include type" << std::endl;
                    continue;
                }

                size_t argend = 0;
                if (relative) {
                    if ((argend = line.find("\"", argbeg + 1)) == std::string::npos) {
                        std::cout << "Line " << linen << ": Error: missing closing character" << std::endl;
                        continue;
                    }
                }
                else {
                    if ((argend = line.find(">", argbeg + 1)) == std::string::npos) {
                        std::cout << "Line " << linen << ": Error: missing closing character" << std::endl;
                        continue;
                    }
                }

                std::string arg = line.substr(argbeg + 1, argend - argbeg - 1);

                
                if (!relative) arg = INCLUDE_PATH + arg;

                std::filesystem::path path(arg);

                std::cout << "Included " << path.string() << std::endl;

            } else {
                std::cout << "Line " << linen << ": Error: unrecognised preprocessor directive " << firststat << std::endl;
            }
        } else {
            preprocessed.push_back(line_t{linen, line});
        }

        linen++;
    }
}
