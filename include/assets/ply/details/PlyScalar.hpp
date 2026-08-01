#pragma once

#ifndef ASSETS_PLY_DETAILS_PLY_SCALAR_H
#define ASSETS_PLY_DETAILS_PLY_SCALAR_H

#include <string>
#include <cstdint>

enum class PlyScalarCategory
{
    Invalid,
    SignedInteger,
    UnsignedInteger,
    FloatingPoint
};

struct PlyScalarInfo
{
    PlyScalarCategory category { PlyScalarCategory::Invalid };

    uint8_t byteCount { 0 };

    double minimum { 0.0 };
    double maximum { 0.0 };
};

PlyScalarInfo parse_ply_scalar_info(const std::string& type);

bool is_valid_ply_scalar_value(double value, const PlyScalarInfo& info);

#endif
