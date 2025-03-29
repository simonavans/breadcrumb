#ifndef GPS_H
#define GPS_H

struct location {
    unsigned long timestamp;
    double lat;
    double lng;
};

void gps_init();

bool gps_get_location(location* loc);

#endif