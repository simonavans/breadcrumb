#define LOG_LORA

#include "lora.h"
#include <Arduino.h>

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
#ifdef LOG_LORA
        Serial.print(ch);
#endif
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
#ifdef LOG_LORA
    Serial.printf("[LORA] Sending command '%s'\n", cmd);
#endif
}