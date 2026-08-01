/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#ifndef BACKEND_UTILS_H
#define BACKEND_UTILS_H

#include <vector>
#include <cstdint>

namespace utils
{
    bool vector_compare(
        const std::vector<const char*>& vec1, 
        const std::vector<const char*>& vec2
    );

    std::vector<uint32_t> read_spv_file(const char* file);

    uint32_t divide_round_up(uint32_t value, uint32_t divisor);
}

#endif
