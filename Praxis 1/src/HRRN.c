#include "../lib/HRRN.h"

static queue_object* HRRN_queue;


void sort_queue(queue_object* queue) {
    queue_object* current = queue->next;
    queue->next = NULL;

    while (current != NULL) {
        queue_object* temp = current;
        current = current -> next;

        if (queue -> next == NULL || ((process*)(temp -> object)) -> priority > ((process*)(queue -> next -> object)) -> priority) {
            temp -> next = queue -> next;
            queue -> next = temp;
        } else {
            queue_object* previous = queue -> next;
            while (previous -> next != NULL && ((process*)(temp -> object)) -> priority <= ((process*)(previous -> next -> object)) -> priority) {
                previous = previous -> next;
            }
            temp -> next = previous -> next;
            previous -> next = temp;
        }
    }
}

process* HRRN_tick(process* running_process) {
    if (running_process == NULL || running_process -> time_left == 0) {
        running_process = queue_poll(HRRN_queue);
    }
    if (running_process != NULL && running_process -> time_left > 0) {
        running_process -> time_left--;
    }

    queue_object* temp = HRRN_queue -> next;
    while (temp) {
        ((process*)(temp -> object)) -> start_time++;
        temp = temp -> next;
    }

    temp = HRRN_queue -> next;
    while (temp) {
        process* current_process = (process*)(temp->object);
        current_process -> priority = (current_process -> start_time + current_process -> time_left) / current_process -> time_left;
        temp = temp -> next;
    }
    sort_queue(HRRN_queue);
    return running_process;
}

int HRRN_startup() {
    HRRN_queue = new_queue();
    if (HRRN_queue == NULL) {
        return 1;
    }
    return 0;
}

process* HRRN_new_arrival(process* arriving_process, process* running_process) {
    if (arriving_process) {
        arriving_process -> start_time = 0;
        queue_add(arriving_process, HRRN_queue);
        sort_queue(HRRN_queue);
    }
    return running_process;
}

void HRRN_finish() {
    free_queue(HRRN_queue);
}
