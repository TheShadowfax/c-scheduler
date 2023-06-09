# Scheduler

`scheduler` is a command-line tool that allows users to submit and manage jobs for execution on a single machine.

Usage

`scheduler` takes a single command-line argument, max_jobs, which specifies the maximum number of jobs that can be running simultaneously. Jobs are executed in a first-in, first-out (FIFO) order, and if the maximum number of jobs is reached, new jobs are added to a queue.

```bash

$ submit_job <max_jobs>
```
## Commands
> submit <program> <args>: Submit a job to the queue to be executed with the specified program and arguments. Output and error files will be generated with the job ID as the file name.

> show jobs: Display the list of jobs in the queue.

> submithistory: Display a list of all completed jobs.

>exit: Exit the program.

## Examples

### Submit a job to the queue:

```
$ submit_job 2
$ submit /hw1 1000
Job 0 added to the queue
$ submit sleep 5
Job 1 added to the queue
```
###Display the list of jobs in the queue:

```
$ show jobs
Job Queue:
Job 0: Running
Job 1: Waiting
```
### Display a list of completed jobs:
```
bash
Copy code
$ history
Job History:
jobid    command    starttime       endtime         status
0        echo       2023-04-08      2023-04-08      completed
```
### Exit the program:
```
$ exit
```
