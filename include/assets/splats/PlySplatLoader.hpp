#pragma once

#ifndef ASSETS_SPLATS_PLY_SPLAT_LOADER_H
#define ASSETS_SPLATS_PLY_SPLAT_LOADER_H

#include "assets/splats/SplatCloud.hpp"

struct PlySplatLoadResult
{
    bool success { false };
    const char* error { nullptr };

    SplatCloud cloud {}; 
};

PlySplatLoadResult load_ply_splat_cloud(const char* path);

#endif
