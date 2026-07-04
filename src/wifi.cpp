#include <Arduino.h>
#include <WiFi.h>
#include "net.h"
#include "gps.h"
#include "settings.h"

void init_wifi() {
  WiFi.mode(WIFI_AP_STA);
}

void disconnect_wifi() {
  WiFi.disconnect();
}

void connect_wifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.print("[INFO] Connecting to WiFi.");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
}

std::vector<Network> scan_networks() {
  Serial.println("[INFO] Scanning networks.");

  disconnect_wifi();

  std::vector<Network> batch;

  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("[WARN] No networks found.");
  } else {
    Serial.print("[INFO] ");
    Serial.print(n);
    Serial.print(" networks found.\n");

    for (int i = 0; i < n; i++) {
      Network net;

      strncpy(net.bssid, WiFi.BSSIDstr(i).c_str(), sizeof(net.bssid) - 1);
      net.bssid[sizeof(net.bssid) - 1] = '\0';

      strncpy(net.ssid, WiFi.SSID(i).c_str(), sizeof(net.ssid) - 1);
      net.ssid[sizeof(net.ssid) - 1] = '\0';

      net.rssi = WiFi.RSSI(i);
      net.lat = gps.location.lat();
      net.lon = gps.location.lng();
      net.timestamp = get_GPS_epoch_time();

      batch.push_back(net);
    }

    WiFi.scanDelete();
  }

  return batch;
}