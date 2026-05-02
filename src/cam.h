#pragma once
#include <esp_camera.h>

// Initialise the OV2640 sensor.  Returns false on error.
bool cam_init();

// Get a HQVGA (240×176) JPEG frame for live preview.
// Caller must call cam_return_fb() when done.
camera_fb_t *cam_get_preview();

// Capture a high-resolution (SVGA 800×600) JPEG for saving.
// Automatically switches frame size and back.
// Caller must call cam_return_fb() when done.
camera_fb_t *cam_capture();

// Return a frame buffer obtained from cam_get_preview() or cam_capture().
void cam_return_fb(camera_fb_t *fb);
