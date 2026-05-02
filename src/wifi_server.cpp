#include "wifi_server.h"
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SD.h>

static AsyncWebServer server(WIFI_PORT);
static bool           running = false;

// ---- HTML page (stored in flash) ---------------------------
static const char HTML_HEAD[] PROGMEM = R"html(
<!DOCTYPE html><html lang="th"><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Kid Camera</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:Arial,sans-serif;background:#fff8e1;color:#333}
header{background:#ff6b35;color:#fff;padding:16px;text-align:center}
header h1{font-size:22px;margin-bottom:4px}
header p{font-size:13px;opacity:.85}
.gallery{display:grid;grid-template-columns:repeat(auto-fill,minmax(160px,1fr));gap:12px;padding:16px}
.card{background:#fff;border-radius:14px;overflow:hidden;box-shadow:0 2px 8px rgba(0,0,0,.12);transition:transform .15s}
.card:hover{transform:translateY(-3px)}
.thumb{width:100%;height:150px;object-fit:cover;cursor:pointer;display:block}
.info{padding:6px 10px;font-size:11px;color:#888;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.actions{display:flex;gap:6px;padding:6px 10px 10px}
.btn{flex:1;padding:6px 4px;border-radius:8px;border:none;cursor:pointer;font-size:12px;font-weight:700;text-align:center;text-decoration:none;display:inline-block}
.btn-dl{background:#4caf50;color:#fff}
.btn-del{background:#f44336;color:#fff}
.empty{text-align:center;padding:60px 20px;color:#aaa}
.empty .icon{font-size:64px;margin-bottom:12px}
#modal{display:none;position:fixed;inset:0;background:rgba(0,0,0,.92);z-index:99;align-items:center;justify-content:center}
#modal.show{display:flex}
#modal img{max-width:95vw;max-height:90vh;border-radius:10px}
#modal-close{position:absolute;top:14px;right:18px;color:#fff;font-size:34px;cursor:pointer;line-height:1;user-select:none}
</style>
</head><body>
<header>
  <h1>📷 Kid Camera</h1>
  <p id="cnt"></p>
</header>
)html";

static const char HTML_FOOT[] PROGMEM = R"html(
<div id="modal">
  <span id="modal-close">&#x2715;</span>
  <img id="modal-img" src="">
</div>
<script>
var mc=document.getElementById('modal');
var mi=document.getElementById('modal-img');
document.getElementById('modal-close').onclick=close;
mc.addEventListener('click',function(e){if(e.target===mc)close();});
function open(url){mi.src=url;mc.className='show';}
function close(){mc.className='';mi.src='';}
function del(name){
  if(!confirm('ลบรูปนี้?'))return;
  fetch('/delete?file='+encodeURIComponent(name),{method:'DELETE'})
    .then(function(r){if(r.ok)location.reload();else alert('ลบไม่ได้');});
}
</script>
</body></html>
)html";

// ---- Build gallery page on every request ------------------
static String buildPage() {
    File dir = SD.open(PHOTO_DIR);
    String items;
    int    count = 0;

    if (dir) {
        File entry = dir.openNextFile();
        while (entry) {
            if (!entry.isDirectory()) {
                String name = String(entry.name());
                if (name.endsWith(".jpg") || name.endsWith(".JPG")) {
                    count++;
                    String enc  = name;
                    String photo = "/photo?file=" + enc;

                    items += "<div class='card'>";
                    items += "<img class='thumb' src='" + photo + "' loading='lazy' "
                             "onclick='open(\"" + photo + "\")' alt='" + name + "'>";
                    items += "<div class='info'>" + name + "</div>";
                    items += "<div class='actions'>";
                    items += "<a class='btn btn-dl' href='" + photo + "' download='" + name + "'>&#x2B07; บันทึก</a>";
                    items += "<button class='btn btn-del' onclick='del(\"" + name + "\")'>&#x1F5D1; ลบ</button>";
                    items += "</div></div>";
                }
            }
            entry.close();
            entry = dir.openNextFile();
        }
        dir.close();
    }

    if (count == 0) {
        items = "<div class='empty'><div class='icon'>📷</div>"
                "<p>ยังไม่มีรูป</p><p>ลองถ่ายรูปดูสิ!</p></div>";
    }

    String page = FPSTR(HTML_HEAD);
    page += "<script>document.getElementById('cnt').textContent='" +
            String(count) + " รูป';</script>";
    page += "<div class='gallery'>" + items + "</div>";
    page += FPSTR(HTML_FOOT);
    return page;
}

// ---- Validate filename to prevent path traversal ----------
static bool safe_filename(const String &name) {
    return name.length() > 0
        && name.indexOf("..") < 0
        && name.indexOf("/")  < 0
        && name.indexOf("\\") < 0;
}

// ---- Public API -------------------------------------------
bool wifi_server_start() {
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    delay(300);

    Serial.printf("[wifi] AP  SSID=%s  IP=%s\n",
                  WIFI_AP_SSID, WiFi.softAPIP().toString().c_str());

    // Gallery page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->send(200, "text/html; charset=utf-8", buildPage());
    });

    // Serve full-size photo from SD
    server.on("/photo", HTTP_GET, [](AsyncWebServerRequest *req) {
        if (!req->hasParam("file")) { req->send(400, "text/plain", "missing file"); return; }
        String name = req->getParam("file")->value();
        if (!safe_filename(name)) { req->send(400, "text/plain", "bad filename"); return; }
        String path = String(PHOTO_DIR) + "/" + name;
        if (SD.exists(path)) {
            req->send(SD, path, "image/jpeg");
        } else {
            req->send(404, "text/plain", "not found");
        }
    });

    // Delete photo
    server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *req) {
        if (!req->hasParam("file")) { req->send(400, "text/plain", "missing file"); return; }
        String name = req->getParam("file")->value();
        if (!safe_filename(name)) { req->send(400, "text/plain", "bad filename"); return; }
        String path = String(PHOTO_DIR) + "/" + name;
        if (SD.remove(path)) {
            req->send(200, "text/plain", "ok");
        } else {
            req->send(500, "text/plain", "delete failed");
        }
    });

    server.begin();
    running = true;
    return true;
}

void wifi_server_stop() {
    server.end();
    WiFi.softAPdisconnect(true);
    running = false;
    Serial.println("[wifi] AP stopped");
}

bool wifi_server_is_running() { return running; }

String wifi_get_ip() { return WiFi.softAPIP().toString(); }
