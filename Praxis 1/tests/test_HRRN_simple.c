#include"../lib/scheduler.h"
#include"../lib/process.h"
#include "helpers.h"
#include<string.h>
#include<stdlib.h>
#include<stdio.h>

#define PROCESS_COUNT   6
#define RR_QUANTUM      4
#define STRATEGY        HRRN

int main()
{
    /*Defining the processes                Arrival Dur.    Prio    ID
                                            uint    uint    uint    char*/
    process processes[PROCESS_COUNT] = {{   0,      3,      1,      'A'},
                                        {   1,      8,      3,      'B'},
                                        {   2,      1,      2,      'C'},
                                        {   3,      10,      1,      'D'},
                                        {   4,      2,      8,      'E'},
                                        {   5,      1,      22,      'F'}};
    char* expected_result = "AAACBBBBBBBBFEEDDDDDDDDDD";
    print_schedule_info(PROCESS_COUNT, STRATEGY, 0, processes);

    char* resulting_schedule = scheduler(processes,PROCESS_COUNT,STRATEGY,RR_QUANTUM);
    unsigned int err = compare_result(resulting_schedule, expected_result);
    free(resulting_schedule);
    exit(err);
}