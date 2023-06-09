#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include "queue.h"
#include <signal.h>

int job_id_counter = 0;


void submit_job(char *command, char **args, queue *job_queue);
void print_job_status(job_t *job);
void show_jobs(queue *job_queue);
void print_job_history(queue *job_queue);
void execute_job(job_t *job);
void execute_next_job(queue *job_queue, int running_jobs);
void run_job_scheduler(queue *job_queue, int max_jobs);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <max_jobs>\n", argv[0]);
        return 1;
    }

    int max_jobs = atoi(argv[1]);

    queue job_queue = *queue_init(MAX_JOBS_IN_QUEUE);

    run_job_scheduler(&job_queue, max_jobs);

    queue_destroy(&job_queue);
    return 0;
}


void submit_job(char *program, char **args, queue *job_queue) {
    // Create a new job struct
    job_t *job = malloc(sizeof(job_t));
    if (job == NULL) {
        perror("malloc");
        return;
    }

    // Initialize the job fields
    job->job_id = job_id_counter;

    strcpy(job->program, program);

    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }
    job->args = (char **)malloc( arg_count+1 * sizeof(char *));
    int pos = 0;
    while (pos < arg_count) {
        job->args[pos] = (char *)malloc((strlen(args[pos]) + 1) * sizeof(char));
        strcpy(job->args[pos], args[pos]);
        pos++;
    }

    job->output_file = malloc(sizeof(char) * MAX_FILENAME_LEN);
    snprintf(job->output_file, MAX_FILENAME_LEN, "%d.out", job_id_counter);       


    job->error_file = malloc(sizeof(char) * MAX_FILENAME_LEN);
    snprintf(job->error_file, MAX_FILENAME_LEN, "%d.err", job_id_counter);

    job->pid = -1;
    job->completed = false;
    job->status = 0;
    

    // Increment the global job id counter
    job_id_counter++;

    // Add the job to the job queue
    if (queue_insert(job_queue, job) == -1) {
        printf("%s \n", program); 
        perror("queue_push");
        return;
    }

    printf("Job %d added to the queue\n", job->job_id);
}

void print_job_status(job_t *job) {
    printf("Job %d: ", job->job_id);
    if (job->status == 0) {
        printf("waiting\n");
    } else {
        printf("running\n");
    }
}

void show_jobs(queue *job_queue) {
    printf("Job Queue:\n");

    printf("jobid \t command \t status\n");
    int pos = 0;

    while (pos < job_queue->count) {
        job_t *current = queue_get(job_queue, pos);
        if (current->program && current->status != 2){
            printf("%d \t %s \t %s\n", current->job_id, current->program, current->status == 0? "waiting" :"running");
        } 
        pos++;
    }
    printf("\n");
}


void execute_job(job_t *job) {
    job->start_time = time(NULL);
    job->status = 1;
    int pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error: fork failed (%s)\n", strerror(errno));
        return;
    }else if (pid == 0) {
        // Child process
        // Generate output and error file names
        char outfile[20];
        char errfile[20];
        sprintf(outfile, "%d.out", job->job_id);
        sprintf(errfile, "%d.err", job->job_id);

        FILE *op = freopen(outfile, "w+", stdout);

        FILE *ep = freopen(errfile, "w+", stderr);

        // Set up alarm to limit execution time
        // signal(SIGALRM, SIG_DFL);
        // alarm(atoi(job->args));

        if (execvp(job->program,job->args) == -1) {
            fprintf(stderr, "Error: command not found (%s)\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        job->status = 1;
        fclose(op);
        fclose(ep);
        exit(0);
    } else if (pid > 0) {
        // Parent process
        job->pid = pid;
    }
}

void print_time(long int t) {
    if (t <0) return;
    struct tm tm = *localtime(&t);
    printf("%04d-%02d-%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    printf("%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void print_job_history(queue *job_queue) {
    printf("Job History:\n");
    printf("jobid \t command \t starttime \t endtime \t status \n");
    int pos = 0;
    while (pos < job_queue->count) {
        job_t *current = queue_get(job_queue, pos++);
        if (current->status !=2) continue;
        printf("%d \t ", current->job_id);
        printf("%s \t ", current->program);
        print_time(current->start_time);
        printf(" \t ");
        print_time(current->end_time);
        printf(" \t ");
        printf("%s \n", current->status == 0 ? "waiting" : current->status == 1 ? "running": "completed");
        
    }
    printf("\n");
}

void execute_next_job(queue *job_queue, int p) {
    if (job_queue->size == 0) {
        return;
    }
    job_t *job = peek(job_queue);
    if (job->completed) {
        dequeue(job_queue);
        return;
    }
    if (job->pid == -1) {
        execute_job(job);
    }
    if (job->pid > 0) {
        int status;
        pid_t result = waitpid(job->pid, &status, WNOHANG);
        if (result == 0) {
            return;
        }
        job->end_time = time(NULL);
        job->completed = 1;
        job->status = 2;
        dequeue(job_queue);
        execute_next_job(job_queue, p);
    }
}




void run_job_scheduler(queue *job_queue, int max_jobs) {
    char command[MAX_COMMAND_LEN];
    int job_id = 1;
    int running_jobs = 0;

    while (true) {
        printf("job-scheduler> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break; // EOF or error occurred
        }

        // Remove newline character from command string
        command[strcspn(command, "\n")] = '\0';


        if (strncmp(command, "submithistory", 13) == 0) {
            print_job_history(job_queue);
        } else if (strncmp(command, "submit", 6) == 0) {
            char args_str[MAX_COMMAND_LEN];
            sscanf(command, "submit %[^\n]s", args_str);  // Remove "submit " from input string
            char program[strlen(args_str) + 1];
            sscanf(args_str, "%s", program); // Extract program name
            if (strlen(program) > 0) {
                char *args[MAX_ARGS_LEN] = { NULL }; // Initialize args array with NULL values
                int arg_count = 0;
                char *arg = strtok(args_str, " ");
                while (arg != NULL) {
                    if (arg_count < MAX_ARGS_LEN){
                       args[arg_count] = malloc(strlen(arg) + 1);
                       strcpy(args[arg_count], arg);
                    }
                    arg_count++;
                    arg = strtok(NULL, " ");
                }
                submit_job(program, args, job_queue);
                job_id++;
            }
            
        } else if (strncmp(command, "showjobs", 6) == 0) {
            show_jobs(job_queue);
        } else if (strncmp(command, "exit", 4) == 0) {
            break; // exit loop on "exit" command
        } else {
            printf("Invalid command.\n");
        }

        // Execute the next job if there are available cores
        execute_next_job(job_queue, running_jobs);

        // // Wait for any completed jobs
        // while (running_jobs >= max_jobs) {
        //     int status;
        //     pid_t pid = wait(&status);

        //     if (pid == -1) {
        //         perror("wait");
        //         break;
        //     } else {
        //         running_jobs--;
        //         // Update the completed job in the queue
        //         for (int i = 0; i < queue_size(job_queue); i++) {
        //             job_t *job = (job_t *) queue_get(job_queue, i);
        //             if (job->pid == pid) {
        //                 job->completed = true;
        //                 printf("Job %d (%s) completed\n", job->job_id, job->program);
        //                 print_job_status(job);
        //                 break;
        //             }
        //         }
        //     }
        // }
    }
}
