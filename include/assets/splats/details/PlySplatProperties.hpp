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

#ifndef ASSETS_SPLATS_DETAILS_PLY_SPLAT_PROPERTIES_H
#define ASSETS_SPLATS_DETAILS_PLY_SPLAT_PROPERTIES_H

#include <vector>

#include "assets/ply/PlyReader.hpp"
#include "assets/splats/SplatCloud.hpp"

void inspect_ply_splat_properties(
    const std::vector<PlyProperty>& properties,
    SplatCloudInfo& info
);

#endif
