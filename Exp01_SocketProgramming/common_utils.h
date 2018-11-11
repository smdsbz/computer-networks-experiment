#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

bool string_starts_with(
    const std::string &lhs,
    const std::string &rhs
);

bool string_ends_with(
    const std::string &lhs,
    const std::string &rhs
);

std::string read_file(const std::string &filename);
