#pragma once

#ifndef ASSETS_PLY_PLY_READER_H
#define ASSETS_PLY_PLY_READER_H

#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

#include "assets/ply/details/PlyScalar.hpp"

enum class PlyFormat
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

class PlyReader
{
public:
    PlyReader() = default;

    PlyReader(const PlyReader&) = delete;
    PlyReader& operator=(const PlyReader&) = delete;

    bool open(const char* path, const char*& error);
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

    std::ifstream m_file;

    PlyFormat m_format { PlyFormat::Unknown };
    uint64_t m_vertexCount { 0 };

    std::vector<PlyProperty> m_vertexProperties;
};

#endif
