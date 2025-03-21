#include "lora.h"
#include <Arduino.h>

static bool led_on = false;

int lora_send_cmd(char* ack, int timeout_ms, char* cmd, char* recv_buf,
                  int recv_buf_len) {
    if (ack == NULL)
        return 0;

    memset(recv_buf, 0, recv_buf_len);
    Serial1.print(cmd);
    Serial.print(cmd);
    delay(200);

    int idx          = 0;
    int millis_start = millis();

    do {
        while (Serial1.available() > 0) {
            char ch         = (char)Serial1.read();
            recv_buf[idx++] = (int)ch;
            Serial.print(ch);
            delay(2);
        }

        if (strstr(recv_buf, ack) != NULL) {
            return 1;
        }
    } while (millis() - millis_start < timeout_ms);

    return 0;
}

void lora_print_status(char* status_msg) {
    if (status_msg == NULL)
        return;

    int data = 0;
    int rssi = 0;
    int snr  = 0;

    char* start = strstr(status_msg, "RX");
    int parsed  = sscanf(start, "RX: \"%d\"\r\n", &data);
    if (start && parsed == 1) {
        Serial.println(data);
        led_on = !!data;
        digitalWrite(LED_BUILTIN, led_on ? LOW : HIGH);
    }

    start  = strstr(status_msg, "RSSI");
    parsed = sscanf(start, "%d", &rssi);
    if (start && parsed == 1) {
        Serial.print("RSSI: ");
        Serial.println(rssi);
    }

    parsed = sscanf(start, "%d", &snr);
    if (start && parsed == 1) {
        Serial.print("SNR: ");
        Serial.println(snr);
    }
}
