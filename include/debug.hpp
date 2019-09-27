// a collection of vulkan debug callback function
#pragma once

#define MY_DEBUG_MODE

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdio.h>

#include "globVar.hpp"

bool setUpDebugMessager();
// need a to look up the function because it is not automatically loaded as an extension
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageTypeFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);