#pragma once

// ============================================================
//  Waveshare ESP32-S3-Touch-LCD-2  –  Pin Definitions
//  (verified from CircuitPython pins.c + official wiki)
// ============================================================

// ---- LCD  ST7789T3  (SPI) ----------------------------------
#define LCD_DC      42
#define LCD_CS      45
#define LCD_SCK     39
#define LCD_MOSI    38
#define LCD_MISO    40
#define LCD_RST     -1    // GPIO0 is also BOOT btn; use software reset
#define LCD_BL       1    // Backlight (active HIGH)
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// ---- Touch  CST816D  (I2C) ---------------------------------
#define TOUCH_SDA   48
#define TOUCH_SCL   47
#define TOUCH_INT   46
#define TOUCH_RST   -1
#define TOUCH_ADDR  0x15

// ---- SD Card  (SPI, shared bus with LCD) -------------------
#define SD_CS       41
// SCK / MOSI / MISO same as LCD SPI bus

// ---- Camera  OV2640  (CSI) ---------------------------------
#define CAM_PWDN    17
#define CAM_RESET   -1
#define CAM_XCLK     8
#define CAM_SIOD    21    // Camera I2C SDA  (separate from touch I2C)
#define CAM_SIOC    16    // Camera I2C SCL
#define CAM_Y9       2    // D7
#define CAM_Y8       7    // D6
#define CAM_Y7      10    // D5
#define CAM_Y6      14    // D4
#define CAM_Y5      11    // D3
#define CAM_Y4      15    // D2
#define CAM_Y3      13    // D1
#define CAM_Y2      12    // D0
#define CAM_VSYNC    6
#define CAM_HREF     4
#define CAM_PCLK     9

// ---- Buzzer  (optional – connect passive piezo to GPIO18) --
#define BUZZER_PIN  18

// ---- Other -------------------------------------------------
#define BOOT_BTN     0    // Also LCD_RST on schematic
#define BATT_ADC     5
#define IMU_INT      3

// ============================================================
//  Application Constants
// ============================================================
#define PHOTO_DIR        "/photos"
#define MAX_GALLERY_FILES 200    // max photos tracked in RAM

#define WIFI_AP_SSID     "KidCamera"
#define WIFI_AP_PASS     "12345678"
#define WIFI_PORT        80

// ============================================================
//  UI Colors  (RGB565)
// ============================================================
#define COLOR_BG         0x0000   // Black
#define COLOR_WHITE      0xFFFF
#define COLOR_YELLOW     0xFFE0
#define COLOR_ORANGE     0xFD20
#define COLOR_GREEN      0x07E0
#define COLOR_RED        0xF800
#define COLOR_BLUE       0x001F
#define COLOR_CYAN       0x07FF
#define COLOR_PINK       0xFC18
#define COLOR_PURPLE     0x8010
#define COLOR_DARK_GRAY  0x39E7
#define COLOR_GRAY       0x7BEF
#define COLOR_LIGHT_GRAY 0xC618
