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

bool initAll();
void destroyAll();
void renderFrame();
void pollEvents(bool *quit);
void fpsControl(Uint32* tNow, Uint32 *tPrev);