#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <set>

#include "globVar.hpp"
#include "debug.hpp"
#include "tools.hpp"

#define APP_NAME        "VkSelfLearn"

#define WINDOW_TITLE    "Vulkan Self-Learn"
#define WINDOW_W        800
#define WINDOW_H        600

#define FPS             144

#ifndef MY_ROOT_DIR
#define MY_ROOT_DIR     "."
#endif

bool initAll();
void destroyAll();
void pollEvents(bool *quit);
void fpsControl(Uint32* tNow, Uint32 *tPrev);