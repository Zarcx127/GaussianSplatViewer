#include "backend/Utils.hpp"

#include <cstring>
#include <fstream>
#include <sstream>

#include "logging/Logger.hpp"

std::vector<uint32_t> utils::read_spv_file(const char* filename)
{
    Logger* logger = Logger::get_logger(); 

    std::ifstream file(filename, (std::ios::ate | std::ios::binary));
    if(!file.is_open())
    {
        std::stringstream line;
        line << "Failed to load " << filename;
        logger->print(line.str().c_str());
        
        return {};
    }
    
    std::streampos fileSizePosition = file.tellg();
    if(fileSizePosition == std::streampos(-1))
    {
        std::stringstream line;
        line << "Failed to get file size for " << filename;
        logger->print(line.str().c_str());

        return {};
    }

    size_t fileSize = static_cast<size_t>(fileSizePosition);
    if(fileSize == 0)
    {
        std::stringstream line;
        line << filename << " is empty" << std::endl;
        logger->print(line.str().c_str());

        return {};
    }

    if((fileSize % sizeof(uint32_t)) != 0)
    {
        std::stringstream line;
        line << filename << " size is not aligned to uint32_t";
        logger->print(line.str().c_str());

        return {};
    }

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    if(!file)
    {
        std::stringstream line;
        line << "Failed to read " << filename;
        logger->print(line.str().c_str());

        return {};
    }

    file.close();
    
    return buffer;
}

bool utils::vector_compare(const std::vector<const char*>& vec1, const std::vector<const char*>& vec2)
{
    Logger* logger = Logger::get_logger();
    std::stringstream line;

    bool found;
    for(const char* str1 : vec1)
    {
        found = false;
        line.str("");
        
        for(const char* str2 : vec2)
        {
            if(std::strcmp(str1, str2) == 0)
            {
                found = true;
                break;
            }
        }

        if(!found)
        {
            line << str1 << " is not supported";
            logger->print(line.str().c_str());

            return false;
        }
    }

    return true;
}
