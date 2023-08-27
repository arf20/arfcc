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

    main.cpp: Program entry point

*/


#include <iostream>
#include <string>

#include <cxxopts.hpp>

#include "preprocessor.hpp"

int main(int argc, char **argv) {
    cxxopts::Options options("arfcc",
        "arfcc: Yeah, Everyone needs to code a compiler, right?");

    options.add_options()
        ("h,help", "Display this message", cxxopts::value<bool>())
        ("d,debug", "Debug output", cxxopts::value<bool>())
        ("files", "The file to compile", cxxopts::value<std::vector<std::string>>());

    options.parse_positional({"files"});
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (result.count("files")) {
        for (std::string file : result["files"].as<std::vector<std::string>>()) {
            
            preprocess(file);
        }
    }
}
