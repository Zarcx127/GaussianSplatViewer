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

#ifndef SPLAT_STRUCTURES_GLSL
#define SPLAT_STRUCTURES_GLSL

struct Splat
{
    vec4 position;
    vec4 color;
    vec4 logScale;
    vec4 rotation;
};

struct SortDispatchCommand
{
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
    uint valueCount;
};

struct ProjectedSplat
{
    vec4 clipCenter;
    vec4 conicOpacity;
    vec4 color;
    uvec4 tileBounds;
};

struct SplatEntryKey
{
    uint tileIndex;
    uint depthKey;
};

struct SplatEntryRange
{
    uint entryCount;
    uint entryOffset;
};

struct SplatTileRange
{
    uint startIndex;
    uint endIndex;
};

#endif
