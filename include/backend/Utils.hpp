#pragma once

#ifndef BACKEND_UTILS_H
#define BACKEND_UTILS_H

#include <vector>
#include <cstdint>

namespace utils
{
    bool vector_compare(const std::vector<const char*>& vec1, const std::vector<const char*>& vec2);

    std::vector<uint32_t> read_spv_file(const char* file);
}

#endif
