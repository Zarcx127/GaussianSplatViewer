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

#include "assets/splats/SplatCloud.hpp"

bool SplatCloud::empty() const
{
    return splats.empty();
}

uint32_t SplatCloud::splat_count() const
{
    return static_cast<uint32_t>(splats.size());
}

uint32_t SplatCloud::spherical_harmonic_coefficient_count() const
{
    if(!info.hasDcColor)
        return 0;

    uint32_t side = (info.sphericalHarmonicDegree + 1);

    return (side * side);
}

void SplatCloud::clear()
{
    splats.clear();
    sphericalHarmonics.clear();

    info = {};
}
