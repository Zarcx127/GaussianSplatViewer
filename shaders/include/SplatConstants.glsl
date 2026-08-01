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
