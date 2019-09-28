#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "globVar.hpp"

#include <set>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <fstream>

struct QueueFamilyIndices
{
    uint32_t graphicsFamily;
    bool graphicsFamilyFound = false;
    uint32_t presentFamily;
    bool presentFamilyFound = false;
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

bool isDeviceSuitable(VkPhysicalDevice device);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);
std::vector<char> readCompiledShader(const std::string& filename);
VkShaderModule createShaderModule(const std::vector<char>& code);
bool createGraphicsPipeline();