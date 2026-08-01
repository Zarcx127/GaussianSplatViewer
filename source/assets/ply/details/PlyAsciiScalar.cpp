#include "assets/ply/details/PlyAsciiScalar.hpp"

bool read_ascii_ply_scalar(
    std::istream& stream,
    PlyScalarInfo info,
    double& value,
    const char*& error
) {
    double parsedValue = 0.0;

    if(!(stream >> parsedValue))
    {
        error = "Failed to read ASCII PLY scalar";
        return false;
    }

    if(!is_valid_ply_scalar_value(parsedValue, info))
    {
        error = "ASCII PLY scalar does not match its declared type";
        return false;
    }

    value = parsedValue;
    
    return true;
}
