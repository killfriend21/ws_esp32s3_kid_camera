#pragma once
#include <Arduino_GFX_Library.h>

// Global GFX object – used by main.cpp for all drawing operations
extern Arduino_GFX *gfx;

void display_init();
void display_backlight(bool on);
