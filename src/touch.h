#pragma once
#include <stdint.h>
#include <stdbool.h>

bool touch_init();

// Returns true when at least one finger is detected.
// x, y are set to screen coordinates (clamped to screen size).
bool touch_read(int16_t *x, int16_t *y);
