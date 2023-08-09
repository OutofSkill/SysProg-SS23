#include "../lib/RR.h"

static queue_object* RR_queue;
int static_quantum;
int zeitscheibe;

process* RR_tick(process* running_process) {
    if (running_process == NULL || running_process -> time_left == 0 || zeitscheibe == 0) {
        if (running_process && running_process -> time_left > 0) {
            queue_add(running_process, RR_queue);
        }
        running_process = queue_poll(RR_queue);
        zeitscheibe = static_quantum;
    }

    if (running_process) {
        running_process -> time_left--;
        zeitscheibe--;
    }

    return running_process;
}

int RR_startup(int quantum) {
    RR_queue = new_queue();
    static_quantum = quantum;
    zeitscheibe = quantum;
    if (RR_queue == NULL) {
        return 1;
    }
    return 0;
}

process* RR_new_arrival(process* arriving_process, process* running_process) {
    if (arriving_process) {
        queue_add(arriving_process, RR_queue);
    }
    return running_process;
}

void RR_finish() {
    free_queue(RR_queue);
}
