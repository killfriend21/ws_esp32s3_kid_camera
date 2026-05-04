# Kid Camera 📷

A fun and interactive camera system for kids built with **Waveshare ESP32-S3-Touch-LCD-2** and Arduino. Take photos, view them in a gallery, and access them via WiFi web browser!

## Features

✅ **Live Camera Preview** – Real-time 240×176 JPEG preview on 2-inch LCD display  
✅ **Photo Capture** – Touch the big red button to snap 800×600 high-quality photos  
✅ **Gallery View** – Browse thumbnails in a scrollable 3×2 grid with pagination  
✅ **Full Photo View** – Tap any thumbnail to see the photo full-screen  
✅ **Photo Management** – Delete unwanted photos right from the gallery  
✅ **Shutter Sound** – Satisfying camera click sound (requires external buzzer)  
✅ **WiFi Web Gallery** – Access photos from any device on the same WiFi network  
✅ **SD Card Storage** – All photos saved to onboard SD card slot  

## Hardware

### Board
- **Waveshare ESP32-S3-Touch-LCD-2**
  - Processor: ESP32-S3-WROOM-1 (N8R8)
  - Flash: 8 MB (QIO)
  - PSRAM: 8 MB (OPI)
  - Display: 240×320 IPS ST7789T3
  - Touch: CST816D capacitive touch
  - USB: Type-C (CDC serial + power)

### Camera Module
- **OV2640** (sold separately)
  - Connects to onboard 24-pin CSI camera connector
  - HQVGA (240×176) for live preview, SVGA (800×600) for capture
  - JPEG compression built-in

### Optional Hardware
- **Passive Piezo Buzzer** – Connect between GPIO18 and GND for shutter sound
- **MicroSD Card** – Up to 32 GB, FAT32 formatted

## Pin Assignments

| Peripheral | Signal | GPIO | Notes |
|---|---|---|---|
| **LCD** | DC | 42 | SPI command/data |
| | CS | 45 | SPI chip select |
| | SCK | 39 | SPI clock (shared with SD) |
| | MOSI | 38 | SPI data out (shared with SD) |
| | MISO | 40 | SPI data in (shared with SD) |
| | RST | -1 | Software reset only |
| | BL | 1 | Backlight (active HIGH) |
| **Touch** | SDA | 48 | I²C data |
| | SCL | 47 | I²C clock |
| | INT | 46 | Interrupt (unused) |
| **SD Card** | CS | 41 | SPI chip select |
| **Camera** | XCLK | 8 | External clock |
| | SIOD | 21 | I²C SDA (camera config) |
| | SIOC | 16 | I²C SCL (camera config) |
| | Y9–Y2 | 2,7,10,14,11,15,13,12 | Data pins (D7–D0) |
| | VSYNC | 6 | Vertical sync |
| | HREF | 4 | Horizontal sync |
| | PCLK | 9 | Pixel clock |
| | PWDN | 17 | Power down |
| **Buzzer** | PWM | 18 | Shutter sound (optional) |

> **SPI Bus Sharing:** LCD (CS=45) and SD card (CS=41) share the same SPI bus (SCK/MOSI/MISO). `display_init()` must be called before `sdcard_init()` so that GFX starts the bus first.

## Getting Started

### 1. Install PlatformIO
   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install the **PlatformIO IDE** extension from the VS Code marketplace

### 2. Clone the Project
   ```bash
   git clone https://github.com/killfriend21/ws_esp32s3_kid_camera.git
   cd ws_esp32s3_kid_camera
   ```

### 3. Connect Hardware
   - Plug the **OV2640 camera module** into the CSI connector (24-pin ribbon cable)
   - Insert a **microSD card** into the SD slot (format as FAT32)
   - (Optional) Connect a **passive piezo buzzer** between GPIO18 and GND
   - Connect the board to your computer via USB-C

### 4. Build and Upload
   ```bash
   # via CLI
   pio run -e ws_esp32s3_touch_lcd2 --target upload

   # or in VS Code: click the Upload (→) button in the PlatformIO status bar
   ```
   First build downloads libraries and takes ~2–3 minutes.

### 5. Serial Monitor (Optional)
   ```bash
   pio device monitor -b 115200
   ```

## Usage

### Viewfinder Screen
```
┌──────────────────────────┐
│   Camera Preview         │  y = 0..175  (HQVGA 240×176)
│        240×176           │
├──────────────────────────┤
│  12 pics       192.168.4.1│  y = 176..199  Info bar
├──────────────────────────┤
│  [PICS]  [●SHOOT]  [WiFi]│  y = 200..319  Control bar
└──────────────────────────┘
```

| Button | Action |
|---|---|
| **[PICS]** | Open gallery |
| **[●SHOOT]** | Take a 800×600 photo |
| **[WiFi]** | Toggle WiFi AP on/off (green = on) |

### Gallery Screen
- **3×2 thumbnail grid** (6 photos per page)
- Tap a thumbnail → full photo view
- **< Prev** / **Next >** for pagination
- **< Back** → viewfinder

### Photo View Screen
- Full-screen JPEG display
- **< Back** → gallery
- **Delete** → removes photo from SD card

### WiFi Web Gallery
1. Tap **[WiFi]** — button turns green, IP shows in info bar
2. On your phone/computer, connect to:
   - **SSID:** `KidCamera`  **Password:** `12345678`
3. Open browser → `http://192.168.4.1`
4. Browse, download, or delete photos from any browser

## Libraries

Auto-installed by PlatformIO from `platformio.ini`:

| Library | Version | Purpose |
|---|---|---|
| `moononournation/GFX Library for Arduino` | **1.4.0** (pinned) | ST7789 SPI display driver |
| `bitbank2/JPEGDEC` | ^1.2.0 | JPEG decode for preview & gallery |
| `ESP32Async/ESPAsyncWebServer` | ^3.7.6 | Async HTTP server (WiFi gallery) |
| `ESP32Async/AsyncTCP` | ^3.3.6 | TCP layer for async server |
| `esp_camera` | built-in | OV2640 camera (ESP32 Arduino core) |
| `SD` / `SPI` / `Wire` | built-in | SD card, SPI bus, I²C |

> **Why GFX 1.4.0?** Versions ≥ 1.4.1 require `esp32-hal-periman.h` and `esp_rgb_panel_t`
> which are only available in Arduino ESP32 3.x / ESP-IDF 5.x. This project targets
> `espressif32 @ 5.3.0` (Arduino ESP32 2.0.6 / ESP-IDF 4.4.x), so 1.4.0 is the
> highest compatible version.

> **Why JPEGDEC?** `gfx->drawJpg()` / `gfx->drawJpgFile()` were removed from GFX
> starting at 1.4.0. JPEGDEC replaces the built-in TJpgDec decoder.

## JPEG / Camera Pipeline

### Live Preview (every loop frame)
```
OV2640 → HQVGA JPEG frame → JPEGDEC::openRAM() → decode(0, 0, 0) → gfx→draw16bitRGBBitmap()
```

### Photo Capture (tap SHOOT)
```
OV2640 → SVGA 800×600 JPEG → SD.open(FILE_WRITE) → /photos/IMGxxxx.jpg → buzzer_shutter()
```

### Gallery Thumbnail
```
SD /photos/IMGxxxx.jpg → ps_malloc(PSRAM) → JPEGDEC::openRAM() → decode(x, y, JPEG_SCALE_EIGHTH)
```
`JPEG_SCALE_EIGHTH = 8` → 800×600 ÷ 8 = 100×75 px, fits the 74×74 slot.

### Full Photo View
```
SD /photos/IMGxxxx.jpg → ps_malloc(PSRAM) → JPEGDEC::openRAM() → decode(0, PV_PHOTO_Y, 0)
```

## State Machine

```
BOOT: display_init → touch_init → cam_init → sdcard_init → buzzer_init → splash
        │
        ▼
STATE_VIEWFINDER ◄────────────────────────────────────────┐
   preview loop                                           │
   ├─ tap SHOOT  ──► STATE_CAPTURE ──► save+buzz ──────── ┤
   ├─ tap PICS   ──► STATE_GALLERY                        │
   │                    ├─ tap thumb ──► STATE_PHOTO_VIEW  │
   │                    │                  ├─ Back ────── ►┤ (gallery)
   │                    │                  └─ Delete ─────►┤ (gallery)
   │                    └─ Back ───────────────────────── ►┘
   └─ tap WiFi   ──► toggle AP on/off (stays in VIEWFINDER)
```

## Project Structure

```
ws_esp32s3_kid_camera/
├── platformio.ini               # Build config (platform, libs, flags)
├── README.md                    # This file
├── KidCamera_Architecture.pptx  # Architecture documentation (8 slides)
└── src/
    ├── config.h             # All pin definitions & app constants
    ├── display.h / .cpp     # ST7789 via Arduino_GFX, exposes gfx*
    ├── touch.h / .cpp       # CST816D I²C touch driver
    ├── cam.h / .cpp         # OV2640 camera (preview + capture)
    ├── sdcard.h / .cpp      # SD card: save / list / delete photos
    ├── buzzer.h / .cpp      # tone() shutter sound on GPIO18
    ├── wifi_server.h / .cpp # AsyncWebServer AP + HTML gallery
    └── main.cpp             # State machine orchestration + JPEG decode
```

## Performance

| Metric | Value |
|---|---|
| Camera preview | ~8–10 fps (HQVGA JPEG decode) |
| Photo capture | ~1–2 s shutter → SD written |
| Gallery page load | < 1 s |
| RAM usage | ~21% (70 KB / 320 KB) |
| Flash usage | ~27% (914 KB / 3.3 MB) |

## Customization

### Change WiFi Credentials
`src/config.h`:
```cpp
#define WIFI_AP_SSID  "MyCamera"
#define WIFI_AP_PASS  "mypassword"
```

### Change Camera Resolution / Quality
`src/cam.cpp`:
```cpp
cfg.frame_size   = FRAMESIZE_XGA;  // 1024×768 capture
cfg.jpeg_quality = 8;              // lower = better quality
```

### Change UI Colors
`src/config.h`:
```cpp
#define COLOR_BG    0x0000   // Black background
#define COLOR_RED   0xF800   // Shoot button
#define COLOR_GREEN 0x07E0   // WiFi active
```

### Add a New Screen / State
`src/main.cpp`:
```cpp
// 1. Add to enum
typedef enum { ..., STATE_SETTINGS } AppState;

// 2. Add draw + touch handler functions
static void draw_settings()          { /* draw your UI */ }
static void handle_settings_touch()  { /* handle input */ }

// 3. Add case in loop()
case STATE_SETTINGS:
    if (settings_dirty) draw_settings();
    if (touched) handle_settings_touch();
    break;
```

## Troubleshooting

| Symptom | Check |
|---|---|
| "Camera FAIL" on boot | OV2640 ribbon fully seated and locked |
| "SD Card FAIL" on boot | FAT32 format, card clicked in |
| "Touch FAIL" on boot | I²C wires: SDA=48, SCL=47 |
| No shutter sound | Normal without buzzer; connect passive piezo to GPIO18+GND |
| Gallery shows blank tiles | SD path `/photos/` must exist (auto-created on first boot) |
| WiFi not found | Tap [WiFi] button (must turn green first) |
| Build fails | Run `pio pkg update` then retry; ensure espressif32 @ 5.3.0 |

## Known Limitations

- WiFi is Access Point only (no STA mode) — one client at a time
- Maximum 200 photos indexed in RAM (all are on SD; restart clears index)
- No USB camera streaming (USB-CDC used for serial debug only)
- No image editing or filters
- WiFi and camera cannot run simultaneously at full speed (shared CPU)

## License

This project uses open-source Arduino libraries. See individual library licenses.

---

For board schematics and datasheets see the [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2).  
For ESP32 Arduino reference see [Arduino ESP32 Docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/).

Happy snapping! 📷✨
