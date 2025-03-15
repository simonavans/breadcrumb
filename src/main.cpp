#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>

#include "gps.h"
#include "lora.h"
#include "secrets.h"

static char recv_buf[512];
static bool joined = false;
static bool connected = false;

void setup() {
    Serial.begin(9600);
    Serial1.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    delay(2000);

    Serial.println("Getting module properties...");

    if (lora_send_cmd("+AT: OK", 100, "AT\r\n", recv_buf, sizeof(recv_buf))) {
        connected = true;
        char appKeyCmd[100];
        sprintf(appKeyCmd, "AT+KEY=APPKEY,\"%s\"\r\n", APP_KEY);

        lora_send_cmd("+ID: AppEui", 1000, "AT+ID\r\n", recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n", recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+DR: EU868", 1000, "AT+DR=EU868\r\n", recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n", recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+KEY: APPKEY", 1000, appKeyCmd, recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+CLASS: C", 1000, "AT+CLASS=A\r\n", recv_buf,
                      sizeof(recv_buf));
        lora_send_cmd("+PORT: 8", 1000, "AT+PORT=8\r\n", recv_buf,
                      sizeof(recv_buf));

        Serial.println("Setup stage successful");

        delay(200);
    } else {
        Serial.println("Could not connect to LoRaWAN E5 module");
    }
}

void loop() {
    if (!connected)
        return;

    char latitude[11], longitude[11];
    location loc;
    gps_get_location(&loc);
    dtostrf(loc.lat, 0, 6, latitude);
    dtostrf(loc.lng, 0, 6, longitude);

    int ret = 0;

    if (!joined) {
        Serial.println("Attempting to join network...");
        ret = lora_send_cmd("+JOIN: Network joined", 12000, "AT+JOIN\r\n",
                            recv_buf, sizeof(recv_buf));
        if (ret) {
            joined = true;
        } else {
            Serial.println("Failed to join network");
            delay(5000);
        }
    } else {
        Serial.println("Attempting to send data...");

        char cmd[128];
        sprintf(cmd, "AT+CMSG=\"%s,%s;10,96.123457,124.123457\"\r\n", latitude,
                longitude);
        ret = lora_send_cmd("Done", 5000, cmd, recv_buf, sizeof(recv_buf));

        if (ret) {
            lora_print_status(recv_buf);
        } else {
            Serial.println("Failed to send data");
        }

        delay(10000);
    }

    delay(1000);
}