#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <stdlib.h>

#define APP_NAME        "VkSelfLearn"
#define WINDOW_TITLE    "Vulkan Self-Learn"
#define WINDOW_W        800
#define WINDOW_H        600

#ifndef MY_ROOT_DIR
#define MY_ROOT_DIR     "."
#endif

bool initAll();
void destroyAll();
void pollEvents(bool *quit);

struct GLOB_VARS
{
    SDL_Window *myWindow = nullptr;
    VkInstance myVkInstance = nullptr;
} globVar;