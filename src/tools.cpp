#include "tools.hpp"

extern GLOB_VARS globVar;

bool isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    bool indiceComplete = indices.graphicsFamilyFound && indices.presentFamilyFound;
    bool extensionSupport = checkDeviceExtensionSupport(device);
    bool swapChainSupport = false;
    if(extensionSupport)
    {
        SwapChainSupportDetails swapChainDetails = querySwapChainSupport(device);
        swapChainSupport = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
    }
    return indiceComplete && extensionSupport && swapChainSupport;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensions(globVar.myDeviceExtensions.begin(), globVar.myDeviceExtensions.end());
    for(std::vector<VkExtensionProperties>::iterator iter = availableExtensions.begin(); iter != availableExtensions.end(); iter++)
    {
        requiredExtensions.erase(iter->extensionName);
    }
    return requiredExtensions.empty();
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for(std::vector<VkQueueFamilyProperties>::iterator iter = queueFamilies.begin(); iter != queueFamilies.end(); iter++)
    {
        // find one that support graphics queue
        if(iter->queueCount > 0 && iter->queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
            indices.graphicsFamilyFound = true;
        }
        // find one that support present queue
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, globVar.myVkSurface, &presentSupport);
        if(iter->queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
            indices.presentFamilyFound = true;
        }
        i++;
    }

    return indices;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, globVar.myVkSurface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, globVar.myVkSurface, &formatCount, nullptr);
    if(formatCount)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, globVar.myVkSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, globVar.myVkSurface, &presentModeCount, nullptr);
    if(presentModeCount)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, globVar.myVkSurface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for(std::vector<VkSurfaceFormatKHR>::iterator iter = availableFormats.begin(); iter != availableFormats.end(); iter++)
    {
        if(iter->format == VK_FORMAT_B8G8R8A8_UNORM && iter->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return *iter;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for(std::vector<VkPresentModeKHR>::iterator iter = availablePresentModes.begin(); iter != availablePresentModes.end(); iter++)
    {
        if(*iter == VK_PRESENT_MODE_MAILBOX_KHR)
            return *iter;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR& capabilities)
{
    if(capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    VkExtent2D actualExtent = {WINDOW_W, WINDOW_H};
    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));
    return actualExtent;
}