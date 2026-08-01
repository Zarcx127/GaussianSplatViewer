/**
 * Copyright (C) 2026  Zarcx127@github.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 **/

#include "backend/Utils.hpp"

#include <cstring>
#include <fstream>
#include <sstream>

#include "logging/Logger.hpp"


bool utils::vector_compare(
    const std::vector<const char*>& vec1, 
    const std::vector<const char*>& vec2
) {
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

uint32_t utils::divide_round_up(uint32_t value, uint32_t divisor)
{
    if(divisor == 0)
        return 0;

    return (
        (value / divisor) +
        static_cast<uint32_t>((value % divisor) != 0)
    );
}