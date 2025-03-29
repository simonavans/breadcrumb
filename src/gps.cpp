#define LOG_GPS

#define COLD_START_CMD  "$PGKC030,3,1*2E\r\n"
#define HOT_START_CMD   "$PGKC030,1,1*2C\r\n"
#define WARM_START_CMD  "$PGKC030,2,1*2F\r\n"
#define BEIDOU_MODE_CMD "$PGKC115,1,0,1,0*2A\r\n"
#define LOW_POWER_CMD   "$PGKC051,0*2B\r\n"

#include "gps.h"
#include "TinyGPS++.h"
#include <Arduino.h>
#include <SoftwareSerial.h>

static SoftwareSerial sws(0, 1);
static char gps_buf[64];
static int gps_buf_cnt = 0;
static TinyGPSPlus gps_dec;

void gps_init() {
    sws.begin(9600);
    delay(2000);
    sws.println("$PGKC030,1,1*2C");
    delay(4000);
}

bool gps_get_location(location* loc) {
    bool ret = false;

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
                    Serial.print("LAT=");
                    Serial.println(loc->lat, 6);
                    Serial.print("LONG=");
                    Serial.println(loc->lng, 6);
#endif
                } else {
#ifdef LOG_GPS
                    Serial.println("[GPS] INVALID LOCATION");
#endif
                }
                ret = true;
            }
        }

        for (int i = 0; i < gps_buf_cnt; i++) {
            gps_buf[i] = '\0';
        }
        gps_buf_cnt = 0;
    }

    return ret;
}