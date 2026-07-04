#pragma once
#include <vector>

struct Network {
  char bssid[18];
  char ssid[33];
  int rssi;
  double lat;
  double lon;
  unsigned long timestamp;
};

void init_wifi();
void disconnect_wifi();
void connect_wifi();
std::vector<Network> scan_networks();