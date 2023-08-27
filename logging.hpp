#pragma once

#define TERM_RED "\e[0;31m"
#define TERM_RESET "\e[0m"

#define LOG_DEBUG std::cout << filename << ":" << linen << ": debug: "
#define LOG_ERROR std::cout << filename << ":" << linen << ": " << TERM_RED "error: " TERM_RESET 
