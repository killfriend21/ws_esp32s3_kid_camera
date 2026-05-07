#include "sdcard.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>

// GFX 1.4.0 on ESP32-S3 drives the SPI bus via internal HAL (HSPI/SPI3)
// without touching the global SPI (FSPI/SPI2) object.  We must explicitly
// create an SPIClass on the same host and attach the shared GPIO pins.
static SPIClass _sd_spi(HSPI);

static int photo_counter = 0;  // highest IMG number seen so far

bool sdcard_init() {
    _sd_spi.begin(LCD_SCK, LCD_MISO, LCD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, _sd_spi, 40000000UL)) {
        Serial.println("[sd] SD card mount failed");
        return false;
    }

    // Create photos directory if it doesn't exist yet
    if (!SD.exists(PHOTO_DIR)) {
        if (!SD.mkdir(PHOTO_DIR)) {
            Serial.println("[sd] Failed to create /photos");
        }
    }

    // Scan for existing photos to initialise the counter
    photo_counter = 0;
    File dir = SD.open(PHOTO_DIR);
    if (dir) {
        File entry = dir.openNextFile();
        while (entry) {
            if (!entry.isDirectory()) {
                String name = String(entry.name());
                // Expect basename like "IMG0001.jpg"
                if (name.startsWith("IMG") && name.length() >= 7) {
                    int num = name.substring(3, 7).toInt();
                    if (num > photo_counter) photo_counter = num;
                }
            }
            entry.close();
            entry = dir.openNextFile();
        }
        dir.close();
    }

    uint64_t total = SD.totalBytes() / (1024 * 1024);
    uint64_t used  = SD.usedBytes()  / (1024 * 1024);
    Serial.printf("[sd] OK  %llu MB total / %llu MB used  (%d photos)\n",
                  total, used, photo_counter);
    return true;
}

bool sdcard_save_photo(const uint8_t *data, size_t len,
                       char *out_filename, size_t filename_buf_len) {
    photo_counter++;
    snprintf(out_filename, filename_buf_len, "IMG%04d.jpg", photo_counter);

    char path[64];
    snprintf(path, sizeof(path), "%s/%s", PHOTO_DIR, out_filename);

    File f = SD.open(path, FILE_WRITE);
    if (!f) {
        Serial.printf("[sd] Cannot open %s for writing\n", path);
        photo_counter--;
        return false;
    }

    size_t written = f.write(data, len);
    f.close();

    if (written != len) {
        Serial.printf("[sd] Write incomplete (%u / %u bytes)\n", written, len);
        return false;
    }

    Serial.printf("[sd] Saved %s  (%u bytes)\n", path, len);
    return true;
}

int sdcard_list_photos(String filenames[], int max_count) {
    File dir = SD.open(PHOTO_DIR);
    if (!dir) return 0;

    int count = 0;
    File entry = dir.openNextFile();
    while (entry && count < max_count) {
        if (!entry.isDirectory()) {
            String name = String(entry.name());
            if (name.endsWith(".jpg") || name.endsWith(".JPG")) {
                filenames[count++] = name;
            }
        }
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();
    return count;
}

bool sdcard_delete_photo(const char *filename) {
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", PHOTO_DIR, filename);
    bool ok = SD.remove(path);
    if (ok) Serial.printf("[sd] Deleted %s\n", path);
    return ok;
}

int sdcard_get_photo_count() {
    return photo_counter;
}
