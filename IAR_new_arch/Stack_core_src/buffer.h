#pragma once
#include "stdbool.h"
#include "frame.h"

struct frame* BF_next_tx(struct frame* frame);
bool BF_remove_tx(struct frame *frame);
bool BF_remove_rx(struct frame *frame);
bool BF_push_rx(struct frame *frame);
bool BF_push_tx(struct frame *frame);

void* BF_cursor_rx(void);
void* BF_cursor_tx(void);
void* BF_cursor_next(void* cursor); // NULL если нет следующего
struct frame* BF_content(void* cursor);
