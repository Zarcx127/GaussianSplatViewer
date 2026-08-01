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

#include "assets/ply/details/PlyAsciiScalar.hpp"

bool read_ascii_ply_scalar(
    std::istream& stream,
    PlyScalarInfo info,
    double& value,
    const char*& error
) {
    double parsedValue = 0.0;

    if(!(stream >> parsedValue))
    {
        error = "Failed to read ASCII PLY scalar";
        return false;
    }

    if(!is_valid_ply_scalar_value(parsedValue, info))
    {
        error = "ASCII PLY scalar does not match its declared type";
        return false;
    }

    value = parsedValue;
    
    return true;
}
