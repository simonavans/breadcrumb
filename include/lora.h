#ifndef LORA_H
#define LORA_H
#pragma once

enum lora_cmd_status { LCS_IDLE = 0, LCS_PENDING, LCS_SUCCESS, LCS_TIMEOUT };
extern lora_cmd_status lora_status;

void lora_update();
void lora_update_block();
void lora_transmit(char* cmd, char* res, int timeout_ms);

int lora_send_cmd(char* ack, int timeout_ms, char* cmd, char* recv_buf,
                  int recv_buf_len);
void lora_print_status(char* status_msg);

#endif