#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>

struct GLOB_VARS
{
    SDL_Window *myWindow = nullptr;
    VkInstance myVkInstance;
    std::vector<const char*> myVkValLayers;
    bool myVkValLayersEnabled = false;
    VkDebugUtilsMessengerEXT myDebugMessager;
    VkPhysicalDevice myVkPhysicalDevice;
    VkDevice myVkLogicalDevice;
};