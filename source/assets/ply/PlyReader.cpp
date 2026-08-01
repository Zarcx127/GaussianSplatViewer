#include "assets/ply/PlyReader.hpp"

#include <limits>
#include <sstream>

#include "assets/ply/details/PlyAsciiScalar.hpp"
#include "assets/ply/details/PlyLittleEndianScalar.hpp"

namespace 
{
    std::string strip_cr(std::string line);

    bool has_property(
        const std::vector<PlyProperty>& properties, 
        const char* name
    );
}

bool PlyReader::open(const char* path, const char*& error)
{
    close();
    
    error = nullptr;
    if(!path || (path[0] == '\0'))
    {
        error = "PLY path is empty";
        return false;
    }

    m_file.open(path, std::ios::binary);
    if(!m_file.is_open())
    {
        error = "Failed to open PLY file";
        return false;
    }

    if(!parse_header(error))
    {
        close();
        return false;
    }

    return true;
}

void PlyReader::close()
{
    if(m_file.is_open()) 
        m_file.close();

    m_file.clear();

    m_format = PlyFormat::Unknown;
    m_vertexCount = 0;

    m_vertexProperties.clear();
}

bool PlyReader::read_scalar(
    const PlyScalarInfo& type, 
    double& value, 
    const char*& error
) {
    error = nullptr;

    if(!m_file.is_open())
    {
        error = "PLY reader has no open file";
        return false;
    }

    if(type.category == PlyScalarCategory::Invalid)
    {
        error = "Cannot read an invalid PLY scalar type";
        return false;
    }

    switch(m_format)
    {
        case PlyFormat::Ascii:
            return read_ascii_ply_scalar(m_file, type, value, error);

        case PlyFormat::BinaryLittleEndian:
            return read_binary_little_endian_ply_scalar(m_file, type, value, error);
        
        case PlyFormat::Unknown:
            error = "PLY reader has no active format";
            return false;
    }

    error = "Unknown PLY reader format";
    
    return false;
}

uint64_t PlyReader::vertex_count() const
{
    return m_vertexCount;
}

const std::vector<PlyProperty>& PlyReader::vertex_properties() const
{
    return m_vertexProperties;
}

bool PlyReader::parse_header(const char*& error)
{
    std::string line;
    if(!std::getline(m_file, line))
    {
        error = "PLY file is empty";
        return false;
    }

    line = strip_cr(line);
    if(line != "ply")
    {
        error = "File is not a PLY file";
        return false;
    }

    bool readingVertexElement = false;
    bool foundEndHeader = false;

    while(std::getline(m_file, line))
    {
        line = strip_cr(line);

        if(line == "end_header")
        {
            foundEndHeader = true;
            break;
        }

        std::istringstream stream(line);

        std::string token;
        stream >> token;

        if(token.empty() || (token == "comment") || (token == "obj_info"))
            continue;

        if(token == "format")
        {
            std::string format;
            std::string version;

            stream >> format >> version;

            if(format == "ascii")
                m_format = PlyFormat::Ascii;
            else if(format == "binary_little_endian")
                m_format = PlyFormat::BinaryLittleEndian;
            else
                m_format = PlyFormat::Unknown;

            if(m_format == PlyFormat::Unknown)
            {
                error = "Unsupported PLY format";
                return false;
            }
        }
        else if(token == "element")
        {
            std::string elementName;
            uint64_t elementCount = 0;

            stream >> elementName >> elementCount;

            readingVertexElement = (elementName == "vertex");
            if(readingVertexElement)
                m_vertexCount = elementCount;

            continue;
        }
        else if(token == "property")
        {
            if(!readingVertexElement) continue;

            std::string type;
            stream >> type;

            if(type == "list")
            {
                error = "List properties inside vertex elements are not supported";
                return false;
            }

            std::string name;
            stream >> name;

            PlyScalarInfo scalarType = parse_ply_scalar_info(type);
            if(scalarType.category == PlyScalarCategory::Invalid)
            {
                error = "Unsupported vertex property type";
                return false;
            }

            if(name.empty())
            {
                error = "Vertex property is missing a name";
                return false;
            }

            m_vertexProperties.push_back({name, scalarType});
        }
    }

    if(!foundEndHeader)
    {
        error = "PLY header is missing end_header";
        return false;
    }

    if(m_format == PlyFormat::Unknown)
    {
        error = "PLY header is missing format";
        return false;
    }

    if(m_vertexCount == 0)
    {
        error = "PLY file has no vertices";
        return false;
    }

    if(m_vertexProperties.empty())
    {
        error = "PLY vertex element has no properties";
        return false;
    }

    if(
        !has_property(m_vertexProperties, "x") ||
        !has_property(m_vertexProperties, "y") ||
        !has_property(m_vertexProperties, "z")
    ) {
        error = "PLY vertex element must contain x, y and z properties";
        return false;
    }

    if(m_vertexCount > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
    {
        error = "PLY vertex count is too large for this platform";
        return false;
    }

    return true;
}

namespace 
{
    std::string strip_cr(std::string line)
    {
        if(!line.empty() && (line.back() == '\r'))
            line.pop_back();

        return line;
    }

    bool has_property(
        const std::vector<PlyProperty>& properties, 
        const char* name
    ) {
        for(const PlyProperty& property : properties)
            if(property.name == name) 
                return true;

        return false;
    }
}
