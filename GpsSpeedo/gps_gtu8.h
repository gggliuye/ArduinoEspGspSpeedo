#pragma once
#include <cstdint>
#include "config.h"

#define GPS_PIN_RXD 20
#define GPS_PIN_TXD 21

typedef struct __GpsData {
  uint32_t age;
  uint32_t real_time;
  uint32_t satellites;
  double lat;
  double lng;
  double alt;
  double speed_kmph;
} GpsData;

extern GpsData gps_data;

#ifdef __cplusplus
extern "C" {
#endif

void GpsInit();
bool GpsGetLastest();
int64_t SystemStateGetGlobalRealTime();
void SystemStateGetTime(int* hour, int* minute, int* second);

#ifdef __cplusplus
}
#endif
