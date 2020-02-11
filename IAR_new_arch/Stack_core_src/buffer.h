#pragma once
#include "stdbool.h"
#include "frame.h"

bool BF_next_tx(struct frame* frame);
bool BF_remove_tx(struct frame *frame);
bool BF_remove_rx(struct frame *frame);
bool BF_push_rx(struct frame *frame);
bool BF_push_tx(struct frame *frame);