#include "gps.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "TinyGPS++.h"

static SoftwareSerial sws(0, 1);
static char gps_buf[64];
static int gps_buf_cnt = 0;
static TinyGPSPlus gps_dec;

void gps_init() {
    sws.begin(9600);
}

void gps_get_location(location* loc) {
    loc->timestamp = millis() / 1000;
    loc->lat = 69.12345678;
    loc->lng = 420.12345678;

    if (sws.available()) {
        while (sws.available()) {
            gps_buf[gps_buf_cnt++] = sws.read();
            if (gps_buf_cnt == 64)
                break;
        }

        char* gps_str = gps_buf;
        while (*gps_str) {
            if (gps_dec.encode(*gps_str++) && gps_dec.location.isValid()) {
                loc->timestamp = millis() / 1000;
                loc->lat = gps_dec.location.lat();
                loc->lng = gps_dec.location.lng();
            }
        }

        for (int i = 0; i < gps_buf_cnt; i++) {
            gps_buf[i] = '\0';
        }
        gps_buf_cnt = 0;
    }
}