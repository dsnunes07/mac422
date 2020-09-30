#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 128
#define QUANTUM 0.05

// user input args
int scheduler;
char *trace_filename;
char *output_filename;
int print_events = 0;

//auxiliar
int trace_size;
struct Process **exec_queue; // circular queue to maintain the processes in execution (for Round-Robin scheduler)
int rear = -1;
int front = -1;

// structure representing a process data
struct Process
{
  char name[30];
  int arrival_time;
  int dt;
  int deadline;
  float exec_time;
};
// linked list of processes from tracefile
struct ProcessTrace
{
  struct Process *current_process;
  struct ProcessTrace *next;
};

struct Simulation
{
  int time_elapsed;
};

// global simulator clock
struct Simulation *simulation;

// Helpers
void print_queue()
{
  printf("[");

  for (int i = 0; i <= rear; i++)
  {
    struct Process *process = exec_queue[i];

    printf("%s, ", &process->name);
  }
  printf("]\n");
}

void enqueue_process(struct Process *process)
{
  printf("\nColocando na fila...");

  if ((front == rear + 1) || front == 0 && rear == trace_size - 1)
    printf("Fila circular cheia!");

  else
  {

    if (front == -1)
      front = 0;
    rear = (rear + 1) % trace_size;
    exec_queue[rear] = process;
    printf("|%s|, t0 = %d, dt = %d, deadline = %d, exectime = %f\n", process->name, process->arrival_time, process->dt, process->deadline, process->exec_time);
    printf("\natualmente a fila esta assim: ");
    print_queue();
  }
}
struct Process *dequeue_process()
{
  struct Process *process = exec_queue[front];
  if (front == -1)
  {
    printf("Underflow!");
    return 0;
  }
  else
  {
    printf("Item deletado da fila: %s\n\n", process->name);
    front = (front + 1) % trace_size;
    return process;
  }
}

int count_linked_list(struct ProcessTrace *trace)
{
  int size = 0;
  while (trace != NULL)
  {
    size++;
    trace = trace->next;
  }
  return size;
}

void print_linked_list(struct ProcessTrace *trace)
{
  struct ProcessTrace *last = trace;
  do
  {
    printf("\nProcess:\nname: %s", trace->current_process->name);
    printf("\nt0: %d", trace->current_process->arrival_time);
    printf("\ndt: %d", trace->current_process->dt);
    printf("\ndeadline: %d", trace->current_process->deadline);
    printf("\nexec_time: %d", trace->current_process->exec_time);
    last = trace;
    trace = last->next;
  } while (trace->next != NULL);
}

// Append a process to the end of a linked list pointed by head
void append_to_linked_list(struct ProcessTrace **head, struct Process *process)
{
  struct ProcessTrace *last = *head;
  struct ProcessTrace *trace = malloc(sizeof(struct ProcessTrace *));
  trace->current_process = process;
  trace->next = NULL;
  if (*head == NULL)
  {
    *head = trace;
    return;
  }

  while (last->next != NULL)
  {
    last = last->next;
  }
  last->next = trace;
  return;
}

void *process_work(void *args)
{
  struct Process *process = (struct Process *)args;
  int dt = process->dt;
  printf("Running %s\n", process->name);
  int exec_time = 0;
  while (dt - exec_time != 0)
  {
    sleep(1);
    simulation->time_elapsed++;
    exec_time++;
    printf("Current time: %f\n", simulation->time_elapsed);
  }

  process->exec_time = exec_time;
  printf("finish executing %s at %f seconds\n", process->name, simulation->time_elapsed);
}

// Schedulers
void fcfs(struct ProcessTrace *trace)
{
  // initialize a new clock for simulation
  simulation = malloc(sizeof(struct Simulation *));
  simulation->time_elapsed = 0;

  while (trace != NULL)
  {
    struct Process *current_process = trace->current_process;
    if (current_process->arrival_time <= simulation->time_elapsed)
    {
      // execute current process until it finishes
      pthread_t process_thread;
      pthread_create(&process_thread, NULL, process_work, current_process);
      pthread_join(process_thread, NULL);
      printf("FCFS executed %s in %d seconds\n", current_process->name, current_process->exec_time);
      // move forward on process trace
      trace = trace->next;
    }
    else
    {
      // keep waiting for a new process
      simulation->time_elapsed++;
      sleep(1);
    }
  }
}

struct ProcessTrace *trace_to_queue(struct ProcessTrace *trace, struct Process *exec_queue)
{
  struct Process *current_process = trace->current_process;
  printf("\n\n***tempo atual = %d***\n", simulation->time_elapsed);

  while (current_process->arrival_time <= simulation->time_elapsed)
  {
    enqueue_process(current_process);
    trace = trace->next;
    current_process = trace->current_process;
    // getchar();
  }
  return trace;
}

void round_robin(struct ProcessTrace *trace)
{
  trace_size = count_linked_list(trace);
  exec_queue = calloc(trace_size, sizeof(struct Process *));
  simulation = malloc(sizeof(struct Simulation *));
  simulation->time_elapsed = 0;

  while (simulation->time_elapsed <= 17)
  {
    trace = trace_to_queue(trace, *exec_queue);
    simulation->time_elapsed++;
  }
}

void simulate(struct ProcessTrace *trace)
{
  if (scheduler == 1)
  {
    fcfs(trace);
  }
  if (scheduler == 3)
  {
    round_robin(trace);
  }
}

/* Read a process from trace file and create a Process struct
   with the data */
struct Process *current_process(char *line)
{
  // extract tokens from tracefile
  char *token;
  char *process_data[4];
  int i = 0;
  while ((token = strsep(&line, " ")) != NULL)
  {
    process_data[i++] = token;
  }
  // allocates space for structs
  struct Process *process = malloc(sizeof(struct Process));
  // assign members of Process with data from tokens extracted from Tracefile
  strcpy(process->name, process_data[0]);
  process->arrival_time = atoi(process_data[1]);
  process->dt = atoi(process_data[2]);
  process->deadline = atoi(process_data[3]);
  process->exec_time = 0;
  return process;
}

/* Build ProcessTrace linked list
  returns head of linked list containing processes sorted by
  arrival time */
struct ProcessTrace *read_tracefile()
{
  FILE *trace_file = fopen(trace_filename, "r");
  if (trace_file == NULL)
  {
    printf("Could not open this tracefile: %s\n", trace_filename);
    exit(1);
  }
  struct ProcessTrace *head = NULL;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), trace_file))
  {
    struct Process *process = current_process(line);
    append_to_linked_list(&head, process);
  }
  fclose(trace_file);
  return head;
}

// reads user input args
void read_args(int argc, char **argv)
{
  if (argc < 4)
  {
    printf("usage: ./ep1 scheduler trace_filename output_filename [-d]\n");
    exit(1);
  }
  scheduler = atoi(argv[1]);
  trace_filename = argv[2];
  output_filename = argv[3];
  if (argc > 4 && strcmp(argv[4], "-d") == 0)
  {
    print_events = 1;
  }
}

int main(int argc, char **argv)
{
  read_args(argc, argv);
  struct ProcessTrace *trace = read_tracefile();
  simulate(trace);
  if (print_events)
  {
    printf("and I will print the events ;)\n");
  }
  return 0;
}
