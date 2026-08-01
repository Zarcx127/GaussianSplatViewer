#include "backend/Utils.hpp"

#ifdef UTILS_H

#include <fstream>
#include <sstream>

#include "logging/Logger.hpp"

std::vector<char> utils::read_file(const char* filename)
{
    Logger* logger = Logger::get_logger();

    std::ifstream file(filename, (std::ios::ate | std::ios::binary));
    if(!file.is_open())
    {
        std::stringstream line;
        line << "Failed to load " << filename << std::endl;

        logger->print(line.str().c_str());
    }
    
    size_t filesize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(filesize);

    file.seekg(0);
    file.read(buffer.data(), filesize);

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
            if(strcmp(str1, str2) == 0)
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

#endif
