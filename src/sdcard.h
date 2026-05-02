#pragma once
#include <Arduino.h>

// Mount the SD card on the shared SPI bus.  Call after display_init().
bool sdcard_init();

// Write a JPEG buffer to /photos/IMGxxxx.jpg.
// out_filename (e.g. "IMG0001.jpg") is written into the caller's buffer.
bool sdcard_save_photo(const uint8_t *data, size_t len,
                       char *out_filename, size_t filename_buf_len);

// Fill filenames[] with basenames of all .jpg files in /photos/.
// Returns the number of files found (capped at max_count).
int  sdcard_list_photos(String filenames[], int max_count);

// Delete /photos/<filename>.  Returns true on success.
bool sdcard_delete_photo(const char *filename);

// Count .jpg files in /photos/ (fast scan, no allocation).
int  sdcard_get_photo_count();
