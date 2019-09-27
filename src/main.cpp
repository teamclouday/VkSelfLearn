#include "main.hpp"

GLOB_VARS globVar;

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
    }
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
                case SDLK_F11:
                {
                    bool isFullScreen = SDL_GetWindowFlags(globVar.myWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP;
#ifdef __unix__
                    SDL_SetWindowPosition(globVar.myWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif
                    SDL_SetWindowFullscreen(globVar.myWindow, isFullScreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
#ifdef _WIN32
                    if(isFullScreen)
                        SDL_SetWindowPosition(globVar.myWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#endif
                    break;
                }
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
    VkApplicationInfo vkAppInfo;
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
    VkInstanceCreateInfo vkInstanceInfo;
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
    VkDeviceQueueCreateInfo queueCreateInfo;
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
    queueCreateInfo.queueCount = 1;
    float queueCreatePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queueCreatePriority;
    queueCreateInfo.flags = 0;
    queueCreateInfo.pNext = nullptr;
    VkPhysicalDeviceFeatures deviceFeatures = {}; // Leave all features to VK_FALSE for now
    VkDeviceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0; // Device specific extension, leave to 0 for now
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
    return true;
}

void destroyAll()
{
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