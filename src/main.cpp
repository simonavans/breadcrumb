#define GPS_COOLDOWN_MS  4000
#define SEND_COOLDOWN_MS 10000

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#include "gps.h"
#include "lora.h"
#include "secrets.h"
#include "trail.h"

void send_payload(location* curr_loc, location* past_loc);

static char recv_buf[512];
static bool joined      = false;
static bool initialized = false;

static unsigned long last_gps_upd_ms  = 0;
static unsigned long last_join_upd_ms = 0;
static unsigned long last_send_upd_ms = 0;

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    gps_init();
    delay(2000);

    Serial.println("Getting LoRa module properties...");

    lora_transmit("AT\r\n", "+AT: OK", 100);
    while (lora_status == LCS_PENDING)
        lora_update();
    if (lora_status == LCS_SUCCESS) {
        initialized = true;

        char appKeyCmd[100];
        sprintf(appKeyCmd, "AT+KEY=APPKEY,\"%s\"\r\n", APP_KEY);

        lora_transmit("AT+ID\r\n", "+ID: AppEui", 1000);
        lora_update_block();
        lora_transmit("AT+MODE=LWOTAA\r\n", "+MODE: LWOTAA", 1000);
        lora_update_block();
        lora_transmit("AT+DR=EU868\r\n", "+DR: EU868", 1000);
        lora_update_block();
        lora_transmit("AT+CH=NUM,0-2\r\n", "+CH: NUM", 1000);
        lora_update_block();
        lora_transmit(appKeyCmd, "+KEY: APPKEY", 1000);
        lora_update_block();
        lora_transmit("AT+CLASS=A\r\n", "+CLASS: C", 1000);
        lora_update_block();
        lora_transmit("AT+PORT=8\r\n", "+PORT: 8", 1000);
        lora_update_block();

        Serial.println("LoRa setup stage successful");
        delay(200);
    } else {
        Serial.println("Could not connect to LoRaWAN E5 module");
    }
    return;
}

void loop() {
    if (!initialized)
        return;

    int ret = 0;
    lora_update();

    if (!joined) {
        switch (lora_status) {
            case LCS_IDLE:
                last_join_upd_ms = millis();
                Serial.println("Attempting to join network...");
                lora_transmit("AT+JOIN\r\n", "+JOIN: Network joined", 12000);
                break;
            case LCS_SUCCESS:
                Serial.println("Successfully joined network");
                lora_status = LCS_IDLE;
                joined      = true;
                break;
            case LCS_TIMEOUT:
                Serial.println("Failed to join network");
                lora_status = LCS_IDLE;
                break;
            default: break;
        }
        return;
    }

    location curr_loc = { 0 };

    if (millis() - last_gps_upd_ms >= GPS_COOLDOWN_MS) {
        bool update = gps_get_location(&curr_loc);

        if (update) {
            last_gps_upd_ms = millis();

            if (curr_loc.lat != 0 || curr_loc.lng != 0)
                trail_store_loc(&curr_loc);

            if (millis() - last_send_upd_ms >= SEND_COOLDOWN_MS) {
                last_send_upd_ms = millis();
                location past_loc;
                trail_get_past_loc(&past_loc);

                switch (lora_status) {
                    case LCS_IDLE: send_payload(&curr_loc, &past_loc); break;
                    case LCS_SUCCESS:
                        Serial.println("Payload sent successfully");
                        lora_status = LCS_IDLE;
                        break;
                    case LCS_TIMEOUT:
                        Serial.println("Failed to send payload");
                        lora_status = LCS_IDLE;
                        break;
                    default: break;
                }
            }
        }
    }
}

void send_payload(location* curr_loc, location* past_loc) {
    char curr_lat[11], curr_lng[11];
    dtostrf(curr_loc->lat, 0, 6, curr_lat);
    dtostrf(curr_loc->lng, 0, 6, curr_lng);
    char past_lat[11], past_lng[11];
    dtostrf(past_loc->lat, 0, 6, past_lat);
    dtostrf(past_loc->lng, 0, 6, past_lng);
    int time_diff = (millis() / 1000) - past_loc->timestamp;

    char cmd[128];
    sprintf(cmd, "AT+CMSG=\"%s,%s;%d,%s,%s\"\r\n", curr_lat, curr_lng,
            time_diff, past_lat, past_lng);

    lora_transmit(cmd, "Done", 5000);
}