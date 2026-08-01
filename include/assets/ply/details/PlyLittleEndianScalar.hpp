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

#ifndef ASSETS_PLY_DETAILS_PLY_LITTLE_ENDIAN_SCALAR_H
#define ASSETS_PLY_DETAILS_PLY_LITTLE_ENDIAN_SCALAR_H

#include <istream>

#include "assets/ply/details/PlyScalar.hpp"

bool read_binary_little_endian_ply_scalar(
    std::istream& stream,
    const PlyScalarInfo& info,
    double& value,
    const char*& error
);

#endif
