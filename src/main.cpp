#include <Arduino.h>
#include "settings.h"
#include "gps.h"
#include "http.h"

unsigned long phase_start_time = 0;

std::vector<Network> master_batch;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD, TXD);

  init_wifi();
}

void loop() { 
  while (Serial2.available() > 0) {
    gps.encode(Serial2.read());
  }

  if (millis() - phase_start_time < 40000) {
    static unsigned long last_individual_scan = 0;
    if (millis() - last_individual_scan >= 4000) {
      if (!has_gps_fix()) {
        static unsigned long last_gps_wait_print = 0;
        if (millis() - last_gps_wait_print >= 2000) {
          last_gps_wait_print = millis();
          Serial.println("[WARN] No GPS fix yet, skipping scan until one is available.");
        }
      } else {
        last_individual_scan = millis();

        std::vector<Network> scan = scan_networks();

        master_batch.insert(master_batch.end(), scan.begin(), scan.end());

        Serial.print("[INFO] Current batch size: ");
        Serial.print(master_batch.size());
        Serial.println();
      }
    }
  } else {
    if (!master_batch.empty()) {
      send_batch_to_server(master_batch);

      master_batch.clear();
    }
    
    phase_start_time = millis();
  }
}

  