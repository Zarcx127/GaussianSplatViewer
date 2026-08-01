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
