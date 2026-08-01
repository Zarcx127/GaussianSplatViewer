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

#ifndef ASSETS_PLY_PLY_READER_H
#define ASSETS_PLY_PLY_READER_H

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include "assets/ply/details/PlyScalar.hpp"

enum class PlyFormat : uint32_t
{
    Unknown,
    Ascii,
    BinaryLittleEndian
};

struct PlyProperty
{
    std::string name;
    PlyScalarInfo scalar {};
};

struct PlyElementProperty
{
    bool isList { false };

    PlyScalarInfo scalar {};
    PlyScalarInfo listCountScalar {};
};

struct PlyElement
{
    uint64_t count { 0 };

    std::vector<PlyElementProperty> properties;
};


class PlyReader
{
public:
    PlyReader() = default;

    PlyReader(const PlyReader&) = delete;
    PlyReader& operator=(const PlyReader&) = delete;

    bool open(const std::filesystem::path& path, const char*& error);
    void close();

    bool read_scalar(
        const PlyScalarInfo& type, 
        double& value, 
        const char*& error
    );

    uint64_t vertex_count() const;

    const std::vector<PlyProperty>& vertex_properties() const;

private:
    bool parse_header(const char*& error);
    bool skip_pre_vertex_elements(const char*& error);

    std::ifstream m_file;

    PlyFormat m_format { PlyFormat::Unknown };
    uint64_t m_vertexCount { 0 };

    std::vector<PlyElement> m_preVertexElements;
    std::vector<PlyProperty> m_vertexProperties;
};

#endif
