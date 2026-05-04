#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <JPEGDEC.h>

#include "config.h"
#include "display.h"
#include "touch.h"
#include "cam.h"
#include "sdcard.h"
#include "buzzer.h"
#include "wifi_server.h"

// ============================================================
//  Screen Layout  (portrait 240×320)
// ============================================================
//
//   y=  0 ┌──────────────────────────┐
//          │  Camera Preview          │  PREVIEW_H = 176 px
//          │  (HQVGA 240×176)         │  (OV2640 native ratio)
//   y=176 ├──────────────────────────┤
//          │  Info bar                │  INFO_H = 24 px
//   y=200 ├──────────────────────────┤
//          │  [PICS] [●SHOOT] [WiFi]  │  CTRL_H = 120 px
//   y=320 └──────────────────────────┘

#define PREVIEW_Y   0
#define PREVIEW_W   240
#define PREVIEW_H   176
#define INFO_Y      176
#define INFO_H      24
#define CTRL_Y      200
#define CTRL_H      120

// ============================================================
//  JPEG decoder  (JPEGDEC library by bitbank2)
// ============================================================
static JPEGDEC _jpeg;

static int _jpeg_draw_cb(JPEGDRAW *pDraw) {
    gfx->draw16bitRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels,
                             pDraw->iWidth, pDraw->iHeight);
    return 1;
}

// Draw a JPEG from SD card at (x,y).
// scale: 0=full, JPEG_SCALE_HALF=2, JPEG_SCALE_QUARTER=4, JPEG_SCALE_EIGHTH=8
static void draw_jpeg_from_sd(const char *path, int x, int y, int scale = 0) {
    File f = SD.open(path);
    if (!f) return;
    size_t sz = f.size();
    uint8_t *buf = (uint8_t *)ps_malloc(sz);
    if (buf) {
        f.read(buf, sz);
        f.close();
        if (_jpeg.openRAM(buf, sz, _jpeg_draw_cb)) {
            _jpeg.decode(x, y, scale);
            _jpeg.close();
        }
        free(buf);
    } else {
        f.close();
    }
}

// ============================================================
//  App State
// ============================================================
typedef enum {
    STATE_VIEWFINDER,
    STATE_CAPTURE,
    STATE_GALLERY,
    STATE_PHOTO_VIEW,
} AppState;

static AppState state     = STATE_VIEWFINDER;
static bool     wifi_on   = false;

// Gallery state
#define THUMBS_PER_PAGE  6          // 3 cols × 2 rows
#define THUMB_W         74
#define THUMB_H         74
static String gallery_files[MAX_GALLERY_FILES];
static int    gallery_total = 0;
static int    gallery_page  = 0;

// Photo-view state
static String photo_view_name = "";

// Dirty flags to avoid unnecessary redraws
static bool ctrl_dirty  = true;
static bool gallery_dirty = true;
static bool photo_dirty   = true;

// ============================================================
//  Touch debounce
// ============================================================
static uint32_t last_touch_ms = 0;
#define DEBOUNCE_MS 280

static bool debounced_touch(int16_t *x, int16_t *y) {
    if (!touch_read(x, y)) return false;
    uint32_t now = millis();
    if (now - last_touch_ms < DEBOUNCE_MS) return false;
    last_touch_ms = now;
    return true;
}

// ============================================================
//  UI helpers
// ============================================================
static void draw_button(int x, int y, int w, int h,
                        uint16_t bg, const char *label,
                        uint16_t fg = COLOR_WHITE, uint8_t size = 2) {
    gfx->fillRoundRect(x, y, w, h, 10, bg);
    gfx->setTextColor(fg);
    gfx->setTextSize(size);
    int tw = strlen(label) * 6 * size;
    int th = 8 * size;
    gfx->setCursor(x + (w - tw) / 2, y + (h - th) / 2);
    gfx->print(label);
}

// ============================================================
//  VIEWFINDER
// ============================================================
static void draw_ctrl_bar() {
    // Background
    gfx->fillRect(0, CTRL_Y, 240, CTRL_H, COLOR_DARK_GRAY);

    // [PICS] button  – left
    draw_button(4, CTRL_Y + 10, 70, 100, COLOR_PURPLE, "PICS");

    // [SHOOT] circle  – centre
    gfx->fillCircle(120, CTRL_Y + 60, 46, COLOR_LIGHT_GRAY);
    gfx->fillCircle(120, CTRL_Y + 60, 38, COLOR_RED);
    gfx->fillCircle(120, CTRL_Y + 60, 18, COLOR_WHITE);

    // [WiFi] button  – right
    uint16_t wifi_col = wifi_on ? COLOR_GREEN : COLOR_GRAY;
    draw_button(166, CTRL_Y + 10, 70, 100, wifi_col, "WiFi");

    ctrl_dirty = false;
}

static void draw_info_bar() {
    gfx->fillRect(0, INFO_Y, 240, INFO_H, COLOR_BG);

    // Photo count (left)
    gfx->setTextColor(COLOR_YELLOW);
    gfx->setTextSize(1);
    gfx->setCursor(4, INFO_Y + 8);
    gfx->printf("%d pics", sdcard_get_photo_count());

    // WiFi IP (right, only when active)
    if (wifi_on) {
        String ip = wifi_get_ip();
        gfx->setTextColor(COLOR_GREEN);
        gfx->setCursor(240 - ip.length() * 6 - 4, INFO_Y + 8);
        gfx->print(ip);
    }
}

static void handle_viewfinder_touch(int16_t tx, int16_t ty) {
    // -- PICS  x:4..74  y:CTRL_Y+10..CTRL_Y+110
    if (tx >= 4 && tx <= 74 && ty >= CTRL_Y + 10 && ty <= CTRL_Y + 110) {
        gallery_dirty = true;
        gallery_page  = 0;
        gallery_total = sdcard_list_photos(gallery_files, MAX_GALLERY_FILES);
        state = STATE_GALLERY;
        return;
    }

    // -- SHOOT  circle centre (120, CTRL_Y+60) r=46
    {
        int dx = tx - 120, dy = ty - (CTRL_Y + 60);
        if (dx*dx + dy*dy <= 46*46) {
            state = STATE_CAPTURE;
            return;
        }
    }

    // -- WiFi  x:166..236  y:CTRL_Y+10..CTRL_Y+110
    if (tx >= 166 && tx <= 236 && ty >= CTRL_Y + 10 && ty <= CTRL_Y + 110) {
        wifi_on = !wifi_on;
        if (wifi_on) wifi_server_start();
        else         wifi_server_stop();
        ctrl_dirty = true;
        return;
    }
}

static void update_viewfinder() {
    camera_fb_t *fb = cam_get_preview();
    if (fb) {
        if (_jpeg.openRAM(fb->buf, fb->len, _jpeg_draw_cb)) {
            _jpeg.decode(0, PREVIEW_Y, 0);
            _jpeg.close();
        }
        cam_return_fb(fb);
    }

    draw_info_bar();

    if (ctrl_dirty) draw_ctrl_bar();
}

// ============================================================
//  CAPTURE
// ============================================================
static void do_capture() {
    // Flash effect
    gfx->fillScreen(COLOR_WHITE);
    delay(40);

    // Save the current HQVGA frame as the file (copy to PSRAM first
    // so we can switch frame size without losing the buffer)
    camera_fb_t *prev = cam_get_preview();
    uint8_t     *thumb_data = nullptr;
    size_t       thumb_len  = 0;
    if (prev) {
        thumb_data = (uint8_t *)ps_malloc(prev->len);
        if (thumb_data) {
            memcpy(thumb_data, prev->buf, prev->len);
            thumb_len = prev->len;
        }
        cam_return_fb(prev);
    }

    // Shutter sound
    buzzer_shutter();

    // Capture high-resolution frame
    camera_fb_t *full = cam_capture();
    char filename[32] = "";

    if (full) {
        if (sdcard_save_photo(full->buf, full->len, filename, sizeof(filename))) {
            // Show a brief "Saved!" banner
            gfx->fillRect(0, 0, 240, 28, COLOR_GREEN);
            gfx->setTextColor(COLOR_WHITE);
            gfx->setTextSize(2);
            gfx->setCursor(8, 6);
            gfx->printf("Saved: %s", filename);
            ctrl_dirty = true;
            delay(900);
        }
        cam_return_fb(full);
    }

    if (thumb_data) free(thumb_data);

    state = STATE_VIEWFINDER;
}

// ============================================================
//  GALLERY
// ============================================================
#define GAL_HEADER_H  44
#define GAL_FOOTER_H  40
#define GAL_COLS       3
#define GAL_ROWS       2
#define GAL_PAD        5

static void draw_gallery() {
    gfx->fillScreen(COLOR_BG);

    // Header
    gfx->fillRect(0, 0, 240, GAL_HEADER_H, COLOR_PURPLE);
    draw_button(4, 7, 60, 30, COLOR_DARK_GRAY, "<Back");
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(80, 13);
    gfx->print("Gallery");
    // Photo count badge
    gfx->setTextSize(1);
    gfx->setCursor(200, 16);
    gfx->printf("%d", gallery_total);

    // Thumbnails area
    int start = gallery_page * THUMBS_PER_PAGE;
    for (int i = 0; i < THUMBS_PER_PAGE; i++) {
        int idx = start + i;
        int col = i % GAL_COLS;
        int row = i / GAL_COLS;
        int x   = col * (THUMB_W + GAL_PAD) + GAL_PAD;
        int y   = GAL_HEADER_H + row * (THUMB_H + GAL_PAD) + GAL_PAD;

        if (idx < gallery_total && !gallery_files[idx].isEmpty()) {
            char path[64];
            snprintf(path, sizeof(path), "%s/%s",
                     PHOTO_DIR, gallery_files[idx].c_str());
            if (SD.exists(path)) {
                draw_jpeg_from_sd(path, x, y, JPEG_SCALE_EIGHTH);
                // Thin border
                gfx->drawRect(x, y, THUMB_W, THUMB_H, COLOR_GRAY);
            } else {
                // Placeholder for missing file
                gfx->fillRect(x, y, THUMB_W, THUMB_H, COLOR_DARK_GRAY);
            }
        } else {
            // Empty slot
            gfx->fillRect(x, y, THUMB_W, THUMB_H, COLOR_DARK_GRAY);
            gfx->drawRect(x, y, THUMB_W, THUMB_H, COLOR_GRAY);
        }
    }

    // Footer
    int fy = LCD_HEIGHT - GAL_FOOTER_H;
    gfx->fillRect(0, fy, 240, GAL_FOOTER_H, COLOR_DARK_GRAY);

    int total_pages = (gallery_total + THUMBS_PER_PAGE - 1) / THUMBS_PER_PAGE;
    if (total_pages < 1) total_pages = 1;

    if (gallery_page > 0)
        draw_button(4, fy + 5, 70, 30, COLOR_BLUE, "<Prev");

    // Page indicator
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->setCursor(100, fy + 14);
    gfx->printf("%d / %d", gallery_page + 1, total_pages);

    if ((gallery_page + 1) * THUMBS_PER_PAGE < gallery_total)
        draw_button(166, fy + 5, 70, 30, COLOR_BLUE, "Next>");

    gallery_dirty = false;
}

static void handle_gallery_touch(int16_t tx, int16_t ty) {
    int fy = LCD_HEIGHT - GAL_FOOTER_H;

    // Back button
    if (tx >= 4 && tx <= 64 && ty >= 7 && ty <= 37) {
        state = STATE_VIEWFINDER;
        ctrl_dirty = true;
        return;
    }

    // Thumbnail tap
    for (int i = 0; i < THUMBS_PER_PAGE; i++) {
        int idx = gallery_page * THUMBS_PER_PAGE + i;
        if (idx >= gallery_total) break;

        int col = i % GAL_COLS;
        int row = i / GAL_COLS;
        int x   = col * (THUMB_W + GAL_PAD) + GAL_PAD;
        int y   = GAL_HEADER_H + row * (THUMB_H + GAL_PAD) + GAL_PAD;

        if (tx >= x && tx < x + THUMB_W && ty >= y && ty < y + THUMB_H) {
            photo_view_name  = gallery_files[idx];
            photo_dirty      = true;
            state            = STATE_PHOTO_VIEW;
            return;
        }
    }

    // Pagination
    if (ty >= fy) {
        if (tx < 120 && gallery_page > 0) {
            gallery_page--;
            gallery_dirty = true;
        } else if (tx >= 120 && (gallery_page + 1) * THUMBS_PER_PAGE < gallery_total) {
            gallery_page++;
            gallery_dirty = true;
        }
    }
}

// ============================================================
//  PHOTO VIEW
// ============================================================
#define PV_HEADER_H 44
#define PV_PHOTO_Y  PV_HEADER_H
#define PV_PHOTO_H  (LCD_HEIGHT - PV_HEADER_H)

static void draw_photo_view() {
    gfx->fillScreen(COLOR_BG);

    // Draw full photo
    char path[64];
    snprintf(path, sizeof(path), "%s/%s", PHOTO_DIR, photo_view_name.c_str());
    if (SD.exists(path)) {
        draw_jpeg_from_sd(path, 0, PV_PHOTO_Y, 0);
    }

    // Header overlay (semi-transparent via solid dark strip)
    gfx->fillRect(0, 0, 240, PV_HEADER_H, COLOR_BG);
    draw_button(4, 7, 60, 30, COLOR_DARK_GRAY, "<Back");

    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    // Truncate filename to fit
    String display_name = photo_view_name;
    if (display_name.length() > 16) display_name = display_name.substring(0, 16) + "..";
    gfx->setCursor(72, 18);
    gfx->print(display_name);

    draw_button(168, 7, 68, 30, COLOR_RED, "Delete");

    photo_dirty = false;
}

static void handle_photo_view_touch(int16_t tx, int16_t ty) {
    if (ty > PV_HEADER_H) return;  // only header area is interactive

    // Back
    if (tx >= 4 && tx <= 64) {
        gallery_dirty = true;
        state = STATE_GALLERY;
        return;
    }

    // Delete
    if (tx >= 168 && tx <= 236) {
        sdcard_delete_photo(photo_view_name.c_str());
        // Refresh gallery list
        gallery_total = sdcard_list_photos(gallery_files, MAX_GALLERY_FILES);
        if (gallery_page * THUMBS_PER_PAGE >= gallery_total && gallery_page > 0)
            gallery_page--;
        gallery_dirty = true;
        state = STATE_GALLERY;
        return;
    }
}

// ============================================================
//  Setup
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("\n[main] Kid Camera booting...");

    // Initialise display first (also starts the SPI bus)
    display_init();

    // Splash screen
    gfx->fillScreen(COLOR_ORANGE);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(3);
    gfx->setCursor(32, 110);
    gfx->print("KidCam!");
    gfx->setTextSize(2);
    gfx->setCursor(60, 160);
    gfx->print("Starting...");

    // Peripherals
    bool touch_ok = touch_init();
    bool cam_ok   = cam_init();
    bool sd_ok    = sdcard_init();
    buzzer_init();

    // Show any init errors on screen
    if (!touch_ok || !cam_ok || !sd_ok) {
        gfx->setTextSize(1);
        int ey = 220;
        if (!touch_ok) { gfx->setTextColor(COLOR_YELLOW); gfx->setCursor(10, ey); gfx->print("Touch FAIL"); ey += 14; }
        if (!cam_ok)   { gfx->setTextColor(COLOR_RED);    gfx->setCursor(10, ey); gfx->print("Camera FAIL"); ey += 14; }
        if (!sd_ok)    { gfx->setTextColor(COLOR_YELLOW); gfx->setCursor(10, ey); gfx->print("SD Card FAIL (photos disabled)"); }
        delay(2500);
    } else {
        delay(600);
        buzzer_shutter();  // startup chime
    }

    gfx->fillScreen(COLOR_BG);
    ctrl_dirty = true;
    Serial.println("[main] Ready");
}

// ============================================================
//  Loop
// ============================================================
void loop() {
    int16_t tx, ty;
    bool touched = debounced_touch(&tx, &ty);

    switch (state) {

        // ── Viewfinder ───────────────────────────────────────
        case STATE_VIEWFINDER:
            update_viewfinder();
            if (touched) handle_viewfinder_touch(tx, ty);
            break;

        // ── Capture ──────────────────────────────────────────
        case STATE_CAPTURE:
            do_capture();
            break;

        // ── Gallery ──────────────────────────────────────────
        case STATE_GALLERY:
            if (gallery_dirty) draw_gallery();
            if (touched) handle_gallery_touch(tx, ty);
            break;

        // ── Photo view ───────────────────────────────────────
        case STATE_PHOTO_VIEW:
            if (photo_dirty) draw_photo_view();
            if (touched) handle_photo_view_touch(tx, ty);
            break;
    }
}
