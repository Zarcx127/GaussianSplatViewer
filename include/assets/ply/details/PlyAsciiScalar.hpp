#pragma once

#ifndef ASSETS_PLY_DETAILS_PLY_ASCII_SCALAR_H
#define ASSETS_PLY_DETAILS_PLY_ASCII_SCALAR_H

#include <istream>

#include "assets/ply/details/PlyScalar.hpp"

bool read_ascii_ply_scalar(
    std::istream& stream,
    PlyScalarInfo info,
    double& value,
    const char*& error
);

#endif
