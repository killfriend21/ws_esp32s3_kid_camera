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
  - Flash: 8 MB
  - PSRAM: 8 MB (OPI)
  - Display: 240×320 IPS ST7789T3
  - Touch: CST816D capacitive touch
  - USB: Type-C (CDC serial + power)

### Camera Module
- **OV2640** (sold separately)
  - Connects to onboard 24-pin CSI camera connector
  - Supports QQVGA (160×120) to XGA (1024×768) resolutions
  - JPEG compression built-in

### Optional Hardware
- **Passive Piezo Buzzer** (connected to GPIO18 + GND) for shutter sound
- **MicroSD Card** (up to 32GB, FAT32 formatted)

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
| **Touch** | SDA | 48 | I2C data |
| | SCL | 47 | I2C clock |
| | INT | 46 | Interrupt (unused) |
| **SD Card** | CS | 41 | SPI chip select |
| **Camera** | XCLK | 8 | External clock |
| | SIOD | 21 | I2C SDA (camera config) |
| | SIOC | 16 | I2C SCL (camera config) |
| | Y9–Y2 | 2,7,10,14,11,15,13,12 | Data pins (D7–D0) |
| | VSYNC | 6 | Vertical sync |
| | HREF | 4 | Horizontal sync |
| | PCLK | 9 | Pixel clock |
| | PWDN | 17 | Power down |
| **Buzzer** | PWM | 18 | Shutter sound (optional) |

## Getting Started

### 1. **Install PlatformIO**
   - Install [Visual Studio Code](https://code.visualstudio.com/)
   - Install the PlatformIO IDE extension from the VS Code marketplace

### 2. **Clone / Setup Project**
   ```bash
   # Open the project folder in VS Code
   # or create it from this directory
   cd ws_esp32s3_kid_camera
   ```

### 3. **Connect Hardware**
   - Plug the **OV2640 camera module** into the CSI connector (24-pin ribbon cable)
   - Insert a **microSD card** into the SD slot (format as FAT32 if needed)
   - (Optional) Connect a **piezo buzzer** between GPIO18 and GND
   - Connect the board to your computer via USB-C cable

### 4. **Build and Upload**
   - Press `Ctrl+Shift+P` (or `Cmd+Shift+P` on Mac)
   - Type "PlatformIO: Upload" and press Enter
   - Or click the **Upload** button in the PlatformIO status bar
   - Wait for compilation and flashing to complete (~1–2 minutes for first build)

### 5. **Serial Monitor** (Optional)
   - Press `Ctrl+Shift+P` and type "PlatformIO: Serial Monitor"
   - Or click the Serial Monitor icon in the status bar
   - Baud rate is 115200

## Usage

### Viewfinder Screen
```
┌─────────────────────┐
│   Camera Preview    │
│     (240×176)       │
├─────────────────────┤
│ Photo count | WiFi IP│
├─────────────────────┤
│ [PICS]  [●]  [WiFi] │  ← Touch to interact
└─────────────────────┘
```

**Controls:**
- **[PICS]** – Open gallery to browse saved photos
- **[●]** – Take a photo (large red circle in center)
- **[WiFi]** – Toggle WiFi access point on/off

### Gallery Screen
- **3×2 thumbnail grid** with up to 6 photos per page
- Tap a thumbnail to view full-screen
- Use **< Prev** and **Next >** buttons to navigate pages
- **< Back** returns to viewfinder

### Photo View Screen
- Full-screen photo display
- **< Back** returns to gallery
- **Delete** button removes the photo

### WiFi Web Gallery
1. Tap **[WiFi]** to enable WiFi (button turns green)
2. Note the IP address shown in the top-right corner (e.g., `192.168.4.1`)
3. On your phone/computer, connect to WiFi network:
   - **SSID:** `KidCamera`
   - **Password:** `12345678`
4. Open a web browser and visit: `http://192.168.4.1`
5. Browse all photos with download and delete buttons

## Libraries

The project uses these open-source libraries (auto-installed by PlatformIO):

- **GFX Library for Arduino** – LCD display driver + JPEG decoder (TJpgDec)
- **ESPAsyncWebServer** – Lightweight web server for photo gallery
- **AsyncTCP** – TCP/IP library for async web server
- **esp_camera** – Built-in ESP32 camera driver
- **SD** / **SPI** / **Wire** – Built-in Arduino libraries

## Screen Layout

**Preview Area (y=0–175):**
- HQVGA (240×176) JPEG camera feed
- Updated every ~100ms for smooth preview

**Info Bar (y=176–199):**
- Photo count (left)
- WiFi IP address when active (right)

**Control Bar (y=200–319):**
- Gallery, Capture, and WiFi buttons
- Touch-responsive button areas

## Storage

Photos are stored on the SD card in:
```
/photos/IMG0001.jpg
/photos/IMG0002.jpg
...
```

**Typical storage:**
- One SVGA JPEG (800×600): 50–150 KB
- microSD 32GB = ~200,000+ photos
- Filenames auto-increment from IMG0001 onwards

## Troubleshooting

### Camera not working
- ✓ Check OV2640 module is properly seated in CSI connector
- ✓ Ensure ribbon cable is fully inserted and locked
- ✓ Look for "Camera FAIL" message on boot splash screen
- ✓ Try replugging the camera module

### SD card errors
- ✓ Insert SD card into slot (should click into place)
- ✓ Format SD card as FAT32 before first use
- ✓ Check for "SD Card FAIL" message (photos will be disabled)

### Touch not responding
- ✓ Tap gently on the LCD screen
- ✓ Check I2C wires (SDA=GPIO48, SCL=GPIO47)
- ✓ Look for "Touch FAIL" on boot

### WiFi not connecting
- ✓ Tap [WiFi] button (should turn green when enabled)
- ✓ Check that both board and phone are on 2.4 GHz band (5 GHz not supported)
- ✓ Try resetting board if connection issues persist

### No shutter sound
- ✓ This is normal if buzzer not connected
- ✓ To add sound, connect passive piezo buzzer between GPIO18 and GND
- ✓ Or solder directly to the board pads

### Compile errors
- ✓ First build is slower; wait 2–3 minutes
- ✓ Try pressing **Rebuild** (not just Upload)
- ✓ Check PlatformIO version is latest (auto-updates)

## Building from Source

### Prerequisites
- PlatformIO CLI or VS Code extension
- Python 3.6+ (comes with PlatformIO)
- ~500 MB free disk space

### Compile
```bash
platformio run -e ws_esp32s3_touch_lcd2
```

### Upload
```bash
platformio run -e ws_esp32s3_touch_lcd2 --target upload
```

### Monitor Serial Output
```bash
platformio device monitor -b 115200
```

## Project Structure

```
ws_esp32s3_kid_camera/
├── platformio.ini           # Build configuration
├── README.md                # This file
└── src/
    ├── main.cpp             # Main app (state machine + UI)
    ├── config.h             # Pin definitions & constants
    ├── display.h/cpp        # LCD driver (Arduino_GFX)
    ├── touch.h/cpp          # CST816D touch driver
    ├── cam.h/cpp            # OV2640 camera driver
    ├── sdcard.h/cpp         # SD card operations
    ├── buzzer.h/cpp         # Shutter sound
    └── wifi_server.h/cpp    # Web server & gallery HTML
```

## Performance

- **Camera preview:** ~8–10 fps (HQVGA JPEG decode)
- **Photo capture:** 1–2 seconds from shutter to SD
- **Gallery load:** <1 second for first page
- **Web gallery:** Instant thumbnail streaming via WiFi

## Customization

### Change WiFi Credentials
Edit `src/config.h`:
```cpp
#define WIFI_AP_SSID     "MyCamera"
#define WIFI_AP_PASS     "mypassword"
```

### Adjust Camera Quality
In `src/cam.cpp`, modify JPEG quality (1–100):
```cpp
s->set_quality(s, 12);  // 1=low, 12=high
```

### Change Button Colors
In `src/config.h`:
```cpp
#define COLOR_RED        0xF800  // RGB565
#define COLOR_GREEN      0x07E0
#define COLOR_BLUE       0x001F
```

## Known Limitations

- No USB camera streaming (USB CDC only for serial debug)
- WiFi range ~10–20m typical indoor
- Maximum 200 photos stored in gallery RAM (but all are on SD card)
- No image editing or filters
- Single WiFi client at a time

## License

This project uses open-source Arduino libraries. See individual library licenses.

## Tips & Tricks

**Pro Tips for Kids:**
- 📸 Take lots of photos and delete the bad ones later
- 🎨 Hold camera at different angles for creative shots
- 🌞 Better lighting = better photos
- 🌐 Share WiFi photos with family easily

**Power Management:**
- USB cable provides power directly
- Onboard 3.7V battery connector available (not populated)
- Backlight always on during use

**SD Card Tips:**
- Use Class 10 microSD for best performance
- Format new cards as FAT32 before use
- Maximum ~32 GB recommended

---

**Questions?** Check the [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2) for board details or [Arduino Docs](https://docs.arduino.cc/boards/esp32/) for ESP32 specifics.

Happy snapping! 📷✨
