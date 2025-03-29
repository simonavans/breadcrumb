#include "trail.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#define TRAIL_STORE_SIZE  512
#define TRAIL_MAX_DENSITY 16

static location loc_store[TRAIL_STORE_SIZE];
static size_t loc_store_size = 0;

void trail_store_loc(location* loc) {
    location new_loc;
    new_loc.timestamp           = loc->timestamp;
    new_loc.lat                 = loc->lat;
    new_loc.lng                 = loc->lng;
    loc_store[loc_store_size++] = new_loc;
}

static int curr_divisor = TRAIL_MAX_DENSITY / 2;
static int prev_divisor = TRAIL_MAX_DENSITY + 1;
static int curr_idx     = 0;

void trail_get_past_loc(location* loc) {
    int idx = 0;

    while (curr_divisor >= 1) {
        while (curr_idx < TRAIL_MAX_DENSITY) {
            if (curr_idx % prev_divisor != 0) {
                idx       = curr_idx;
                curr_idx += curr_divisor;
            } else {
                curr_idx += curr_divisor;
                continue;
            }
            int store_idx =
                ((float)idx / (float)TRAIL_MAX_DENSITY) * loc_store_size - 1;
            location stored_loc = loc_store[store_idx];
            loc->timestamp      = stored_loc.timestamp;
            loc->lat            = stored_loc.lat;
            loc->lng            = stored_loc.lng;

            if (curr_divisor == 1 && curr_idx == TRAIL_MAX_DENSITY) {
                curr_divisor = TRAIL_MAX_DENSITY / 2;
                prev_divisor = TRAIL_MAX_DENSITY + 1;
                curr_idx     = 0;
            }
            return;
        }

        prev_divisor  = curr_divisor;
        curr_divisor /= 2;
        curr_idx      = 0;
    }
    return;
}