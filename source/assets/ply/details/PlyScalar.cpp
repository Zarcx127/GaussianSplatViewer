#include "assets/ply/details/PlyScalar.hpp"

#include <cmath>
#include <limits>

namespace 
{
    template<PlyScalarCategory Category, typename Type>
    PlyScalarInfo make_scalar_info();
};

PlyScalarInfo parse_ply_scalar_info(const std::string& type)
{
    if((type == "char") || (type == "int8")) 
        return make_scalar_info<PlyScalarCategory::SignedInteger, int8_t>();

    if((type == "short") || (type == "int16"))
        return make_scalar_info<PlyScalarCategory::SignedInteger, int16_t>();

    if((type == "int") || (type == "int32")) 
        return make_scalar_info<PlyScalarCategory::SignedInteger, int32_t>();

    if((type == "uchar") || (type == "uint8"))
        return make_scalar_info<PlyScalarCategory::UnsignedInteger, uint8_t>();
    
    if((type == "ushort") || (type == "uint16")) 
        return make_scalar_info<PlyScalarCategory::UnsignedInteger, uint16_t>();
    
    if((type == "uint") || (type == "uint32")) 
        return make_scalar_info<PlyScalarCategory::UnsignedInteger, uint32_t>();
    
    if((type == "float") || (type == "float32")) 
        return make_scalar_info<PlyScalarCategory::FloatingPoint, float>();
    
    if((type == "double") || (type == "float64")) 
        return make_scalar_info<PlyScalarCategory::FloatingPoint, double>();

    return {};
}

bool is_valid_ply_scalar_value(double value, const PlyScalarInfo& info)
{
    if(!std::isfinite(value)) 
        return false;

    if(info.category == PlyScalarCategory::Invalid)
        return false;

    bool isInteger = (
        (info.category == PlyScalarCategory::SignedInteger) ||
        (info.category == PlyScalarCategory::UnsignedInteger)
    );

    if(isInteger && (std::trunc(value) != value))
        return false;

    return (
        (value >= info.minimum) && 
        (value <= info.maximum)
   );
}

namespace 
{
    template<PlyScalarCategory Category, typename Type>
    PlyScalarInfo make_scalar_info()
    {
        return {
            Category,
            static_cast<uint8_t>(sizeof(Type)),
            static_cast<double>(std::numeric_limits<Type>::lowest()),
            static_cast<double>(std::numeric_limits<Type>::max()),
        };
    }
};
