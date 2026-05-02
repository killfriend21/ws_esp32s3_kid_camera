#include "touch.h"
#include "config.h"
#include <Wire.h>
#include <Arduino.h>

// CST816D I2C register map
// 0x01  Gesture  (0=none, 1=up, 2=down, 3=left, 4=right, 5=click)
// 0x02  Finger count
// 0x03  XH  [7:6]=event  [3:0]=X[11:8]
// 0x04  XL  X[7:0]
// 0x05  YH  [7:4]=touchID  [3:0]=Y[11:8]
// 0x06  YL  Y[7:0]

bool touch_init() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(400000);

    // Probe device
    Wire.beginTransmission(TOUCH_ADDR);
    if (Wire.endTransmission() != 0) {
        Serial.printf("[touch] CST816D not found at 0x%02X\n", TOUCH_ADDR);
        return false;
    }
    Serial.println("[touch] CST816D OK");
    return true;
}

bool touch_read(int16_t *x, int16_t *y) {
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(0x01);  // start at gesture register
    if (Wire.endTransmission(false) != 0) return false;

    if (Wire.requestFrom((uint8_t)TOUCH_ADDR, (uint8_t)6) < 6) return false;

    /* uint8_t gesture = */ Wire.read();  // 0x01
    uint8_t fingers  = Wire.read();       // 0x02
    uint8_t xh       = Wire.read() & 0x0F; // 0x03  (mask event bits)
    uint8_t xl       = Wire.read();         // 0x04
    uint8_t yh       = Wire.read() & 0x0F; // 0x05  (mask touch-ID bits)
    uint8_t yl       = Wire.read();         // 0x06

    if (fingers == 0) return false;

    *x = (int16_t)((xh << 8) | xl);
    *y = (int16_t)((yh << 8) | yl);

    // Clamp to screen bounds
    if (*x < 0)           *x = 0;
    if (*x >= LCD_WIDTH)  *x = LCD_WIDTH  - 1;
    if (*y < 0)           *y = 0;
    if (*y >= LCD_HEIGHT) *y = LCD_HEIGHT - 1;

    return true;
}
