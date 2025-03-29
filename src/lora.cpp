#include "lora.h"
#include <Arduino.h>

static bool led_on = false;

static char cmd_res[64];
static int cmd_buf_idx     = 0;
static char cmd_buf[512]   = { 0 };
static int last_cmd_upd_ms = 0;
static int cmd_timeout_ms  = 0;

lora_cmd_status lora_status = LCS_TIMEOUT;

void lora_update() {
    if (lora_status != LCS_PENDING)
        return;

    if (cmd_buf_idx >= sizeof(cmd_buf))
        cmd_buf_idx = 0;

    if (Serial1.available() > 0) {
        char ch                = (char)Serial1.read();
        cmd_buf[cmd_buf_idx++] = (int)ch;
        Serial.print(ch);
        delay(2);
    }

    if (strstr(cmd_buf, cmd_res) != NULL) {
        lora_status = LCS_SUCCESS;
        cmd_buf_idx = 0;
    } else if (millis() - last_cmd_upd_ms >= cmd_timeout_ms) {
        lora_status = LCS_TIMEOUT;
        cmd_buf_idx = 0;
    }
}

void lora_update_block() {
    while (lora_status == LCS_PENDING)
        lora_update();
    lora_status = LCS_IDLE;
}

void lora_transmit(char* cmd, char* res, int timeout_ms) {
    if (lora_status == LCS_PENDING) {
        Serial.printf("[LORA] Failed to transmit command '%s': Another command "
                      "is already running\n",
                      cmd);
        return;
    }

    memset(cmd_buf, 0, sizeof(cmd_buf));
    strlcpy(cmd_res, res, sizeof(cmd_res));
    last_cmd_upd_ms = millis();
    cmd_timeout_ms  = timeout_ms;
    lora_status     = LCS_PENDING;

    Serial1.print(cmd);
    Serial.print(cmd);
}

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
