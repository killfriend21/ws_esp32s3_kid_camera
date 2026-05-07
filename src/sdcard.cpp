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
    // Pull CS high first so SD card is deselected during SPI init
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

    // Enable pull-up on MISO – some boards lack an external resistor;
    // a floating MISO causes CMD0/CMD8 responses to read as garbage.
    pinMode(LCD_MISO, INPUT_PULLUP);

    // Start SPI3 (HSPI) with our custom GPIO mapping.
    // Must use HSPI because GFX 1.4.0 already owns SPI3 on ESP32-S3.
    _sd_spi.begin(LCD_SCK, LCD_MISO, LCD_MOSI);
    delay(100);  // give card time to power up

    Serial.printf("[sd] init  HSPI=%d  SCK=%d MISO=%d MOSI=%d CS=%d\n",
                  HSPI, LCD_SCK, LCD_MISO, LCD_MOSI, SD_CS);

    // Try descending speeds: 4 MHz → 1 MHz (some cheap cards need slow init)
    static const uint32_t speeds[] = { 4000000UL, 1000000UL, 400000UL };
    for (uint32_t spd : speeds) {
        if (SD.begin(SD_CS, _sd_spi, spd)) {
            Serial.printf("[sd] mounted at %lu Hz\n", spd);
            goto sd_ok;
        }
        Serial.printf("[sd] mount failed at %lu Hz, retrying slower...\n", spd);
        SD.end();
        delay(200);
    }
    Serial.println("[sd] SD card mount failed – check card/format/contacts");
    return false;

sd_ok:

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

    // Ensure directory exists (might not on a brand-new card)
    if (!SD.exists(PHOTO_DIR)) {
        SD.mkdir(PHOTO_DIR);
    }

    File f = SD.open(path, FILE_WRITE);
    if (!f) {
        Serial.printf("[sd] Cannot open %s for writing  (dir exists: %d)\n",
                      path, SD.exists(PHOTO_DIR));
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
