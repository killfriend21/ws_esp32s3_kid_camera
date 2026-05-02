#pragma once
#include <Arduino.h>

// Start a WiFi Access Point and launch the web gallery server.
bool wifi_server_start();

// Stop the server and turn off the AP.
void wifi_server_stop();

bool   wifi_server_is_running();

// Returns the AP's IP address as a String (e.g. "192.168.4.1").
String wifi_get_ip();
