#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <vector>

namespace utils
{
    bool vector_compare(const std::vector<const char*>& vec1, const std::vector<const char*>& vec2);

    std::vector<char> read_file(const char* file);
}

#endif
