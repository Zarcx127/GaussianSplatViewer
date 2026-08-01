#pragma once

#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cstdint>

namespace utils
{
    bool vector_compare(const std::vector<const char*>& vec1, const std::vector<const char*>& vec2);

    std::vector<uint32_t> read_file(const char* file);
}

#endif
