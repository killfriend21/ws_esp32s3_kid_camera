#include "display.h"
#include "config.h"
#include <Arduino.h>

Arduino_GFX *gfx = nullptr;

void display_init() {
    // SPI bus: SCK=39, MISO=40, MOSI=38  (shared with SD card)
    Arduino_DataBus *bus = new Arduino_ESP32SPI(
        LCD_DC,   // DC
        LCD_CS,   // CS
        LCD_SCK,  // SCK
        LCD_MOSI, // MOSI
        LCD_MISO  // MISO
        // spi_num defaults to HSPI, is_shared_interface defaults to false
    );

    // ST7789T3  240×320 IPS, portrait (rotation=0)
    gfx = new Arduino_ST7789(
        bus,
        LCD_RST,  // RST (-1 = software reset via init sequence)
        0,        // rotation 0 = portrait
        true,     // IPS panel
        LCD_WIDTH,
        LCD_HEIGHT
    );

    gfx->begin(80000000UL);  // 80 MHz SPI

    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);  // backlight on

    gfx->fillScreen(BLACK);
}

void display_backlight(bool on) {
    digitalWrite(LCD_BL, on ? HIGH : LOW);
}
