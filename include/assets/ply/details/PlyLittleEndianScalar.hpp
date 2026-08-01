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
