#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>
#include <set>
#include <string>

struct GLOB_VARS
{
    SDL_Window *myWindow = nullptr;
    VkInstance myVkInstance = nullptr;
    std::vector<const char*> myVkValLayers;
    bool myVkValLayersEnabled = false;
    std::vector<const char*> myDeviceExtensions;
    VkPhysicalDevice myVkPhysicalDevice = nullptr;
    VkDevice myVkLogicalDevice = nullptr;
    VkQueue myVkGraphicsQueue = nullptr;
    VkQueue myVkPresentQueue = nullptr;
    VkSurfaceKHR myVkSurface = nullptr;
    VkDebugUtilsMessengerEXT myDebugMessager = nullptr;
};