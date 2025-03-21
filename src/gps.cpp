#include "gps.h"
#include "TinyGPS++.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

static SoftwareSerial sws(0, 1);
static char gps_buf[64];
static int gps_buf_cnt = 0;
static TinyGPSPlus gps_dec;

void gps_init() { sws.begin(9600); }

void gps_get_location(location* loc) {
    loc->timestamp = millis() / 1000;
    loc->lat       = 69.12345678;
    loc->lng       = 70.12345678;

    if (sws.available()) {
        while (sws.available()) {
            gps_buf[gps_buf_cnt++] = sws.read();
            if (gps_buf_cnt == 64)
                break;
        }

        char* gps_str = gps_buf;
        while (*gps_str) {
            if (gps_dec.encode(*gps_str++)) {
                if (gps_dec.location.isValid()) {
                    loc->timestamp = millis() / 1000;
                    loc->lat       = gps_dec.location.lat();
                    loc->lng       = gps_dec.location.lng();
#ifdef LOG_GPS
                    Serial.print("[GPS] Lat:");
                    Serial.printf("%s ", gps_dec.location.lat());
                    Serial.printf("Lng: %s", gps_dec.location.lng());
                    Serial.println();
#endif
                } else {
#ifdef LOG_GPS
                    Serial.println("[GPS] INVALID LOCATION");
#endif
                }
            }
        }

        for (int i = 0; i < gps_buf_cnt; i++) {
            gps_buf[i] = '\0';
        }
        gps_buf_cnt = 0;
    }
}
