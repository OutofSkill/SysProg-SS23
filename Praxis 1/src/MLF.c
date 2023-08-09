#include "../lib/MLF.h"
#include <stdlib.h>

static queue_object** MLF_queues;
static int zeitscheibe;
static int aktuelle_Ebene;

process* find_process(){
    queue_object* temp;
    for(int i = 1; i < 5; i++){
        temp = MLF_queues[i];
        if(temp -> next != NULL){
            aktuelle_Ebene = i;
            break;
        }
    }
    int fak = 1;
    for(int i = 1; i <= aktuelle_Ebene; i++){
        fak = fak * i;
    }
    zeitscheibe = fak;
    if(aktuelle_Ebene == 4){
        zeitscheibe = 100000000;
    }
    return queue_poll(temp);
}

process* MLF_tick(process* running_process) {
    if (running_process == NULL || running_process -> time_left == 0) {
        running_process = find_process();
    }
    if(zeitscheibe == 0){
        queue_add(running_process, MLF_queues[aktuelle_Ebene +1]);
        running_process = find_process();
    }

    if (running_process != NULL) {
        running_process->time_left--;
        zeitscheibe--;
    }

    return running_process;
}

int MLF_startup() {
    MLF_queues = calloc(5, sizeof(queue_object*));
    for (int i = 0; i < 5; i++) {
        MLF_queues[i] = new_queue();
    }
    return 0;
}

process* MLF_new_arrival(process* arriving_process, process* running_process) {
    if (arriving_process != NULL) {
        queue_add(arriving_process, MLF_queues[1]);
    }
    return running_process;
}

void MLF_finish() {
    for (int i = 0; i < 5; i++) {
        if (MLF_queues[i] != NULL) {
            free_queue(MLF_queues[i]);
        }
    }
    free(MLF_queues);
}
