#pragma once
#include <TinyGPSPlus.h>

extern TinyGPSPlus gps;

void display_info();
unsigned long get_GPS_epoch_time();
bool has_gps_fix();