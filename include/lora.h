#ifndef LORA_H
#define LORA_H
#pragma once

int lora_send_cmd(char* ack,
                  int timeout_ms,
                  char* cmd,
                  char* recv_buf,
                  int recv_buf_len);
void lora_print_status(char* status_msg);

#endif