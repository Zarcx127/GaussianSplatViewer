#include "assets/ply/details/PlyLittleEndianScalar.hpp"

#include <limits>
#include <cstdint>
#include <cstring>

static_assert(
    ((sizeof(float) == 4) && std::numeric_limits<float>::is_iec559),
    "PLY Float32 requires a 32-bit IEEE-754 float"
);

static_assert(
    ((sizeof(double) == 8) && std::numeric_limits<double>::is_iec559),
    "PLY Float64 requires a 64-bit IEEE-754 double"
);

namespace 
{
    bool read_little_endian_bits(
        std::istream& stream,
        uint8_t byteCount,
        uint64_t& raw,
        const char*& error
    );

    double decode_unsigned_integer(uint64_t raw);
    double decode_signed_integer(uint64_t raw, uint8_t byteCount);
    double decode_floating_point(uint64_t raw, uint8_t byteCount);
}

bool read_binary_little_endian_ply_scalar(
    std::istream& stream,
    const PlyScalarInfo& info,
    double& value,
    const char*& error
) {
    if(info.category == PlyScalarCategory::Invalid)
    {
        error = "Cannot read an invalid PLY scalar";
        return false;
    }

    uint64_t raw = 0;
    if(!read_little_endian_bits(stream, info.byteCount, raw, error))
        return false;

    double parsedValue = 0.0;
    switch(info.category)
    {
        case PlyScalarCategory::SignedInteger:
            parsedValue = decode_signed_integer(raw, info.byteCount);
            break;

        case PlyScalarCategory::UnsignedInteger:
            parsedValue = decode_unsigned_integer(raw);
            break;

        case PlyScalarCategory::FloatingPoint:
            parsedValue = decode_floating_point(raw, info.byteCount);
            break;

        case PlyScalarCategory::Invalid:
        default:
            error = "Unsupported PLY scalar category";
            return false;
    }

    if(!is_valid_ply_scalar_value(parsedValue, info))
    {
        error = "Binary PLY scalar contains an invalid value";
        return false;
    }

    value = parsedValue;

    return true;
}

namespace
{
    bool read_little_endian_bits(
        std::istream& stream,
        uint8_t byteCount,
        uint64_t& raw,
        const char*& error
    ) {
        if((byteCount == 0) || (byteCount > 8))
        {
            error = "Invalid PLY scalar byte count";
            return false;
        }

        uint8_t bytes[8] = {};
        stream.read(
            reinterpret_cast<char*>(bytes),
            static_cast<std::streamsize>(byteCount)
        );

        if(!stream)
        {
            error = "Unexpected end of binary PLY scalar data";
            return false;
        }

        raw = 0;
        for(uint8_t index = 0; index < byteCount; index++)
        {
            uint32_t shift = (static_cast<uint32_t>(index) * 8);
            raw |= (static_cast<uint64_t>(bytes[index]) << shift);
        }

        return true;
    }

    double decode_unsigned_integer(uint64_t raw)
    {
        return static_cast<double>(raw);
    }

    double decode_signed_integer(uint64_t raw, uint8_t byteCount)
    {
        uint32_t bitCount = (static_cast<uint32_t>(byteCount) * 8);
        uint64_t signMask = (static_cast<uint64_t>(1) << (bitCount - 1));

        if((raw & signMask) == 0)
            return static_cast<double>(raw);

        uint64_t modulus = (static_cast<uint64_t>(1) << bitCount);

        return (static_cast<double>(raw) - static_cast<double>(modulus));
    }

    double decode_floating_point(uint64_t raw, uint8_t byteCount)
    {
        if(byteCount == 4)
        {
            float decoded;
            std::memcpy(&decoded, &raw, byteCount);

            return static_cast<double>(decoded);
        }

        double decoded;
        std::memcpy(&decoded, &raw, byteCount);

        return decoded;
    }
}
