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

    preprocessor.cpp: Program entry point

*/


#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <tuple>
#include <filesystem>
#include <fstream>
#include <errno.h>
#include <string.h>

#include "config.hpp"
#include "logging.hpp"

#include "preprocessor.hpp"

std::string stripbegend(std::string str) {
    if (str == "") return "";
    auto beg = str.find_first_not_of(" \t");
    auto end = str.find(" \t", beg);
    return str.substr(beg, end - beg);
}

std::vector<line_t> preprocess(const std::string& filename) {
    std::vector<line_t> preprocessed;
    std::vector<std::tuple<std::string, std::string>> defines;

    // open and read file
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        std::cout << TERM_RED "Error opening " << filename << ": " TERM_RESET << strerror(errno) << std::endl;
        return std::vector<line_t>(); // return empty
    }
    std::stringstream ss;
    ss << fs.rdbuf();
    fs.close();
    std::cout << "Preprocessing " << filename << std::endl;

    // read line by line
    std::string line;
    int linen = 0;
    while (std::getline(ss, line)) {
        auto start = line.find_first_not_of(" \t");

        // strip line comments
        size_t linecommpos = 0;
        if ((linecommpos = line.find("//")) != std::string::npos) {
            line = line.substr(0, linecommpos);
        }
        
        // get first token
        std::stringstream liness(line);
        std::string firsttok;
        liness >> firsttok;

        if (firsttok[0] == '#') {
            if (firsttok == "#") {
                liness >> firsttok;
                firsttok.insert(firsttok.begin(), '#');
            }
            // Preprocessor directive
            //LOG_DEBUG << "Preprocessor directive: " << firststat << std::endl;

            if (firsttok == "#define") {
                // #define CNAME expression
                //        ^ space required
                std::string cname, expression;
                liness >> cname;
                std::getline(liness, expression);
                expression = stripbegend(expression);

                defines.push_back(std::tuple<std::string, std::string>(cname, expression));

                LOG_DEBUG << "defined " << cname << " as " << expression << std::endl;
            } else if (firsttok.substr(0, 8) == "#include") {
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
                    LOG_ERROR << "invalid include type" << std::endl;
                    continue;
                }

                size_t argend = 0;
                if (relative) {
                    if ((argend = line.find("\"", argbeg + 1)) == std::string::npos) {
                        LOG_ERROR << "missing closing character" << std::endl;
                        continue;
                    }
                }
                else {
                    if ((argend = line.find(">", argbeg + 1)) == std::string::npos) {
                        LOG_ERROR << "missing closing character" << std::endl;
                        continue;
                    }
                }

                std::string arg = line.substr(argbeg + 1, argend - argbeg - 1);

                auto relative_pwd = std::filesystem::path(filename).remove_filename().string();

                std::vector<std::string> include_paths = {INCLUDE_PATHS};
                if (relative) arg = relative_pwd + arg;
                else {
                    for (auto& p : include_paths) {
                        if (std::filesystem::exists(p + arg)) {
                            arg = p + arg;
                            break;
                        }
                    }
                }

                std::filesystem::path path(arg);

                auto include_preprocessed = preprocess(path.string());
                preprocessed.insert(preprocessed.end(), include_preprocessed.begin(), include_preprocessed.end());

                LOG_DEBUG << "included " << path.string() << std::endl;

            } else {
                LOG_ERROR << "unrecognised preprocessor directive " << firsttok << std::endl;
                continue;
            }
        } else {
            preprocessed.push_back(line_t{filename, linen, line});
        }

        linen++;
    }

    return preprocessed;
}
