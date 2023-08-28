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
#include <deque>
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

std::vector<line_t> preprocess(std::string filename) {
    std::vector<line_t> preprocessed;
    std::vector<std::tuple<std::string, std::string>> defines;

    std::deque<line_t> unprocessed;

    // open and read file
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        std::cout << TERM_RED "Error opening " << filename << ": " TERM_RESET << strerror(errno) << std::endl;
        return std::vector<line_t>(); // return empty
    }
    std::stringstream ss;
    ss << fs.rdbuf();
    fs.close();

    std::string linestr;
    size_t linen = 0;
    while (std::getline(ss, linestr)) {
        unprocessed.push_back(line_t{filename, linen, linestr});
        linen++;
    }

    std::cout << "Preprocessing " << filename << std::endl;

    // read line by line
    while (unprocessed.size()) {
        line_t line = unprocessed[0];
        linestr = line.str;
        linen = line.linen;
        filename = line.file;

        auto start = linestr.find_first_not_of(" \t");

        // strip line comments
        size_t linecommpos = 0;
        if ((linecommpos = linestr.find("//")) != std::string::npos) {
            linestr = linestr.substr(0, linecommpos);
        }
        
        // get first token
        std::stringstream liness(linestr);
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
                auto dirbeg = linestr.find('#');
                auto argbeg = linestr.find("\"", dirbeg);
                if (argbeg == std::string::npos) {
                    argbeg = linestr.find("<", dirbeg);
                    relative = false;
                }
                if (argbeg == std::string::npos) {
                    LOG_ERROR << "invalid include type" << std::endl;
                    goto next;
                }

                size_t argend = 0;
                if (relative) {
                    if ((argend = linestr.find("\"", argbeg + 1)) == std::string::npos) {
                        LOG_ERROR << "missing closing character" << std::endl;
                        goto next;
                    }
                }
                else {
                    if ((argend = linestr.find(">", argbeg + 1)) == std::string::npos) {
                        LOG_ERROR << "missing closing character" << std::endl;
                        goto next;
                    }
                }

                std::string arg = linestr.substr(argbeg + 1, argend - argbeg - 1);

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

                std::filesystem::path inpath(arg);

                //auto include_preprocessed = preprocess(path.string());
                //preprocessed.insert(preprocessed.end(), include_preprocessed.begin(), include_preprocessed.end());
                // read include
                std::ifstream infs(inpath);
                if (!infs.is_open()) {
                    LOG_ERROR << "opening " << arg << ": " TERM_RESET << strerror(errno) << std::endl;
                    goto next;
                }
                std::stringstream inss;
                inss << infs.rdbuf();
                infs.close();

                std::vector<line_t> included;

                size_t i = 0;
                while (std::getline(inss, linestr)) {
                    included.push_back(line_t{arg, i, linestr});
                    i++;
                }

                unprocessed.insert(unprocessed.begin(), included.begin(), included.end());

                //std::streamsize orgpos = ss.tellp();
                //ss.str(inss.str() + ss.str());
                //ss.seekp(pos + s.length());

                LOG_DEBUG << "included " << inpath.string() << std::endl;

            } else {
                LOG_ERROR << "unrecognised preprocessor directive " << firsttok << std::endl;
                goto next;
            }
        } else {
            preprocessed.push_back(line);
        }

    next:
        unprocessed.pop_front();
    }

    return preprocessed;
}
