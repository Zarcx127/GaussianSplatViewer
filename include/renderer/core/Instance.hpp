#pragma once

#ifndef INSTANCE_H
#define INSTANCE_H

#include <deque>
#include <vector>
#include <functional>

#include <vulkan/vulkan.hpp>

vk::Instance make_instance(
    const char* applicationName, 
    std::deque<std::function<void(vk::Instance)>>& deletionQueue
);

#endif
