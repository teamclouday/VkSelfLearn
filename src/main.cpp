#include "main.hpp"

GLOB_VARS globVar;

void renderFrame()
{
    vkWaitForFences(globVar.myVkLogicalDevice, 1, &globVar.inFlightFences[globVar.currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(globVar.myVkLogicalDevice, 1, &globVar.inFlightFences[globVar.currentFrame]);
    // aquire an image from swap chain
    uint32_t imageIndex;
    vkAcquireNextImageKHR(globVar.myVkLogicalDevice, globVar.myVkSwapChain, UINT64_MAX, globVar.imageAvailableSemaphores[globVar.currentFrame], VK_NULL_HANDLE, &imageIndex);
    // submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    VkSemaphore waitSemaphores[] = {globVar.imageAvailableSemaphores[globVar.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &globVar.myVkCommandBuffers[imageIndex];
    VkSemaphore signalSemaphore[] = {globVar.renderFinishedSemaphores[globVar.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphore;
    if(vkQueueSubmit(globVar.myVkGraphicsQueue, 1, &submitInfo, globVar.inFlightFences[globVar.currentFrame]) != VK_SUCCESS)
    {
        printf("Failed to submit draw command buffer\n");
        return;
    }
    // create presentation
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphore;
    VkSwapchainKHR swapChains[] = {globVar.myVkSwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;
    vkQueuePresentKHR(globVar.myVkPresentQueue, &presentInfo);

    globVar.currentFrame = (globVar.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main(int argc, char *argv[])
{
    if(!initAll())
        exit(1);
    bool quit = false;
    Uint32 tNow = SDL_GetTicks();
    Uint32 tPrev = SDL_GetTicks();
    while(!quit)
    {
        pollEvents(&quit);
        fpsControl(&tNow, &tPrev);
        renderFrame();
    }
    vkDeviceWaitIdle(globVar.myVkLogicalDevice);
    return 0;
}

void pollEvents(bool *quit)
{
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
        {
            *quit = true;
            break;
        }
        else if(e.type == SDL_KEYDOWN)
        {
            switch(e.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    *quit = true;
                    break;
//                 case SDLK_F11:
//                 {
//                     bool isFullScreen = SDL_GetWindowFlags(globVar.myWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;
// #ifdef __unix__
//                     SDL_SetWindowPosition(globVar.myWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
// #endif
//                     SDL_SetWindowFullscreen(globVar.myWindow, isFullScreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
// #ifdef _WIN32
//                     if(isFullScreen)
//                         SDL_SetWindowPosition(globVar.myWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
// #endif
//                     break;
//                 }
            }
        }
    }
}

bool initAll()
{
    // Init SDL2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
    {
        printf("Failed to init SDL2\nSDL Error: %s\n", SDL_GetError());
        return false;
    }
    // Init SDL2_Window
    globVar.myWindow = SDL_CreateWindow(WINDOW_TITLE,
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        WINDOW_W, WINDOW_H,
                                        SDL_WINDOW_VULKAN);
    if(!globVar.myWindow)
    {
        printf("Failed to init SDL2 Window\nSDL Error: %s\n", SDL_GetError());
        return false;
    }
    // Set Vulkan Validation Layers
    globVar.myVkValLayers.push_back("VK_LAYER_KHRONOS_validation");
#ifdef MY_DEBUG_MODE
    globVar.myVkValLayersEnabled = true;
#endif
    unsigned int layercount = 0;
    VkLayerProperties* availableLayers = nullptr;
    if(globVar.myVkValLayersEnabled)
    {
        vkEnumerateInstanceLayerProperties(&layercount, nullptr);
        availableLayers = (VkLayerProperties*)malloc(layercount*sizeof(VkLayerProperties));
        vkEnumerateInstanceLayerProperties(&layercount, availableLayers);
    }
    // Check layers exists in available layers
    if(globVar.myVkValLayersEnabled)
    {
        if(!availableLayers)
        {
            printf("Error: No available layers found in Vulkan sdk\n");
            return false;
        }
        
        for(std::vector<const char*>::iterator iter = globVar.myVkValLayers.begin(); iter != globVar.myVkValLayers.end(); iter++)
        {
            bool layerFound = false;
            for(VkLayerProperties* ptr = availableLayers; ptr < availableLayers + layercount; ptr++)
            {
                if(!strcmp(*iter, ptr->layerName))
                {
                    layerFound = true;
                    break;
                }
            }
            if(!layerFound)
            {
                printf("Error: layer %s is not found in Vulkan sdk\n", *iter);
                return false;
            }
        }
    }
    // Set Vulkan Application Info
    VkApplicationInfo vkAppInfo = {};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pNext = NULL;
    vkAppInfo.pApplicationName = APP_NAME;
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.apiVersion = VK_API_VERSION_1_0;
    vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.pEngineName = APP_NAME;
    // Get Vulkan Extensions Info
    unsigned int vkecount = 0;
    if(!SDL_Vulkan_GetInstanceExtensions(globVar.myWindow, &vkecount, nullptr))
    {
        printf("SDL Failed to get Vulkan Instance Extensions\nSDL Error: %s\n", SDL_GetError());
        return false;
    }
    std::vector<const char*>vkenames;
    if(globVar.myVkValLayersEnabled)
    {
        vkenames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    size_t additional_ext_count = vkenames.size();
    vkenames.resize(additional_ext_count + vkecount);
    if(!SDL_Vulkan_GetInstanceExtensions(globVar.myWindow, &vkecount, vkenames.data() + additional_ext_count))
    {
        printf("SDL Failed to get Vulkan Instance Extensions\nSDL Error: %s\n", SDL_GetError());
        return false;
    }
    VkInstanceCreateInfo vkInstanceInfo = {};
    vkInstanceInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceInfo.enabledExtensionCount = static_cast<uint32_t>(vkenames.size());
    vkInstanceInfo.ppEnabledExtensionNames = vkenames.data();
    vkInstanceInfo.flags = 0;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if(globVar.myVkValLayersEnabled)
    {
        vkInstanceInfo.enabledLayerCount = static_cast<uint32_t>(globVar.myVkValLayers.size());
        vkInstanceInfo.ppEnabledLayerNames = globVar.myVkValLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        vkInstanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    }
    else
    {
        vkInstanceInfo.enabledLayerCount = 0;
        vkInstanceInfo.pNext = nullptr;
    }
    // Create Vulkan Instance
    if(vkCreateInstance(&vkInstanceInfo, nullptr, &globVar.myVkInstance) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan instance\n");
        return false;
    }
    // Create Debug Messager
    if(globVar.myVkValLayersEnabled)
    {
        if(!setUpDebugMessager())
            return false;
    }
    // Create Window Surface
    if(SDL_Vulkan_CreateSurface(globVar.myWindow, globVar.myVkInstance, &globVar.myVkSurface) != SDL_TRUE)
    {
        printf("Failed to create Vulkan surface from SDL2\nSDL Error: %s\n", SDL_GetError());
        return false;
    }
    // set device extensions
    globVar.myDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    // Initialize Physical Device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(globVar.myVkInstance, &deviceCount, nullptr);
    if(!deviceCount)
    {
        printf("Failed to find a GPU that supports Vulkan on this machine\n");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(globVar.myVkInstance, &deviceCount, devices.data());
    for(std::vector<VkPhysicalDevice>::iterator iter = devices.begin(); iter != devices.end(); iter++)
    {
        if(isDeviceSuitable(*iter))
        {
            globVar.myVkPhysicalDevice = *iter;
            break;
        }
    }
    if(!globVar.myVkPhysicalDevice)
    {
        printf("Failed to find a suitable GPU\n");
        return false;
    }
    // Initialize Logical Device
    QueueFamilyIndices indices = findQueueFamilies(globVar.myVkPhysicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};
    float queueCreatePriority = 1.0f;
    for(std::vector<uint32_t>::iterator iter = uniqueQueueFamilies.begin(); iter != uniqueQueueFamilies.end(); iter++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = *iter;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queueCreatePriority;
        queueCreateInfo.flags = 0;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures = {}; // Leave all features to VK_FALSE for now
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(globVar.myDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = globVar.myDeviceExtensions.data();
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    if(globVar.myVkValLayersEnabled)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(globVar.myVkValLayers.size());
        createInfo.ppEnabledLayerNames = globVar.myVkValLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }
    if(vkCreateDevice(globVar.myVkPhysicalDevice, &createInfo, nullptr, &(globVar.myVkLogicalDevice)) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan logical device\n");
        return false;
    }
    // Retrieve Queue Handles
    vkGetDeviceQueue(globVar.myVkLogicalDevice, indices.graphicsFamily, 0, &globVar.myVkGraphicsQueue);
    vkGetDeviceQueue(globVar.myVkLogicalDevice, indices.presentFamily, 0, &globVar.myVkPresentQueue);
    // Create Swap Chain
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(globVar.myVkPhysicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount; // not exceed the max image count
    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.surface = globVar.myVkSurface;
    swapChainInfo.minImageCount = imageCount;
    swapChainInfo.imageFormat = surfaceFormat.format;
    swapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainInfo.imageExtent = extent;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if(indices.graphicsFamily != indices.presentFamily)
    {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainInfo.queueFamilyIndexCount = 2;
        swapChainInfo.pQueueFamilyIndices = uniqueQueueFamilies.data();
    }
    else
    {
        swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainInfo.queueFamilyIndexCount = 0;
    }
    swapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = presentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainInfo.flags = 0;
    swapChainInfo.pNext = nullptr;
    if(vkCreateSwapchainKHR(globVar.myVkLogicalDevice, &swapChainInfo, nullptr, &globVar.myVkSwapChain) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan Swap Chain\n");
        return false;
    }
    // Get Swap Chain Images
    vkGetSwapchainImagesKHR(globVar.myVkLogicalDevice, globVar.myVkSwapChain, &imageCount, nullptr);
    globVar.myVkSwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(globVar.myVkLogicalDevice, globVar.myVkSwapChain, &imageCount, globVar.myVkSwapChainImages.data());
    // Store Swap Chain Vars
    globVar.myVkSwapChainFormat = surfaceFormat.format;
    globVar.myVkSwapChainExtent = extent;
    // Create Swap Chain Image Views
    globVar.myVkSwapChainImageViews.resize(globVar.myVkSwapChainImages.size());
    for(size_t i = 0; i < globVar.myVkSwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.image = globVar.myVkSwapChainImages[i];
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = globVar.myVkSwapChainFormat;
        viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        if(vkCreateImageView(globVar.myVkLogicalDevice, &viewCreateInfo, nullptr, &globVar.myVkSwapChainImageViews[i]) != VK_SUCCESS)
        {
            printf("Failed to create Vulkan Swap Chain Image View\n");
            return false;
        }
    }
    // Create Graphics Pipeline
    if(!createGraphicsPipeline())
        return false;
    // Create Framebuffers
    globVar.myVkSwapChainFramebuffers.resize(globVar.myVkSwapChainImageViews.size());
    for(size_t i = 0; i < globVar.myVkSwapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {globVar.myVkSwapChainImageViews[i]};
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = globVar.myVkRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = globVar.myVkSwapChainExtent.width;
        framebufferInfo.height = globVar.myVkSwapChainExtent.height;
        framebufferInfo.layers = 1;

        if(vkCreateFramebuffer(globVar.myVkLogicalDevice, &framebufferInfo, nullptr, &globVar.myVkSwapChainFramebuffers[i]) != VK_SUCCESS)
        {
            printf("Failed to create Vulkan Framebuffer\n");
            return false;
        }
    }
    // Create Command Pool
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;
    poolInfo.flags = 0;
    if(vkCreateCommandPool(globVar.myVkLogicalDevice, &poolInfo, nullptr, &globVar.myVkCommandPool) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan Command Pool\n");
        return false;
    }
    // Allocate Command Buffers
    globVar.myVkCommandBuffers.resize(globVar.myVkSwapChainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = globVar.myVkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(globVar.myVkCommandBuffers.size());
    if(vkAllocateCommandBuffers(globVar.myVkLogicalDevice, &allocInfo, globVar.myVkCommandBuffers.data()) != VK_SUCCESS)
    {
        printf("Failed to allocate Vulkan Command Buffers\n");
        return false;
    }
    // Start Command Buffer Recording
    for(size_t i = 0; i < globVar.myVkCommandBuffers.size(); i++)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        if(vkBeginCommandBuffer(globVar.myVkCommandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            printf("Failed to begin Recording Vulkan Command Buffer\n");
            return false;
        }
        // Start Render Pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = globVar.myVkRenderPass;
        renderPassInfo.framebuffer = globVar.myVkSwapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = globVar.myVkSwapChainExtent;
        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;
        vkCmdBeginRenderPass(globVar.myVkCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(globVar.myVkCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, globVar.myVkGraphicsPipeline);
        vkCmdDraw(globVar.myVkCommandBuffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(globVar.myVkCommandBuffers[i]);
        if(vkEndCommandBuffer(globVar.myVkCommandBuffers[i]) != VK_SUCCESS)
        {
            printf("Failed to Record Vulkan Command Buffer\n");
            return false;
        }
    }
    // Create Semaphores and Fences
    globVar.imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    globVar.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    globVar.inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if(vkCreateSemaphore(globVar.myVkLogicalDevice, &semaphoreInfo, nullptr, &globVar.imageAvailableSemaphores[i]) != VK_SUCCESS ||
           vkCreateSemaphore(globVar.myVkLogicalDevice, &semaphoreInfo, nullptr, &globVar.renderFinishedSemaphores[i]) != VK_SUCCESS ||
           vkCreateFence(globVar.myVkLogicalDevice, &fenceInfo, nullptr, &globVar.inFlightFences[i]) != VK_SUCCESS)
        {
            printf("Failed to create Vulkan Synchronization Objects\n");
            return false;
        }
    }
    return true;
}

void destroyAll()
{
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(globVar.myVkLogicalDevice, globVar.imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(globVar.myVkLogicalDevice, globVar.renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(globVar.myVkLogicalDevice, globVar.inFlightFences[i], nullptr);
    }
    if(globVar.myVkCommandPool)
        vkDestroyCommandPool(globVar.myVkLogicalDevice, globVar.myVkCommandPool, nullptr);
    for(std::vector<VkFramebuffer>::iterator iter = globVar.myVkSwapChainFramebuffers.begin(); iter != globVar.myVkSwapChainFramebuffers.end(); iter++)
    {
        vkDestroyFramebuffer(globVar.myVkLogicalDevice, *iter, nullptr);
    }
    if(globVar.myVkGraphicsPipeline)
        vkDestroyPipeline(globVar.myVkLogicalDevice, globVar.myVkGraphicsPipeline, nullptr);
    if(globVar.myVkPipelineLayout)
        vkDestroyPipelineLayout(globVar.myVkLogicalDevice, globVar.myVkPipelineLayout, nullptr);
    if(globVar.myVkRenderPass)
        vkDestroyRenderPass(globVar.myVkLogicalDevice, globVar.myVkRenderPass, nullptr);
    for(std::vector<VkImageView>::iterator iter = globVar.myVkSwapChainImageViews.begin(); iter != globVar.myVkSwapChainImageViews.end(); iter++)
    {
        vkDestroyImageView(globVar.myVkLogicalDevice, *iter, nullptr);
    }
    if(globVar.myVkSwapChain)
        vkDestroySwapchainKHR(globVar.myVkLogicalDevice, globVar.myVkSwapChain, nullptr);
    if(globVar.myVkSurface)
        vkDestroySurfaceKHR(globVar.myVkInstance, globVar.myVkSurface, nullptr);
    if(globVar.myVkLogicalDevice)
        vkDestroyDevice(globVar.myVkLogicalDevice, nullptr);
    if(globVar.myVkValLayersEnabled)
        DestroyDebugUtilsMessengerEXT(globVar.myVkInstance, globVar.myDebugMessager, nullptr);
    if(globVar.myVkInstance)
        vkDestroyInstance(globVar.myVkInstance, nullptr);
    if(globVar.myWindow)
        SDL_DestroyWindow(globVar.myWindow);
    SDL_Quit();
}

void fpsControl(Uint32* tNow, Uint32 *tPrev)
{
    *tNow = SDL_GetTicks();
    Uint32 tDelta = *tNow - *tPrev;
    if(tDelta < (1000 / FPS))
        SDL_Delay((Uint32)(1000 / FPS) - tDelta);
    *tPrev = SDL_GetTicks();
}