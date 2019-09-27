#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>

struct GLOB_VARS
{
    SDL_Window *myWindow = nullptr;
    VkInstance myVkInstance = nullptr;
    std::vector<const char*> myvkValLayers;
    bool myvkValLayersEnabled = false;
    VkDebugUtilsMessengerEXT myDebugMessager = nullptr;
};