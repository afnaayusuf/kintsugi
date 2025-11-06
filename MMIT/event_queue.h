/*
 * Event Queue Module - Header
 * Event-driven simulation engine for BlackBox DPU
 */

#ifndef EVENT_QUEUE_H
#define EVENT_QUEUE_H

#include "blackbox_common.h"

/* ============================================================================
 * EVENT QUEUE MANAGEMENT FUNCTIONS
 * ============================================================================ */

void event_queue_init(EventQueue* eq);
void event_schedule(EventQueue* eq, uint64_t delay, void (*callback)(void*), void* context);
bool event_process_next(EventQueue* eq);

#endif // EVENT_QUEUE_H
