#include "../lib/PRIONP.h"
#include <stdio.h>

static queue_object* PRIONP_queue;

process* PRIONP_tick(process* running_process) {
    if (running_process == NULL || running_process -> time_left == 0) {
        running_process = queue_poll(PRIONP_queue);
    }
    if (running_process != NULL && running_process -> time_left > 0) {
        running_process -> time_left--;
    }
    return running_process;
}

int PRIONP_startup() {
    PRIONP_queue = new_queue();
    if (PRIONP_queue == NULL) {
        return 1;
    }
    return 0;
}

process* PRIONP_new_arrival(process* arriving_process, process* running_process) {
    if (arriving_process != NULL) {
        queue_object* new_elem = (queue_object*)malloc(sizeof(queue_object));
        new_elem -> object = arriving_process;
        new_elem -> next = NULL;

        if (PRIONP_queue -> next == NULL || arriving_process -> priority > ((process*)(PRIONP_queue -> next -> object)) -> priority) {
            new_elem -> next = PRIONP_queue -> next;
            PRIONP_queue -> next = new_elem;
        } else {
            queue_object* curr = PRIONP_queue -> next;
            queue_object* prev = PRIONP_queue;

            while (curr != NULL && arriving_process -> priority <= ((process*)(curr -> object)) -> priority) {
                prev = curr;
                curr = curr -> next;
            }

            new_elem -> next = curr;
            prev -> next = new_elem;
        }
    }
    return running_process;
}

void PRIONP_finish() {
    free_queue(PRIONP_queue);
}
