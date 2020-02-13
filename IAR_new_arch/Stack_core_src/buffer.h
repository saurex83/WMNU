#pragma once
#include "stdbool.h"
#include "frame.h"

bool BF_push_rx(struct frame *frame);
bool BF_push_tx(struct frame *frame);
void* BF_cursor_rx(void);
void* BF_cursor_tx(void);
void* BF_cursor_next(void* cursor);
struct frame* BF_content(void* cursor);
bool BF_remove(void *cursor);
int BF_rx_busy();
int BF_tx_busy();
int BF_available();
