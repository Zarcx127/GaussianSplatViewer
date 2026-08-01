#pragma once

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

namespace build
{
#ifdef DEBUG
    
    constexpr bool enableLogging = true;
    constexpr bool enableValidation = true;

#else

    constexpr bool enableLogging = false;
    constexpr bool enableValidation = false;

#endif
}

#endif
