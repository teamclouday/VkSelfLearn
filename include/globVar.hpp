#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vector>
#include <set>
#include <string>

#define APP_NAME                "VkSelfLearn"
#define WINDOW_TITLE            "Vulkan Self-Learn"
#define WINDOW_W                800
#define WINDOW_H                600
#define FPS                     144
#define MAX_FRAMES_IN_FLIGHT    2

#ifndef MY_ROOT_DIR
#define MY_ROOT_DIR             "."
#endif

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
    VkSwapchainKHR myVkSwapChain = nullptr;
    std::vector<VkImage> myVkSwapChainImages;
    std::vector<VkImageView> myVkSwapChainImageViews;
    VkFormat myVkSwapChainFormat;
    VkExtent2D myVkSwapChainExtent;
    VkDebugUtilsMessengerEXT myDebugMessager = nullptr;
    VkPipelineLayout myVkPipelineLayout = nullptr;
    VkRenderPass myVkRenderPass = nullptr;
    VkPipeline myVkGraphicsPipeline = nullptr;
    std::vector<VkFramebuffer> myVkSwapChainFramebuffers;
    VkCommandPool myVkCommandPool = nullptr;
    std::vector<VkCommandBuffer> myVkCommandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    size_t currentFrame = 0;
};