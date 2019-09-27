#include "main.hpp"

int main(int argc, char *argv[])
{
    if(!initAll())
        exit(1);
    bool quit = false;
    while(!quit)
    {
        pollEvents(&quit);
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
    const char *vkenames;
    SDL_Vulkan_GetInstanceExtensions(globVar.myWindow, &vkecount, &vkenames);
    // Set Vulkan Create Instance Info
    VkInstanceCreateInfo vkInstanceInfo;
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceInfo.enabledExtensionCount = vkecount;
    vkInstanceInfo.ppEnabledExtensionNames = (const char* const*) vkenames;
    vkInstanceInfo.pNext = NULL;
    vkInstanceInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceInfo.enabledLayerCount = 0;
    // Create Vulkan Instance
    if(vkCreateInstance(&vkInstanceInfo, nullptr, &globVar.myVkInstance) != VK_SUCCESS)
    {
        printf("Failed to create Vulkan instance\n");
        return false;
    }
    
    return true;
}

void destroyAll()
{
    if(globVar.myVkInstance)
        vkDestroyInstance(globVar.myVkInstance, nullptr);
    if(globVar.myWindow)
        SDL_DestroyWindow(globVar.myWindow);
    SDL_Quit();
}