#include "cam.h"
#include "config.h"
#include <Arduino.h>

static bool in_capture_mode = false;

bool cam_init() {
    camera_config_t cfg = {};

    cfg.ledc_channel  = LEDC_CHANNEL_1;
    cfg.ledc_timer    = LEDC_TIMER_1;

    // Data pins (D0–D7 mapped to Y2–Y9)
    cfg.pin_d0    = CAM_Y2;
    cfg.pin_d1    = CAM_Y3;
    cfg.pin_d2    = CAM_Y4;
    cfg.pin_d3    = CAM_Y5;
    cfg.pin_d4    = CAM_Y6;
    cfg.pin_d5    = CAM_Y7;
    cfg.pin_d6    = CAM_Y8;
    cfg.pin_d7    = CAM_Y9;

    cfg.pin_xclk      = CAM_XCLK;
    cfg.pin_pclk      = CAM_PCLK;
    cfg.pin_vsync     = CAM_VSYNC;
    cfg.pin_href      = CAM_HREF;
    cfg.pin_sccb_sda  = CAM_SIOD;
    cfg.pin_sccb_scl  = CAM_SIOC;
    cfg.pin_pwdn      = CAM_PWDN;
    cfg.pin_reset     = CAM_RESET;

    cfg.xclk_freq_hz  = 20000000;
    cfg.pixel_format  = PIXFORMAT_JPEG;
    // Allocate frame buffers for the LARGEST resolution we will ever use (SVGA
    // 800×600).  If we init with HQVGA, the PSRAM buffers are too small for an
    // SVGA JPEG and cam_capture() returns NULL.  HQVGA frames fit fine in an
    // SVGA-sized buffer, so preview is unaffected.
    cfg.frame_size    = FRAMESIZE_SVGA;
    cfg.jpeg_quality  = 10;
    cfg.fb_count      = 2;               // double-buffer for smooth preview
    cfg.grab_mode     = CAMERA_GRAB_LATEST; // always return the newest frame
    cfg.fb_location   = CAMERA_FB_IN_PSRAM; // use 8 MB PSRAM

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("[cam] init failed: 0x%x\n", err);
        return false;
    }

    // Sensor tweaks + switch to HQVGA for live preview
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s,  0);
        s->set_saturation(s,  0);
        s->set_whitebal(s,    1);
        s->set_awb_gain(s,    1);
        s->set_wb_mode(s,     0);
        s->set_exposure_ctrl(s, 1);
        s->set_aec2(s,        1);
        s->set_gain_ctrl(s,   1);
        s->set_agc_gain(s,    0);
        s->set_bpc(s,         0);
        s->set_wpc(s,         1);
        s->set_raw_gma(s,     1);
        s->set_lenc(s,        1);
        s->set_hmirror(s,     0);
        s->set_vflip(s,       0);
        s->set_colorbar(s,    0);
        // Switch down to HQVGA for the live preview loop
        s->set_framesize(s, FRAMESIZE_HQVGA);
        s->set_quality(s, 12);
    }

    Serial.println("[cam] OV2640 OK  (buffers=SVGA, preview=HQVGA)");
    return true;
}

camera_fb_t *cam_get_preview() {
    // If we were in capture mode, restore preview frame size
    if (in_capture_mode) {
        sensor_t *s = esp_camera_sensor_get();
        if (s) {
            s->set_framesize(s, FRAMESIZE_HQVGA);
            s->set_quality(s, 12);
        }
        // Drop one stale frame after resize
        camera_fb_t *dummy = esp_camera_fb_get();
        if (dummy) esp_camera_fb_return(dummy);
        in_capture_mode = false;
    }
    return esp_camera_fb_get();
}

camera_fb_t *cam_capture() {
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_framesize(s, FRAMESIZE_SVGA);  // 800×600
        s->set_quality(s, 10);               // higher quality for saved photo
    }
    in_capture_mode = true;

    // Drop one frame so the sensor flushes the old HQVGA frame from its DMA buffer.
    // (Two drops was slower with no meaningful image quality improvement.)
    camera_fb_t *dummy = esp_camera_fb_get();
    if (dummy) esp_camera_fb_return(dummy);

    return esp_camera_fb_get();
}

void cam_return_fb(camera_fb_t *fb) {
    if (fb) esp_camera_fb_return(fb);
}
