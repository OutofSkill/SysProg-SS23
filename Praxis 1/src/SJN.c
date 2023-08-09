#include "../lib/SJN.h"

static queue_object* SJN_queue;
//You can add more global variables here

process* SJN_tick (process* running_process){
    if (running_process == NULL || running_process -> time_left == 0) {
        running_process = queue_poll(SJN_queue);
    }
    if (running_process != NULL && running_process -> time_left > 0) {
        running_process -> time_left--;
    }
    return running_process;
}

int SJN_startup(){
    SJN_queue = new_queue();
    if (SJN_queue == NULL) {
        return 1;
    }
    return 0;
}

process* SJN_new_arrival(process* arriving_process, process* running_process){
    if (arriving_process != NULL) {
        queue_object* new_elem = (queue_object*)malloc(sizeof(queue_object));
        new_elem -> object = arriving_process;
        new_elem -> next = NULL;

    if (SJN_queue -> next == NULL || arriving_process -> time_left < ((process*)(SJN_queue -> next -> object)) -> time_left) {
        new_elem -> next = SJN_queue -> next;
        SJN_queue -> next = new_elem;
    }
    else {
        queue_object* current = SJN_queue -> next;
        queue_object* previous = SJN_queue;

        while (current != NULL && arriving_process -> time_left >= ((process*)(current -> object)) -> time_left) {
            previous = current;
            current = current -> next;
        }

        new_elem -> next = current;
        previous -> next = new_elem;
    }
}
return running_process;
}

void SJN_finish(){
    free_queue(SJN_queue);
}
