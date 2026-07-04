#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 
#include "http.h"
#include "settings.h"

void send_batch_to_server(const std::vector<Network>& batch) {
  connect_wifi();

  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();

  for (const auto& net : batch) {
    JsonObject obj = array.createNestedObject();
    obj["bssid"] = net.bssid;
    obj["ssid"] = net.ssid;
    obj["rssi"] = net.rssi;
    obj["lat"] = net.lat;
    obj["lon"] = net.lon;
    obj["time"] = net.timestamp;
  }

  String json_payload;
  serializeJson(doc, json_payload);

  HTTPClient http;
  http.begin(ENDPOINT);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-API-Key", API_KEY);

  Serial.println("[INFO] Sending data to server..");
  int http_response_code = http.POST(json_payload);

  if (http_response_code > 0) {
    String response = http.getString();
    Serial.print("[INFO] Server Response Code: ");
    Serial.println(http_response_code);
    Serial.print("[INFO] Server Message: ");
    Serial.println(response);
  } else {
    Serial.print("[WARN] Error sending POST request: ");
    Serial.println(http.errorToString(http_response_code).c_str());
  }
}