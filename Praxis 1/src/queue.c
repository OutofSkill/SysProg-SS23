#include "../lib/queue.h"
#include <stdlib.h>
#include <stdio.h>

int queue_add(void* new_object, queue_object* queue) {
    queue_object* new_elem = (queue_object*)malloc(sizeof(queue_object));
    if (new_elem == NULL) {
        return 1;
    }
    new_elem -> object = new_object;
    new_elem -> next = NULL;
    queue_object* last = queue;
    while (last -> next != NULL) {
        last = last -> next;
    }
    last -> next = new_elem;
    return 0;
}

void* queue_poll(queue_object* queue) {
    if (queue->next == NULL) {
        return NULL;
    }
    queue_object* temp = queue -> next;
    void* polled_item = temp -> object;
    queue -> next = temp -> next;
    free(temp);
    return polled_item;
}

queue_object* new_queue() {
    queue_object* new_queue = (queue_object*)malloc(sizeof(queue_object));
    new_queue -> object = NULL;
    new_queue -> next = NULL;
    return new_queue;
}

void free_queue(queue_object* queue) {
    queue_object* temp = queue -> next;
    while (temp != NULL) {
        queue_object* next = temp -> next;
        free(temp);
        temp = next;
    }
    free(queue);
}

void* queue_peek(queue_object* queue) {
    if (queue -> next == NULL) {
        return NULL;
    }
    return queue -> next -> object;
}
