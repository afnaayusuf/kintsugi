/*
 * Event Queue Module - Implementation
 * Event-driven simulation engine for BlackBox DPU
 */

#include "event_queue.h"

void event_queue_init(EventQueue* eq) {
    eq->head = NULL;
    eq->current_time = 0;
}

void event_schedule(EventQueue* eq, uint64_t delay, void (*callback)(void*), void* context) {
    Event* new_event = (Event*)malloc(sizeof(Event));
    new_event->timestamp = eq->current_time + delay;
    new_event->callback = callback;
    new_event->context = context;
    new_event->next = NULL;
    
    // Insert in time-ordered position
    if (eq->head == NULL || new_event->timestamp < eq->head->timestamp) {
        new_event->next = eq->head;
        eq->head = new_event;
    } else {
        Event* current = eq->head;
        while (current->next != NULL && current->next->timestamp <= new_event->timestamp) {
            current = current->next;
        }
        new_event->next = current->next;
        current->next = new_event;
    }
}

bool event_process_next(EventQueue* eq) {
    if (eq->head == NULL) return false;
    
    Event* event = eq->head;
    eq->head = event->next;
    eq->current_time = event->timestamp;
    
    event->callback(event->context);
    free(event);
    
    return true;
}
