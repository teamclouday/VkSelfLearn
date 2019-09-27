#include "tools.hpp"

extern GLOB_VARS globVar;

bool isDeviceSuitable(VkPhysicalDevice device)
{
    // VkPhysicalDeviceProperties deviceProperties;
    // VkPhysicalDeviceFeatures deviceFeatures;
    // vkGetPhysicalDeviceProperties(device, &deviceProperties);
    // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    // return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
    //        deviceFeatures.geometryShader;
    
    // check queue families of the device
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.found; // just use any gpu available
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
            indices.found = true;
            break;
        }
        i++;
    }

    return indices;
}