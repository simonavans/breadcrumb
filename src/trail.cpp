#include "trail.h"
#include <stdio.h>
#include <string.h>
#include <vector>

#define TRAIL_STORE_SIZE 512
#define TRAIL_MAX_DENSITY 16

// LIFO queue?
static std::vector<location> loc_store(TRAIL_STORE_SIZE);

void trail_store_loc(location loc) {
    loc_store.push_back(loc);
}

static int curr_divisor = TRAIL_MAX_DENSITY / 2;
static int prev_divisor = TRAIL_MAX_DENSITY + 1;
static int curr_idx = 0;

void trail_get_past_loc(location* loc) {
    int idx = 0;

    while (curr_divisor >= 1) {
        if (curr_idx == 0) {
            curr_idx = 0;
        }

        while (curr_idx < TRAIL_MAX_DENSITY) {
            if (curr_idx % prev_divisor != 0) {
                idx = curr_idx;
                curr_idx += curr_divisor;
            } else {
                curr_idx += curr_divisor;
                continue;
            }
            printf("idx: %d\n", idx);
            location stored_loc = loc_store[(idx / TRAIL_MAX_DENSITY) * loc_store.size()];
            loc->lat = stored_loc.lat;
            loc->lng = stored_loc.lng;
            return;
        }
        
        prev_divisor = curr_divisor;
        curr_divisor /= 2;
        curr_idx = 0;
    }
    return;
}