#include <Arduino.h>
#include <time.h>
#include "gps.h"

TinyGPSPlus gps;

void display_info() {
  Serial.println("=== DATA RECEIVED FROM THE SATTELITE ==="); 
  if (gps.satellites.isValid()) {
    Serial.print("Available satellites: ");
    Serial.print(gps.satellites.value());
    Serial.println();
  }
  if (gps.location.isValid()) {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(", "));
    Serial.print(gps.location.lng(), 6);
    Serial.println();
  }
  if (gps.altitude.isValid()) {
    Serial.print(gps.altitude.meters());
    Serial.println();
  }
  if (gps.date.isValid()) {
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
    Serial.println();
  }
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
    Serial.println();
  }
  Serial.println();
}

unsigned long get_GPS_epoch_time() {
  if (!gps.time.isValid() || !gps.date.isValid()) {
    return 0;
  }

  struct tm t;
  t.tm_year = gps.date.year() - 1900;
  t.tm_mon = gps.date.month() - 1;
  t.tm_mday = gps.date.day();
  t.tm_hour = gps.time.hour();
  t.tm_min = gps.time.minute();
  t.tm_sec = gps.time.second();
  t.tm_isdst = 0;

  return (unsigned long)mktime(&t);
}

bool has_gps_fix() {
  // TinyGPSPlus can report location.isValid() == true while still holding
  // the default 0.0, 0.0 it starts with, before a real fix ever arrives.
  return gps.location.isValid() &&
         (gps.location.lat() != 0.0 || gps.location.lng() != 0.0);
}