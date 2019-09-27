#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "globVar.hpp"

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    bool graphicsFamilyFound = false;
    uint32_t presentFamily;
    bool presentFamilyFound = false;
};

bool isDeviceSuitable(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);