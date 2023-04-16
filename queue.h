/* Header file for the simple circular queue example */
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>



#define MAX_COMMAND_LEN 10000
#define MAX_ARGS_LEN 100
#define MAX_RUNNING_JOBS 4 // maximum number of jobs allowed to run in parallel
#define MAX_JOBS_IN_QUEUE (256)
#define MAX_FILENAME_LEN 256


typedef struct {
    int job_id;
    pid_t pid;
    bool completed;
    char program[MAX_COMMAND_LEN];
    char *args;
    int arg_count;
    char *output_file;
    char *error_file;
    time_t start_time;
    time_t end_time;
    int status; // 0 - waiting, 1 - running, 2 - completed
} job_t;

typedef struct _queue {
	int size;    /* maximum size of the queue */
	job_t *buffer; /* queue buffer */
	int start;   /* index to the start of the queue */
	int end;     /* index to the end of the queue */
	int count;   /* no. of elements in the queue */
} queue;

queue *queue_init(int n);
int queue_insert(queue *q, job_t *item);
void dequeue(queue *q);
void queue_display(queue *q);
void queue_destroy(queue *q);
int queue_size(queue *q);
job_t *queue_get(queue* q, int pos);
job_t *peek(queue* q);

#endif