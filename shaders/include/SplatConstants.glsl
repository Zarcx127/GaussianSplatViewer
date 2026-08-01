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

#ifndef SPLAT_CONSTANTS_GLSL_H
#define SPLAT_CONSTANTS_GLSL_H

const float SIGMA_EXTENT = 3.0;

const float SPLAT_MIN_ALPHA = (1.0 / 255.0);
const float SPLAT_MAX_ALPHA = 0.99;

const uint SPLAT_TILE_SIZE = 16;

const uint SPLAT_ENTRY_SCAN_LOCAL_SIZE = 256;

const uint SPLAT_SORT_LOCAL_SIZE = 256;
const uint SPLAT_SORT_SCAN_LOCAL_SIZE = 256;

const uint SPLAT_SORT_RADIX_BITS = 4;
const uint SPLAT_SORT_RADIX_BUCKET_COUNT = (
    1U << SPLAT_SORT_RADIX_BITS
);

const uint SPLAT_SORT_KEY_TILE = 1;

#endif
